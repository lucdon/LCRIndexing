import benchmarker

if __name__ == "__main__":
    # Rerun for these indexes at degree 3. since they failed at degree 5.
    # Which caused them to not run further at degree 3.
    indexNameAndParamsList = [
        # base cases
        ["LI+", []],
        ["pruned-2-hop", []],

        # experimental, working
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
        3
    ]

    benchmarker.runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees)
