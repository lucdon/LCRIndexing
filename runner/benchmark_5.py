import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        # base cases
        ["BFS", []],
        ["pruned-2-hop", []],

        # experimental, working
        ["KLC", ["PLL"]]
    ]

    # graphs with label >=  64
    graphs = [
        "swdf",
        "reddit",
        "notreDame",
        "wordnet",
        "wikiTalk"
    ]

    # larger graphs with label >=  64
    graphsOnlyOnSuccess = [
        "patents",
        "socPokec",
        "wikitionary",
        "lgd"
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess)
