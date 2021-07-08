from graphProcessing.readers import RdfReader

if __name__ == "__main__":
    file = "./workload/wikitionary/graph.rdf"
    graphFile = './workload/wikitionary/graph.nt'

    # download from: https://www.rdfhdt.org/datasets/
    RdfReader.readFromToDoubleSided(file, graphFile)
