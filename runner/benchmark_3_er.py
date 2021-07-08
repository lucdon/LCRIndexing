import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["BFS", []],
        ["pruned-2-hop", []],
        ["sh", ["klc", "pll"]],
        ["sh", ["klc-freq", "pll"]],
        ["sh", ["klc-bfl"]]
    ]

    nodeSizes = [
        100_000,
        500_000
    ]

    labelSizes = [
        8,
        16,
        32,
        64,
        128
    ]

    degrees = [
        3,
        5
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees, difficulty=2)
