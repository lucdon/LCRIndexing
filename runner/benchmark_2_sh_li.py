import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["sh", ["li+", "custom", "20"]],
        ["sh", ["LWBF", "custom", "custom", "32"]]
    ]

    nodeSizes = [
        10_000,
        25_000,
        50_000,
        100_000,
        250_000,
    ]

    labelSizes = [
        16,
        32,
        64
    ]

    degrees = [
        3,
        5
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees)
