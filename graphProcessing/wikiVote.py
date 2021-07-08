from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/wikiVote/Wiki-Vote.txt"
    graphFile = './workload/wikiVote/graph.nt'

    # download from: https://snap.stanford.edu/data/wiki-Vote.html
    stanfordReader.readFromToWithLabels(file, graphFile, 6)
