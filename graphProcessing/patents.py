from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/patents/cit-Patents.txt"
    graphFile = './workload/patents/graph.nt'

    # download from: https://snap.stanford.edu/data/cit-Patents.html
    stanfordReader.readFromToWithLabels(file, graphFile, 4096)
