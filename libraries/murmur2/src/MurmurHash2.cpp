//-----------------------------------------------------------------------------
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// This was modified as to possibilitate it's usage incrementally.
// Those modifications are also placed in the public domain, and the author of
// such modifications hereby disclaims copyright to this source code.

#include "MurmurHash2.h"

#include <cstring>
#include <vector>

//-----------------------------------------------------------------------------

// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.
const uint32_t m = 0x5bd1e995;
const int r = 24;

static uint32_t MurmurHash2(const char* data, uint32_t size)
{
    IncrementalHashInfo info{ (uint32_t)1 ^ size, (uint32_t)size };

    uint32_t i = 0;
    for (; i + 4 <= size; i += 4)
        FourBytes_MurmurHash2((const unsigned char*)data + i, info);

    unsigned char tail[4] = { 0, 0, 0, 0 };
    if (size > i)
        std::memcpy(tail, data + i, size - i);
    FourBytes_MurmurHash2(tail, info);

    return info.h;
}

uint32_t MurmurHash2(std::ifstream&& file_stream, std::size_t buffer_size, std::function<bool(char)> filter_out)
{
    bool filtered[256];
    for (int c = 0; c < 256; c++)
        filtered[c] = filter_out(static_cast<char>(c));

    file_stream.seekg(0, file_stream.end);
    auto length = static_cast<std::streamoff>(file_stream.tellg());
    file_stream.seekg(0, file_stream.beg);

    if (file_stream && length >= 0 && length <= 32 * MiB) {
        std::vector<char> contents(static_cast<std::size_t>(length));
        file_stream.read(contents.data(), length);
        auto read = file_stream.gcount();

        uint32_t size = 0;
        for (std::streamsize i = 0; i < read; i++) {
            if (!filtered[static_cast<unsigned char>(contents[i])])
                contents[size++] = contents[i];
        }

        file_stream.close();
        return MurmurHash2(contents.data(), size);
    }

    file_stream.clear();
    file_stream.seekg(0, file_stream.beg);

    auto* buffer = new char[buffer_size];
    char data[4];

    int read = 0;
    uint32_t size = 0;

    // We need the size without the filtered out characters before actually calculating the hash,
    // to setup the initial value for the hash.
    do {
        file_stream.read(buffer, buffer_size);
        read = file_stream.gcount();
        for (int i = 0; i < read; i++) {
            if (!filtered[static_cast<unsigned char>(buffer[i])])
                size += 1;
        }
    } while (file_stream); // stops at the end of the file, but also when reading failed

    file_stream.clear();
    file_stream.seekg(0, file_stream.beg);

    int index = 0;

    // This forces a seed of 1.
    IncrementalHashInfo info{ (uint32_t)1 ^ size, (uint32_t)size };
    do {
        file_stream.read(buffer, buffer_size);
        read = file_stream.gcount();
        for (int i = 0; i < read; i++) {
            char c = buffer[i];

            if (filtered[static_cast<unsigned char>(c)])
                continue;

            data[index] = c;
            index = (index + 1) % 4;

            // Mix 4 bytes at a time into the hash
            if (index == 0)
                FourBytes_MurmurHash2((unsigned char*)&data, info);
        }
    } while (file_stream); // stops at the end of the file, but also when reading failed

    // Do one last bit shuffle in the hash
    FourBytes_MurmurHash2((unsigned char*)&data, info);

    delete[] buffer;

    file_stream.close();
    return info.h;
}

void FourBytes_MurmurHash2(const unsigned char* data, IncrementalHashInfo& prev)
{
    if (prev.len >= 4) {
        // Not the final mix
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        prev.h *= m;
        prev.h ^= k;

        prev.len -= 4;
    } else {
        // The final mix

        // Handle the last few bytes of the input array
        switch (prev.len) {
            case 3:
                prev.h ^= data[2] << 16;
                [[fallthrough]];
            case 2:
                prev.h ^= data[1] << 8;
                [[fallthrough]];
            case 1:
                prev.h ^= data[0];
                prev.h *= m;
        };

        // Do a few final mixes of the hash to ensure the last few
        // bytes are well-incorporated.

        prev.h ^= prev.h >> 13;
        prev.h *= m;
        prev.h ^= prev.h >> 15;

        prev.len = 0;
    }
}

//-----------------------------------------------------------------------------
