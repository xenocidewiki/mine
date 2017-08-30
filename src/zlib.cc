//
//  zlib.cc
//  Part of Mine crypto library
//
//  You should not use this file, use mine.cc
//  instead which is automatically generated and includes this file
//  This is seperated to aid the development
//
//  Copyright (c) 2017 Muflihun Labs
//
//  This library is released under the Apache 2.0 license
//  https://github.com/muflihun/mine/blob/master/LICENSE
//
//  https://github.com/muflihun/mine
//

#include <cerrno>
#include <cstring>
#include <sstream>
#include <zlib.h>
#include "src/zlib.h"

using namespace mine;

bool ZLib::compressFile(const std::string& gzFilename, const std::string& inputFile) noexcept
{
    gzFile out = gzopen(gzFilename.c_str(), "wb");
    if (!out) {
        throw std::invalid_argument("Unable to open file [" + gzFilename + "] for writing." + std::strerror(errno));
     }
    char buff[kBufferSize];
    std::FILE* in = std::fopen(inputFile.c_str(), "rb");
    std::size_t nRead = 0;
    while((nRead = std::fread(buff, sizeof(char), kBufferSize, in)) > 0) {
        int bytes_written = gzwrite(out, buff, nRead);
        if (bytes_written == 0) {
           int err_no = 0;
           throw std::runtime_error("Error during compression: " + std::string(gzerror(out, &err_no)));
           gzclose(out);
           return false;
        }
    }
    gzclose(out);
    std::fclose(in);
    return true;
}

std::string ZLib::compressString(const std::string& str)
{
    int compressionlevel = Z_BEST_COMPRESSION;
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK) {
        throw std::runtime_error("Unable to initialize zlib deflate");
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = str.size();

    int ret;
    char outbuffer[kBufferSize];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw std::runtime_error(oss.str());
    }

    return outstring;
}

std::string ZLib::decompressString(const std::string& str)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK) {
        throw std::runtime_error("Unable to initialize zlib inflate");
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = str.size();

    int ret;
    char outbuffer[kBufferSize];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
        throw std::runtime_error(oss.str());
    }

    return outstring;
}
