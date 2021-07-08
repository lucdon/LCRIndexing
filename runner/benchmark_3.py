import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        # base cases
        ["BFS", []],
        ["LI+", []],
        ["pruned-2-hop", []],
        ["pruned-2-hop-rec-sec", []],

        # experimental, working
        ["KLC", ["PLL"]]
    ]

    nodeSizes = [
        100_000,
        500_000
    ]

    labelSizes = [
        256,
        512,
        1024,
        2048,
        4096
    ]

    degrees = [
        3,
        5
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees, difficulty=2)
