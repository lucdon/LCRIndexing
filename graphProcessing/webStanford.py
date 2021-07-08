from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/webStanford/web-Stanford.txt"
    graphFile = './workload/webStanford/graph.nt'

    # download from: https://snap.stanford.edu/data/web-Stanford.html
    stanfordReader.readFromToWithLabels(file, graphFile, 64)
