#include "Compression.h"

// Cabinet Compression
#include <fdi.h>
#include <fci.h>

namespace {
    FNALLOC(fdiAlloc) {
        return operator new(cb);
    }

    FNFREE(fdiFree) {
        operator delete(pv);
    }

    FNOPEN(fdiOpen) {
        LibGens::File* file;
        sscanf(pszFile, "%p", &file);

        return (INT_PTR)file;
    }

    FNREAD(fdiRead) {
        return (UINT)((LibGens::File*)hf)->read(pv, cb);
    }

    FNWRITE(fdiWrite) {
        return (UINT)((LibGens::File*)hf)->write(pv, cb);
    }

    FNCLOSE(fdiClose) {
        return 0;
    }

    FNSEEK(fdiSeek) {
        LibGens::File* file = (LibGens::File*)hf;
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
        LibGens::File* file;
        sscanf(pszFile, "%p", &file);
        delete file;

        return 0;
    }

    FNFCIGETTEMPFILE(fciGetTempFile) {
        LibGens::File* file = new LibGens::File();
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
        LibGens::File* file = NULL;
        int size = 0;
    };

    int mspackRead(mspack_file* file, void* buffer, int bytes) {
        ReadStream* stream = reinterpret_cast<ReadStream*>(file);

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
        LibGens::File* file = NULL;
        size_t size = 0;
    };

    int mspackWrite(mspack_file* file, void* buffer, int bytes) {
        WriteStream* stream = reinterpret_cast<WriteStream*>(file);

        size_t size_to_write = min(stream->size, static_cast<size_t>(bytes));
        stream->file->write(buffer, size_to_write);
        stream->size -= size_to_write;

        return static_cast<int>(size_to_write);
    }

    void* mspackAlloc(mspack_system* self, size_t bytes) {
        return operator new(bytes);
    }

    void mspackFree(void* ptr) {
        operator delete(ptr);
    }

    void mspackCopy(void* src, void* dst, size_t bytes) {
        memcpy(dst, src, bytes);
    }

    mspack_system lzx_system = {
        nullptr,
        nullptr,
        mspackRead,
        mspackWrite,
        nullptr,
        nullptr,
        nullptr,
        mspackAlloc,
        mspackFree,
        mspackCopy
    };

    struct XCompressHeader {
        unsigned int signature;
        unsigned int field_04;
        unsigned int field_08;
        unsigned int field_0C;
        unsigned int window_size;
        unsigned int compressed_block_size;
        unsigned long long uncompressed_size;
        unsigned long long compressed_size;
        unsigned int uncompressed_block_size;
        unsigned int field_2C;

        void endianSwap() {
            Endian::swap(signature);
            Endian::swap(field_04);
            Endian::swap(field_08);
            Endian::swap(field_0C);
            Endian::swap(window_size);
            Endian::swap(compressed_block_size);
            Endian::swap(uncompressed_size);
            Endian::swap(compressed_size);
            Endian::swap(uncompressed_block_size);
            Endian::swap(field_2C);
        }
    };
}

namespace LibGens {
    void Compression::decompress(File* src_file, File* dst_file, CompressionType type) {
        switch (type) {

        case COMPRESSION_CAB: {
            char cabinet[1]{};
            char cabPath[24]{};

            sprintf(cabPath, "%p", src_file);

            ERF erf;

            HFDI fdi = FDICreate(
                fdiAlloc,
                fdiFree,
                fdiOpen,
                fdiRead,
                fdiWrite,
                fdiClose,
                fdiSeek,
                cpuUNKNOWN,
                &erf);

            FDICopy(fdi, cabinet, cabPath, 0, fdiNotify, nullptr, dst_file);
            FDIDestroy(fdi);

            break;
        }

        case COMPRESSION_X: {
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

                lzxd_stream* lzx = lzxd_init(
                    &lzx_system,
                    reinterpret_cast<mspack_file*>(&src_stream),
                    reinterpret_cast<mspack_file*>(&dst_stream),
                    window_bits,
                    0,
                    static_cast<int>(header.compressed_block_size),
                    static_cast<off_t>(uncompressed_block_size),
                    0);

                lzxd_decompress(lzx, uncompressed_block_size);
                lzxd_free(lzx);

                src_file->goToAddress(end_offset);
            }

            break;
        }

        }
    }
}