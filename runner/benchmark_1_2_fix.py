import benchmarker

if __name__ == "__main__":
    # first run failed for rec-sec
    indexNameAndParamsList = [
        ["pruned-2-hop-rec-sec", []],
    ]

    nodeSizes = [
        10_000,
        25_000,
        50_000,
        100_000,
        250_000,
    ]

    labelSizes = [
        2,
        4,
        8,
        16,
        32,
        64
    ]

    degrees = [
        3,
        5
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees)
