#include "Compression.h"

// Cabinet Compression
#include <fci.h>
#include <fdi.h>

namespace {
	FNALLOC(fdiAlloc) {
		return operator new(cb);
	}

	FNFREE(fdiFree) {
		operator delete(pv);
	}

	FNOPEN(fdiOpen) {
		LibGens::File *file;
		sscanf(pszFile, "%p", &file);

		return (INT_PTR)file;
	}

	FNREAD(fdiRead) {
		return (UINT)((LibGens::File *)hf)->read(pv, cb);
	}

	FNWRITE(fdiWrite) {
		return (UINT)((LibGens::File *)hf)->write(pv, cb);
	}

	FNCLOSE(fdiClose) {
		return 0;
	}

	FNSEEK(fdiSeek) {
		LibGens::File *file = (LibGens::File *)hf;
		file->seek(dist, seektype);
		return (long)file->getCurrentAddress();
	}

	FNFDINOTIFY(fdiNotify) {
		return fdint == fdintCOPY_FILE ? (INT_PTR)pfdin->pv : 0;
	}

	FNFCIFILEPLACED(fciFilePlaced) {
		return 0;
	}

	FNFCIALLOC(fciAlloc) {
		return fdiAlloc(cb);
	}

	FNFCIFREE(fciFree) {
		return fdiFree(memory);
	}

	FNFCIOPEN(fciOpen) {
		return fdiOpen(pszFile, oflag, pmode);
	}

	FNFCIREAD(fciRead) {
		return fdiRead(hf, memory, cb);
	}

	FNFCIWRITE(fciWrite) {
		return fdiWrite(hf, memory, cb);
	}

	FNFCICLOSE(fciClose) {
		return fdiClose(hf);
	}

	FNFCISEEK(fciSeek) {
		return fdiSeek(hf, dist, seektype);
	}

	FNFCIDELETE(fciDelete) {
		LibGens::File *file;
		sscanf(pszFile, "%p", &file);
		delete file;

		return 0;
	}

	FNFCIGETTEMPFILE(fciGetTempFile) {
		LibGens::File *file = new LibGens::File();
		sprintf(pszTempName, "%p", file);

		return TRUE;
	}

	FNFCIGETNEXTCABINET(fciGetNextCabinet) {
		return false;
	}

	FNFCISTATUS(fciStatus) {
		return 0;
	}

	FNFCIGETOPENINFO(fciGetOpenInfo) {
		return fdiOpen(pszName, 0, 0);
	}
}

// Xbox Compression
#include <lzx.h>
#include <lzxd.c>

namespace {
	struct ReadStream {
			LibGens::File *file = NULL;
			int size = 0;
	};

	int mspackRead(mspack_file *file, void *buffer, int bytes) {
		ReadStream *stream = reinterpret_cast<ReadStream *>(file);

		if (stream->size == 0) {
			unsigned short size;
			stream->file->readInt16BE(&size);

			if ((size & 0xFF00) == 0xFF00) {
				stream->file->moveAddress(1);
				stream->file->readInt16BE(&size);
			}

			stream->size = size;
		}

		int size_to_read = min(stream->size, bytes);
		stream->file->read(buffer, size_to_read);
		stream->size -= size_to_read;

		return size_to_read;
	}

	struct WriteStream {
			LibGens::File *file = NULL;
			size_t size = 0;
	};

	int mspackWrite(mspack_file *file, void *buffer, int bytes) {
		WriteStream *stream = reinterpret_cast<WriteStream *>(file);

		size_t size_to_write = min(stream->size, static_cast<size_t>(bytes));
		stream->file->write(buffer, size_to_write);
		stream->size -= size_to_write;

		return static_cast<int>(size_to_write);
	}

	void *mspackAlloc(mspack_system *self, size_t bytes) {
		return operator new(bytes);
	}

	void mspackFree(void *ptr) {
		operator delete(ptr);
	}

	void mspackCopy(void *src, void *dst, size_t bytes) {
		memcpy(dst, src, bytes);
	}

	mspack_system lzx_system = {nullptr, nullptr, mspackRead, mspackWrite, nullptr, nullptr, nullptr, mspackAlloc, mspackFree, mspackCopy};

	struct XCompressHeader {
			unsigned int signature;
			unsigned int field_04;
			unsigned int field_08;
			unsigned int field_0C;
			unsigned int window_size;
			unsigned int field_14;
			unsigned long long uncompressed_size;
			unsigned long long compressed_size;
			unsigned int uncompressed_block_size;
			unsigned int compressed_block_size;

			void endianSwap() {
				Endian::swap(signature);
				Endian::swap(field_04);
				Endian::swap(field_08);
				Endian::swap(field_0C);
				Endian::swap(window_size);
				Endian::swap(field_14);
				Endian::swap(uncompressed_size);
				Endian::swap(compressed_size);
				Endian::swap(uncompressed_block_size);
				Endian::swap(compressed_block_size);
			}
	};
}

// PS3 Compression
#include <zlib/deflate.h>

namespace {
	struct SEGSHeader {
			unsigned int signature;
			unsigned short flags;
			unsigned short chunk_count;
			unsigned int uncompressed_size;
			unsigned int total_file_size;

			void endianSwap() {
				Endian::swap(signature);
				Endian::swap(flags);
				Endian::swap(chunk_count);
				Endian::swap(uncompressed_size);
				Endian::swap(total_file_size);
			}
	};

	struct SEGSChunk {
			unsigned short compressed_size;
			unsigned short uncompressed_size;
			unsigned int offset;

			void endianSwap() {
				Endian::swap(compressed_size);
				Endian::swap(uncompressed_size);
				Endian::swap(offset);
			}
	};
}

namespace LibGens {
	bool Compression::check(uint32_t signature) {
		return (signature == COMPRESSION_CAB) || (signature == COMPRESSION_X) || (signature == COMPRESSION_SEGS);
	}

	void Compression::decompress(File *src_file, File *dst_file, CompressionType type) {
		switch (type) {

			case COMPRESSION_CAB :
				{
					char cabinet[1]{};
					char cabPath[24]{};

					sprintf(cabPath, "%p", src_file);

					ERF erf;

					HFDI fdi = FDICreate(fdiAlloc, fdiFree, fdiOpen, fdiRead, fdiWrite, fdiClose, fdiSeek, cpuUNKNOWN, &erf);

					FDICopy(fdi, cabinet, cabPath, 0, fdiNotify, nullptr, dst_file);
					FDIDestroy(fdi);

					break;
				}

			case COMPRESSION_X :
				{
					XCompressHeader header;
					src_file->read(&header, sizeof(XCompressHeader));
					header.endianSwap();

					WriteStream dst_stream;
					dst_stream.file = dst_file;
					dst_stream.size = header.uncompressed_size;

					int window_bits = 0;
					unsigned int window_size = header.window_size;
					while ((window_size & 0x1) == 0) {
						++window_bits;
						window_size >>= 1;
					}

					while (src_file->getCurrentAddress() < src_file->getFileSize() && dst_file->getCurrentAddress() < header.uncompressed_size) {
						unsigned int compressed_size;
						src_file->readInt32BE(&compressed_size);
						size_t end_offset = src_file->getCurrentAddress() + compressed_size;

						ReadStream src_stream;
						src_stream.file = src_file;

						size_t uncompressed_block_size = min(static_cast<size_t>(header.uncompressed_block_size), dst_stream.size);

						lzxd_stream *lzx = lzxd_init(&lzx_system, reinterpret_cast<mspack_file *>(&src_stream), reinterpret_cast<mspack_file *>(&dst_stream), window_bits, 0, static_cast<int>(header.compressed_block_size),
													 static_cast<off_t>(uncompressed_block_size), 0);

						lzxd_decompress(lzx, uncompressed_block_size);
						lzxd_free(lzx);

						src_file->goToAddress(end_offset);
					}

					break;
				}

			case COMPRESSION_SEGS :
				{
					SEGSHeader header;
					src_file->read(&header, sizeof(SEGSHeader));
					header.endianSwap();

					for (int i = 0; i < header.chunk_count; i++) {
						SEGSChunk chunk;
						src_file->read(&chunk, sizeof(SEGSChunk));
						chunk.endianSwap();

						size_t next_chunk_offset = src_file->getCurrentAddress();
						unsigned int chunk_data_offset = chunk.offset;

						vector<unsigned char> src_data;
						src_data.resize(chunk.compressed_size);

						if (chunk.compressed_size == chunk.uncompressed_size) {
							src_file->goToAddress(chunk_data_offset);
							src_file->read(src_data.data(), chunk.compressed_size);
							dst_file->write(src_data.data(), chunk.compressed_size);
							continue;
						}
						else {
							chunk_data_offset -= 1;
						}

						src_file->goToAddress(chunk_data_offset);
						src_file->read(src_data.data(), chunk.compressed_size);

						unsigned int uncomp_size = chunk.uncompressed_size;
						if (uncomp_size == 0) {
							uncomp_size = 0x10000;
						}

						vector<unsigned char> out_data;
						out_data.resize(uncomp_size);

						z_stream zs = {0};

						if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
							break;
						}

						zs.next_in = (Bytef *)src_data.data();
						zs.avail_in = src_data.size();
						zs.next_out = (Bytef *)out_data.data();
						zs.avail_out = out_data.size();

						if (inflate(&zs, Z_SYNC_FLUSH) < Z_OK) {
							break;
						}

						dst_file->write(out_data.data(), zs.total_out);
						src_file->goToAddress(next_chunk_offset);
					}

					break;
				}
		}
	}

	static void compressCAB(File *src_file, File *dst_file, char *file_name, size_t window_size) {
		CCAB ccab;
		ZeroMemory(&ccab, sizeof(ccab));

		sprintf(ccab.szCabPath, "%p", dst_file);

		ERF erf;
		HFDI fci = FCICreate(&erf, fciFilePlaced, fciAlloc, fciFree, fciOpen, fciRead, fciWrite, fciClose, fciSeek, fciDelete, fciGetTempFile, &ccab, nullptr);

		char source_file[24]{};
		sprintf(source_file, "%p", src_file);

		FCIAddFile(fci, source_file, file_name, FALSE, fciGetNextCabinet, fciStatus, fciGetOpenInfo, TCOMPfromLZXWindow(window_size));
		FCIFlushCabinet(fci, FALSE, fciGetNextCabinet, fciStatus);

		FCIDestroy(fci);
	}

	void Compression::compress(File *src_file, File *dst_file, CompressionType type, char *file_name) {
		switch (type) {
			case COMPRESSION_CAB :
				{
					compressCAB(src_file, dst_file, file_name, 18);
					break;
				}

			case COMPRESSION_X :
				{
					File cab_file;
					compressCAB(src_file, &cab_file, "XCompression", 17);

					size_t header_address = dst_file->getCurrentAddress();
					dst_file->writeNull(sizeof(XCompressHeader) + sizeof(unsigned int));

					unsigned int data_start = 0;
					unsigned short data_count = 0;
					vector<unsigned char> compressed_data;

					cab_file.goToAddress(0x24);
					cab_file.readInt32(&data_start);
					cab_file.readInt16(&data_count);

					cab_file.goToAddress(data_start);
					for (size_t i = 0; i < data_count; i++) {
						unsigned short compressed_size = 0;
						unsigned short uncompressed_size = 0;

						cab_file.moveAddress(4);
						cab_file.readInt16(&compressed_size);
						cab_file.readInt16(&uncompressed_size);

						if (uncompressed_size != 0x8000) {
							unsigned char token = 0xFF;
							dst_file->writeUChar(&token);
							dst_file->writeInt16BE(&uncompressed_size);
						}

						dst_file->writeInt16BE(&compressed_size);

						compressed_data.resize(compressed_size);
						cab_file.read(compressed_data.data(), compressed_size);
						dst_file->write(compressed_data.data(), compressed_size);
					}

					dst_file->writeNull(5);

					size_t end_address = dst_file->getCurrentAddress();

					XCompressHeader header = {};
					header.signature = 0xFF512EE;
					header.field_04 = 0x1030000;
					header.window_size = (1 << 17);
					header.field_14 = 0x80000;
					header.uncompressed_size = src_file->getFileSize();
					header.compressed_size = end_address - (header_address + sizeof(XCompressHeader));
					header.uncompressed_block_size = header.uncompressed_size;
					header.compressed_block_size = end_address - (header_address + sizeof(XCompressHeader) + sizeof(unsigned int));
					header.endianSwap();

					dst_file->goToAddress(header_address);
					dst_file->write(&header, sizeof(XCompressHeader));
					dst_file->writeInt32(&header.compressed_block_size);
					dst_file->goToAddress(end_address);

					break;
				}
		}
	}
}