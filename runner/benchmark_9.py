import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["pruned-2-hop", []],
        ["sh", ["klc-bfl"]]
    ]

    graphs = [
        "dbpedia"
    ]

    graphsOnlyOnSuccess = [
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess, difficulty=120)
