from graphProcessing.readers import RdfReader

if __name__ == "__main__":
    file = "./workload/wordnet/graph.rdf"
    graphFile = './workload/wordnet/graph.nt'

    # download from: https://www.rdfhdt.org/datasets/
    RdfReader.readFromToDoubleSided(file, graphFile)
