from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/webGoogle/web-Google.txt"
    graphFile = './workload/webGoogle/graph.nt'

    # download from: https://snap.stanford.edu/data/web-Google.html
    stanfordReader.readFromToWithLabels(file, graphFile, 32)
