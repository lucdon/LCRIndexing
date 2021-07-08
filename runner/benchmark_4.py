import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        # base cases
        ["BFS", []],
        ["LI+", []],
        ["pruned-2-hop", []],

        # experimental, working
        ["KLC", ["PLL"]],
        ["KLCFreq", ["PLL"]],
        ["LWBF", ["custom", "custom", "32"]]
    ]

    # graph with label <= 64.
    graphs = [
        "robots",
        "advogato",
        "wikiVote",
        "gnutella",
        "arxiv",
        "biograd",
        "socSlashdot",
        "socEpinions",
        "email",
        "stringHS",
        "stringFC"
    ]

    # larger graphs with label <= 64.
    graphsOnlyOnSuccess = [
        "webGoogle",
        "webStanford",
        "webBerkStan"
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess)
