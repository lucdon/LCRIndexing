uint64_t calculateCombinationsUpToK(uint32_t k, uint32_t n) {
    if (k > n) {
        k = n;
    }

    if (k == 0) {
        return 0;
    }

    uint64_t result = 0;
    uint64_t numerator = n;
    uint64_t denominator = 1;

    for (uint64_t i = 1; i <= k; i++) {
        denominator *= i;
        result += numerator / denominator;
        numerator *= (n - i);
    }

    return result;
}

uint64_t calculateCombinationsBetween(uint32_t kMin, uint32_t kMax, uint32_t n) {
    if (kMax > n) {
        kMax = n;
    }

    int diff = int(kMax) - int(kMin);

    if (diff <= 0) {
        return 0;
    }

    uint64_t result = 0;
    uint64_t numerator = n;
    uint64_t denominator = 1;

    for (uint64_t i = 1; i <= kMax; i++) {
        denominator *= i;

        if (i >= kMin) {
            result += numerator / denominator;
        }

        numerator *= (n - i);
    }

    return result;
}

uint64_t calculateCombinationsAtK(uint32_t k, uint32_t n) {
    if (k > n) {
        return 0;
    }

    if (k * 2 > n) {
        k = n - k;
    }

    if (k == 0) {
        return 1;
    }

    uint64_t numerator = n;
    uint64_t denominator = 1;

    for (uint64_t i = 1; i < k; i++) {
        denominator *= i;
        numerator *= (n - i);
    }

    denominator *= k;

    return numerator / denominator;
}