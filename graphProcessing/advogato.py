from graphProcessing.readers import EdgeReader

if __name__ == "__main__":
    file = "./workload/advogato/advogato.edge"
    graphFile = './workload/advogato/graph.nt'

    # download from: http://konect.cc/files/download.tsv.advogato.tar.bz2
    EdgeReader.readFromTo(file, graphFile)
