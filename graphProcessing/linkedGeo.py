from graphProcessing.readers import RdfReader

if __name__ == "__main__":
    file = "./workload/lgd/graph.rdf"
    graphFile = './workload/lgd/graph.nt'

    # download from: https://www.rdfhdt.org/datasets/
    RdfReader.readFromToDoubleSided(file, graphFile)
