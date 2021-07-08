from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/socPokec/soc-pokec-relationships.txt"
    graphFile = './workload/socPokec/graph.nt'

    # download from: https://snap.stanford.edu/data/soc-Pokec.html
    stanfordReader.readFromToWithLabels(file, graphFile, 64)
