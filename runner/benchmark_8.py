import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["sh", ["klc", "pll"]],
        ["sh", ["klc-bfl"]]
    ]

    graphs = [
        "zhishi",
        "wikipediaLinkFr"
    ]

    graphsOnlyOnSuccess = [
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess, difficulty=4)
