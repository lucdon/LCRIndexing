from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/wikiTalk/WikiTalk.txt"
    graphFile = './workload/wikiTalk/graph.nt'

    # download from: https://snap.stanford.edu/data/wiki-Talk.html
    stanfordReader.readFromToWithLabels(file, graphFile, 512)
