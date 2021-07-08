from graphProcessing.readers import RdfReader

if __name__ == "__main__":
    file = "./workload/swdf/graph.rdf"
    graphFile = './workload/swdf/graph.nt'

    # download from: https://www.rdfhdt.org/datasets/
    RdfReader.readFromToDoubleSided(file, graphFile)
