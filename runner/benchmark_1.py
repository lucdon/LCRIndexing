import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        # base cases
        ["BFS", []],
        ["LI+", []],
        ["pruned-2-hop", []],
        ["pruned-2-hop-rec-sec", []],
        ["ALC", ["PLL"]],

        # experimental, working
        ["KLC", ["PLL"]],
        ["KLCFreq", ["PLL"]],
        ["KLCBFL", ["4"]],
        ["LWBF", ["custom", "custom", "32"]],

        # experimental, not working
        ["BFFLP", ["4"]],
        ["BPI", ["4"]],
        ["BFI", ["4"]],
        ["BGI", ["4"]],
        ["BGMI", ["4"]]
    ]

    nodeSizes = [
        10_000,
        25_000
    ]

    labelSizes = [
        2,
        4,
        8
    ]

    degrees = [
        3,
        5
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees)
