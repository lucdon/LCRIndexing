from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/notreDame/web-NotreDame.txt"
    graphFile = './workload/notreDame/graph.nt'

    # download from: https://snap.stanford.edu/data/web-NotreDame.html
    stanfordReader.readFromToWithLabels(file, graphFile, 1024)
