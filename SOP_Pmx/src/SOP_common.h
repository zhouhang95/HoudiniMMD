/*
 * Copyright (c) 2024
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * This SOP builds a star.
 */

#ifndef __SOP_common_h__
#define __SOP_common_h__

#include <SOP/SOP_Node.h>
#include <UT/UT_StringHolder.h>
#include <filesystem>
#include <codecvt>
#include <locale>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace HDK_Sample {
    static std::vector<char> file_get_binary(std::string const &path) {
  std::string native_path = std::filesystem::u8path(path).string();
  char const *filename = native_path.c_str();
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    perror(filename);
    return {};
  }
  if (fseek(fp, 0, SEEK_END) != 0) {
    perror(filename);
    fclose(fp);
    return {};
  }
  long size = ftell(fp);
  if (size == -1) {
    perror(filename);
    fclose(fp);
    return {};
  }
  rewind(fp);
  std::vector<char> res;
  res.resize(size);
  size_t n = fread(res.data(), res.size(), 1, fp);
  if (n != 1) {
    perror(filename);
  }
  fclose(fp);
  return res;
}

class BinaryReader {
    size_t cur = 0;
    std::vector<char> data;
public:
    bool is_eof() {
        return cur >= data.size();
    }
    BinaryReader(std::vector<char> data_) {
        data = std::move(data_);
    }
    std::string read_string(size_t len) {
        if (cur + len > data.size()) {
            throw std::out_of_range("BinaryReader::read_string");
        }
        std::string content;
        content.reserve(len);
        for (auto i = 0; i < len; i++) {
            content.push_back(read_LE<char>());
        }
        return content;
    }
    std::vector<char> read_chunk(size_t len) {
        if (cur + len > data.size()) {
            throw std::out_of_range("BinaryReader::read_chunk");
        }
        std::vector<char> content;
        content.reserve(len);
        for (auto i = 0; i < len; i++) {
            content.push_back(read_LE<char>());
        }
        return content;
    }
    size_t current() const {
        return cur;
    }
    void skip(size_t step) {
        // must use '>' rather than '>='
        if (cur + step > data.size()) {
            throw std::out_of_range("BinaryReader::skip");
        }
        cur += step;
    }
    void seek_from_begin(size_t pos) {
        // must use '>' rather than '>='
        if (pos > data.size()) {
            throw std::out_of_range("BinaryReader::seek_from_begin");
        }
        cur = pos;
    }
    template <class T>
    T read_LE() {
        // must use '>' rather than '>='
        if (cur + sizeof(T) > data.size()) {
            throw std::out_of_range("BinaryReader::read_LE");
        }
        T &ret = *(T *)(data.data() + cur);
        cur += sizeof(T);
        return ret;
    }

    // just work for basic type, not work for vec
    template <class T>
    T read_BE() {
        // must use '>' rather than '>='
        if (cur + sizeof(T) > data.size()) {
            throw std::out_of_range("BinaryReader::read_BE");
        }
        T ret = *(T *)(data.data() + cur);
        char* ptr = (char*)&ret;
        for (auto i = 0; i < sizeof(T) / 2; i++) {
            std::swap(ptr[i], ptr[sizeof(T) - 1 - i]);
        }
        cur += sizeof(T);
        return ret;
    }
    std::string read_pmx_string() {
        auto len = read_LE<int32_t>();
        if (len == 0) {
            return {};
        }
        auto bin = read_chunk(len);
        std::wstring utf16_str;
        utf16_str.resize(len / 2);
        memcpy(utf16_str.data(), bin.data(), len);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(utf16_str);
    }
    int read_pmx_int(uint8_t size) {
        if (size == 1) {
            return read_LE<int8_t>();
        }
        else if (size == 2) {
            return read_LE<int16_t>();
        }
        else if (size == 4) {
            return read_LE<int32_t>();
        }
        return 0;
    }
    int read_pmx_uint(uint8_t size) {
        if (size == 1) {
            return read_LE<uint8_t>();
        }
        else if (size == 2) {
            return read_LE<uint16_t>();
        }
        else if (size == 4) {
            return read_LE<uint32_t>();
        }
        return 0;
    }
    glm::quat read_quat() {
        auto x = read_LE<float>();
        auto y = read_LE<float>();
        auto z = read_LE<float>();
        auto w = read_LE<float>();
        return {w, x, y, z};
    }
    glm::vec4 read_float4() {
        auto x = read_LE<float>();
        auto y = read_LE<float>();
        auto z = read_LE<float>();
        auto w = read_LE<float>();
        return {x, y, z, w};
    }
    glm::vec3 read_float3() {
        auto x = read_LE<float>();
        auto y = read_LE<float>();
        auto z = read_LE<float>();
        return {x, y, z};
    }
    glm::vec2 read_float2() {
        auto x = read_LE<float>();
        auto y = read_LE<float>();
        return {x, y};
    }
};

static void setUserData(GU_Detail *detail, std::string key, std::string value) {
    GA_RWHandleS name_gah(detail->addStringTuple(GA_ATTRIB_DETAIL, key, 1));
    name_gah.set(GA_Offset(0), value);
}

static void setUserData(GU_Detail *detail, std::string key, exint value) {
    GA_RWHandleI name_gah(detail->addIntTuple(GA_ATTRIB_DETAIL, key, 1));
    name_gah.set(GA_Offset(0), value);
}

static void setUserData(GU_Detail *detail, std::string key, float value) {
    GA_RWHandleF name_gah(detail->addFloatTuple(GA_ATTRIB_DETAIL, key, 1));
    name_gah.set(GA_Offset(0), value);
}

static void setUserData(GU_Detail *detail, std::string key, std::vector<std::string> values) {
    auto names_attrib = detail->addStringArray(GA_ATTRIB_DETAIL, key);
    if (const GA_AIFSharedStringArray* aif = names_attrib->getAIFSharedStringArray()) {
        UT_StringArray names_for_set;
        names_for_set.setCapacity(values.size());
        for (auto& name : values) {
            names_for_set.append(name);
        }
        aif->set(names_attrib, GA_Offset(0), names_for_set);
    }
}

} // End HDK_Sample namespace

#endif
