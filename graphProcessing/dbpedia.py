from graphProcessing.readers import RdfReader

if __name__ == "__main__":
    file = "./workload/dbpedia/dbpedia.rdf"
    graphFile = './workload/dbpedia/graph.nt'

    # download from: https://www.rdfhdt.org/datasets/
    RdfReader.readFromTo(file, graphFile)
