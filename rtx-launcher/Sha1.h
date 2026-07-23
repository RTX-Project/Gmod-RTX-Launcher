// Sha1.h - компактная самостоятельная реализация SHA-1 (без внешних
// зависимостей). Используется только для сравнения содержимого файлов
// (не для криптографической защиты), поэтому SHA-1 вполне достаточно.
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

class Sha1 {
public:
    Sha1() { reset(); }

    void reset() {
        h[0] = 0x67452301; h[1] = 0xEFCDAB89; h[2] = 0x98BADCFE;
        h[3] = 0x10325476; h[4] = 0xC3D2E1F0;
        bufferLen = 0;
        totalLen = 0;
    }

    void update(const uint8_t* data, size_t len) {
        totalLen += len;
        while (len > 0) {
            size_t toCopy = 64 - bufferLen;
            if (toCopy > len) toCopy = len;
            std::memcpy(buffer + bufferLen, data, toCopy);
            bufferLen += toCopy;
            data += toCopy;
            len -= toCopy;
            if (bufferLen == 64) {
                processBlock(buffer);
                bufferLen = 0;
            }
        }
    }

    std::string finalizeHex() {
        uint64_t bitLen = totalLen * 8;
        uint8_t pad = 0x80;
        update(&pad, 1);
        uint8_t zero = 0x00;
        while (bufferLen != 56) update(&zero, 1);
        uint8_t lenBytes[8];
        for (int i = 0; i < 8; i++) lenBytes[i] = static_cast<uint8_t>(bitLen >> (56 - i * 8));
        // напрямую добавляем длину без повторного паддинга (bufferLen уже 56)
        std::memcpy(buffer + 56, lenBytes, 8);
        processBlock(buffer);
        bufferLen = 0;

        std::ostringstream oss;
        for (int i = 0; i < 5; i++) {
            oss << std::hex << std::setw(8) << std::setfill('0') << h[i];
        }
        return oss.str();
    }

    static std::string hashFile(const std::wstring& path, bool* ok = nullptr) {
        std::ifstream f(path.c_str(), std::ios::binary);
        if (!f) { if (ok) *ok = false; return ""; }
        Sha1 sha;
        std::vector<uint8_t> buf(1 << 20);
        while (f) {
            f.read(reinterpret_cast<char*>(buf.data()), buf.size());
            std::streamsize n = f.gcount();
            if (n > 0) sha.update(buf.data(), static_cast<size_t>(n));
        }
        if (ok) *ok = true;
        return sha.finalizeHex();
    }

private:
    uint32_t h[5];
    uint8_t buffer[64];
    size_t bufferLen;
    uint64_t totalLen;

    static uint32_t rol(uint32_t v, int bits) { return (v << bits) | (v >> (32 - bits)); }

    void processBlock(const uint8_t* block) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(block[i * 4 + 3]));
        }
        for (int i = 16; i < 80; i++) w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else { f = b ^ c ^ d; k = 0xCA62C1D6; }
            uint32_t temp = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = temp;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }
};
