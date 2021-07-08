//
// Created by lucdon_vr on 07/06/2021.
//

#include "BloomFilter.hpp"

static const uint32_t hashValues[128] = { 0xAAAAAAAA, 0x55555555, 0x33333333, 0xCCCCCCCC, 0x66666666, 0x99999999,
                                          0xB5B5B5B5, 0x4B4B4B4B, 0xAA55AA55, 0x55335533, 0x33CC33CC, 0xCC66CC66,
                                          0x66996699, 0x99B599B5, 0xB54BB54B, 0x4BAA4BAA, 0xAA33AA33, 0x55CC55CC,
                                          0x33663366, 0xCC99CC99, 0x66B566B5, 0x994B994B, 0xB5AAB5AA, 0xAAAAAA33,
                                          0x555555CC, 0x33333366, 0xCCCCCC99, 0x666666B5, 0x9999994B, 0xB5B5B5AA,
                                          0xFFFFFFFF, 0xFFFF0000, 0xB823D5EB, 0xC1191CDF, 0xF623AEB3, 0xDB58499F,
                                          0xC8D42E70, 0xB173F616, 0xA91A5967, 0xDA427D63, 0xB1E8A2EA, 0xF6C0D155,
                                          0x4909FEA3, 0xA68CC6A7, 0xC395E782, 0xA26057EB, 0x0CD5DA28, 0x467C5492,
                                          0xF15E6982, 0x61C6FAD3, 0x9615E352, 0x6E9E355A, 0x689B563E, 0x0C9831A8,
                                          0x6753C18B, 0xA622689B, 0x8CA63C47, 0x42CC2884, 0x8E89919B, 0x6EDBD7D3,
                                          0x15B6796C, 0x1D6FDFE4, 0x63FF9092, 0xE7401432, 0xEFFE9412, 0xAEAEDF79,
                                          0x9F245A31, 0x83C136FC, 0xC3DA4A8C, 0xA5112C8C, 0x5271F491, 0x9A948DAB,
                                          0xCEE59A8D, 0xB5F525AB, 0x59D13217, 0x24E7C331, 0x697C2103, 0x84B0A460,
                                          0x86156DA9, 0xAEF2AC68, 0x23243DA5, 0x3F649643, 0x5FA495A8, 0x67710DF8,
                                          0x9A6C499E, 0xDCFB0227, 0x46A43433, 0x1832B07A, 0xC46AFF3C, 0xB9C8FFF0,
                                          0xC9500467, 0x34431BDF, 0xB652432B, 0xE367F12B, 0x427F4C1B, 0x224C006E,
                                          0x2E7E5A89, 0x96F99AA5, 0x0BEB452A, 0x2FD87C39, 0x74B2E1FB, 0x222EFD24,
                                          0xF357F60C, 0x440FCB1E, 0x8BBE030F, 0x6704DC29, 0x1144D12F, 0x948B1355,
                                          0x6D8FD7E9, 0x1C11A014, 0xADD1592F, 0xFB3C712E, 0xFC77642F, 0xF9C4CE8C,
                                          0x31312FB9, 0x08B0DD79, 0x318FA6E7, 0xC040D23D, 0xC0589AA7, 0x0CA5C075,
                                          0xF874B172, 0x0CF914D5, 0x784D3280, 0x4E8CFEBC, 0xC569F575, 0xCDB2A091,
                                          0x2CC016B4, 0x5C5F4421 };

void joinHash(uint32_t input, uint32_t &hash) {
    hash ^= (~((hash << 11) + (input ^ (hash >> 5))));
}

uint32_t hashFunc(uint32_t input, uint32_t index) {
    auto hash = hashValues[index];
    joinHash(input, hash);
    return hash;
}

uint32_t hashFunc(uint32_t input) {
    auto hash = 0xAAAAAAAA;
    joinHash(input, hash);
    return hash;
}

void BloomFilterDynamic::add(Vertex vertex) {
    for (uint32_t i = 0u; i < hashFunctions; i++) {
        auto hash = hashFunc(vertex, i);

        bitVector[hash % bitVector.size()] = true;
    }
}

bool BloomFilterDynamic::contains(Vertex vertex) const {
    for (uint32_t i = 0u; i < hashFunctions; i++) {
        auto hash = hashFunc(vertex, i);

        if (!bitVector[hash % bitVector.size()]) {
            return false;
        }
    }

    return true;
}

