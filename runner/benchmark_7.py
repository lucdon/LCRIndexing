import benchmarker

if __name__ == "__main__":
    indexNameAndParamsList = [
        ["sh", ["klc", "pll"]],
        ["sh", ["klc-freq", "pll"]],
        ["sh", ["klc-bfl"]]
    ]

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
        "stringFC",

        "webGoogle",
        "webStanford",
        "webBerkStan",

        "swdf",
        "reddit",
        "notreDame",
        "wordnet",
        "wikiTalk",

        "patents",
        "socPokec",
        "wikitionary",
        "lgd"
    ]

    # larger graphs with label <= 64.
    graphsOnlyOnSuccess = [
    ]

    benchmarker.runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess)
