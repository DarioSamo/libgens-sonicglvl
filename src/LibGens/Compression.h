#pragma once

namespace LibGens {
    enum CompressionType {
        COMPRESSION_CAB = 0x4643534D,
        COMPRESSION_X = 0xEE12F50F,
        COMPRESSION_SEGS = 0x73676573,
    };

    class Compression {
    public:
        static bool check(uint32_t signature);
        static void decompress(File* src_file, File* dst_file, CompressionType type);
    };
}