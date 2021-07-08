import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["BFS", []],
        ["pruned-2-hop", []]
    ]

    graphs = [
        "zhishi",
        "socPokec",
        "wikipediaLinkFr",
        "dbpedia"
    ]

    graphsOnlyOnSuccess = [
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess, difficulty=4)
