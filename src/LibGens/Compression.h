#pragma once

namespace LibGens {
    enum CompressionType {
        COMPRESSION_CAB = 0x4643534D,
        COMPRESSION_X = 0xEE12F50F,
    };

    class Compression {
    public:
        static void decompress(File* src_file, File* dst_file, CompressionType type);
    };
}