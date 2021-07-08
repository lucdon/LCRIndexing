import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["sh", ["klc", "pll"]],
        # ["sh", ["klc-freq", "pll"]],
        ["sh", ["klc-bfl"]]
    ]

    graphs = [
        "patents",
        "wikitionary",
        "lgd"
    ]

    graphsOnlyOnSuccess = [
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess, difficulty=3)
