//#include <AvUtils/avMemory.h>
#include <AvUtils/avTypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <immintrin.h>



typedef void (*memswap_fn)(void*, void*, uint64);

typedef struct {
    const char* name;
    memswap_fn fn;
} bench_entry;

// High-resolution timer (nanoseconds)
static inline long long get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1e9 + ts.tv_nsec;
}

long long run_test(memswap_fn fn, unsigned char* a, unsigned char* b,
                   size_t size, int repeats) {
    // Warm-up
    for (int i = 0; i < 10; i++) {
        fn(a, b, size);
    }

    long long start = get_ns();

    for (int i = 0; i < repeats; i++) {
        fn(a, b, size);
    }

    long long end = get_ns();
    return end - start;
}




__attribute__((optimize("O2")))
void avMemswapO2(void* restrict dst, void* restrict src, uint64 size){
    byte* d = (byte*)dst;
    byte* s = (byte*)src;
    register uint64 tmp;
    while (((uint64)d & 7) && size) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
        size--;
    }

    uint64* s64 = (uint64*)s;
    uint64* d64 = (uint64*)d;
    while (size >= 8) {
        tmp = *d64;
        *d64++ = *s64;
        *s64++ = tmp;
        size -= 8;
    }

    d = (byte*)d64;
    s = (byte*)s64;

    while (size--) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
    }
}

__attribute__((optimize("O3")))
void avMemswapO3(void* restrict dst, void* restrict src, uint64 size){
    byte* d = (byte*)dst;
    byte* s = (byte*)src;
    register uint64 tmp;
    while (((uint64)d & 7) && size) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
        size--;
    }

    uint64* s64 = (uint64*)s;
    uint64* d64 = (uint64*)d;
    while (size >= 8) {
        tmp = *d64;
        *d64++ = *s64;
        *s64++ = tmp;
        size -= 8;
    }

    d = (byte*)d64;
    s = (byte*)s64;

    while (size--) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
    }
}

__attribute__((optimize("O1")))
void avMemswapO1(void* restrict dst, void* restrict src, uint64 size){
    byte* d = (byte*)dst;
    byte* s = (byte*)src;
    register uint64 tmp;
    while (((uint64)d & 7) && size) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
        size--;
    }

    uint64* s64 = (uint64*)s;
    uint64* d64 = (uint64*)d;
    while (size >= 8) {
        tmp = *d64;
        *d64++ = *s64;
        *s64++ = tmp;
        size -= 8;
    }

    d = (byte*)d64;
    s = (byte*)s64;

    while (size--) {
        tmp = *d;
        *d++ = *s;
        *s++ = tmp;
    }
}

__attribute__((optimize("O3")))
void avMemswap_prefetch(void* restrict dst, void* restrict src, uint64 size) {
    byte* d = dst;
    byte* s = src;

    const size_t PREFETCH_DISTANCE = 64; // bytes ahead

    while (size >= 16) {
        // Prefetch future data
        __builtin_prefetch(d + PREFETCH_DISTANCE, 1, 1);
        __builtin_prefetch(s + PREFETCH_DISTANCE, 1, 1);

        uint64 tmp1 = *(uint64*)d;
        uint64 tmp2 = *(uint64*)(d + 8);

        *(uint64*)d = *(uint64*)s;
        *(uint64*)(d + 8) = *(uint64*)(s + 8);

        *(uint64*)s = tmp1;
        *(uint64*)(s + 8) = tmp2;

        d += 16;
        s += 16;
        size -= 16;
    }

    while (size--) {
        byte tmp = *d;
        *d++ = *s;
        *s++ = tmp;
    }
}

__attribute__((optimize("O3")))
void avMemswap_memcpy(void* restrict dst, void* restrict src, uint64 size){
    byte tmp[size];
    memcpy(tmp, src, size);
    memcpy(src, dst, size);
    memcpy(dst, tmp, size);
}

__attribute__((optimize("O3")))
void avMemswap_naive(void* restrict dst, void* restrict src, uint64 size){
    byte* a = (byte*)dst;
    byte* b = (byte*)src;
    uint64 index = 0;
    while(index < size){
        byte buffer = a[index];
        a[index] = b[index];
        b[index] = buffer;
        index++;
    }
}

__attribute__((optimize("O3")))
void avMemswap_naive_unrolled(void* restrict dst, void* restrict src, uint64 size){
    uint8* a = dst;
    uint8* b = src;

    uint64 i = 0;

    // Swap 8 bytes at a time
    for (; i + 8 <= size; i += 8) {
        uint64 tmp = *(uint64*)(a + i);
        *(uint64*)(a + i) = *(uint64*)(b + i);
        *(uint64*)(b + i) = tmp;
    }

    // Remaining bytes
    for (; i < size; i++) {
        uint8 tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

__attribute__((target("avx2")))
void avMemswap_avx2(void* restrict dst, void* restrict src, uint64 size) {
    unsigned char* d = (unsigned char*)dst;
    unsigned char* s = (unsigned char*)src;

    // Align to 32 bytes (AVX2)
    while (((uintptr_t)d & 31) && size) {
        unsigned char tmp = *d;
        *d++ = *s;
        *s++ = tmp;
        size--;
    }

    // Main AVX2 loop (32 bytes at a time)
    while (size >= 32) {
        __m256i a = _mm256_load_si256((__m256i*)d);
        __m256i b = _mm256_load_si256((__m256i*)s);

        _mm256_store_si256((__m256i*)d, b);
        _mm256_store_si256((__m256i*)s, a);

        d += 32;
        s += 32;
        size -= 32;
    }

    // Tail
    while (size--) {
        unsigned char tmp = *d;
        *d++ = *s;
        *s++ = tmp;
    }
}

void benchmark_all() {
    const size_t MIN_SIZE = 8;
    const size_t MAX_SIZE = 1 << 22;
    const int REPEATS = 1000;

    bench_entry tests[] = {
        {"baseline", avMemswapO1},
        {"baseline(O2)", avMemswapO2},
        {"baseline(O3)", avMemswapO3},
        {"prefetch", avMemswap_prefetch},
        {"libc", avMemswap_memcpy},
        {"avx2", avMemswap_avx2},
        {"naive", avMemswap_naive},
        {"naive_unrolled", avMemswap_naive_unrolled},
    };

    const int NUM_TESTS = sizeof(tests) / sizeof(tests[0]);

    // Header
    printf("%-10s", "Size");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("%-15s", tests[i].name);
    }
    for (int i = 1; i < NUM_TESTS; i++) {
        printf("%-12s", "speedup");
    }
    printf("%-12s", "fastest");
    printf("\n");

    printf("-------------------------------------------------------------------------------\n");

    for (size_t size = MIN_SIZE; size <= MAX_SIZE; size <<= 1) {
        unsigned char *a_unaligned = malloc(size + 64);
        unsigned char *b_unaligned = malloc(size + 64);

        unsigned char *a = (unsigned char*)(((uintptr_t)a_unaligned + 63) & ~(uintptr_t)63);
        unsigned char *b = (unsigned char*)(((uintptr_t)b_unaligned + 63) & ~(uintptr_t)63);

        memset(a, 0xAA, size);
        memset(b, 0x55, size);

        long long results[NUM_TESTS];

        // Run all tests on identical data
        for (int t = 0; t < NUM_TESTS; t++) {
            results[t] = run_test(tests[t].fn, a, b, size, REPEATS);
        }

        // Print row
        printf("%-10zu", size);
        double min_ns_per_byte = 1e128;
        const char* fastest = "NONE";
        for (int t = 0; t < NUM_TESTS; t++) {
            double ns_per_byte = (double)results[t] / (REPEATS * size);
            if(ns_per_byte <= min_ns_per_byte){
                min_ns_per_byte = ns_per_byte;
                fastest = tests[t].name;
            }
            printf("%-15.4f", ns_per_byte);
        }

        // Speedups vs baseline
        for (int t = 1; t < NUM_TESTS; t++) {
            double speedup = (double)results[0] / results[t];
            printf("x%-12.2f", speedup);
        }

        printf("%-15s", fastest);

        printf("\n");

        free(a_unaligned);
        free(b_unaligned);
    }
}

int main() {
    benchmark_all();
    return 0;
}