from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/webBerkStan/web-berkStan.txt"
    graphFile = './workload/webBerkStan/graph.nt'

    # download from: https://snap.stanford.edu/data/web-BerkStan.html
    stanfordReader.readFromToWithLabels(file, graphFile, 16)
