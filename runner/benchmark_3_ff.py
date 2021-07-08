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
        128,
        256,
        512,
        1024,
        2048,
        4096
    ]

    degrees = [
        3
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees, models=['ff', 'pl'], difficulty=2)
