import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        # base cases
        ["BFS", []],
        ["LI+", []],
        ["pruned-2-hop", []],
        ["pruned-2-hop-rec-sec", []],

        # experimental, working
        ["KLC", ["PLL"]],
        ["KLCFreq", ["PLL"]],
        ["KLCBFL", ["4"]],
        ["LWBF", ["custom", "custom", "32"]]
    ]

    nodeSizes = [
        25_000,
        50_000,
        100_000,
        250_000,
    ]

    labelSizes = [
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
