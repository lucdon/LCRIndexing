from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/socEpinions/out.soc-Epinions1"
    graphFile = './workload/socEpinions/graph.nt'

    # download from: https://snap.stanford.edu/data/soc-Epinions1.html
    stanfordReader.readFromToWithLabels(file, graphFile, 24)
