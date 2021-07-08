from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/wikipediaLinkFr/out.wikipedia_link_fr"
    graphFile = './workload/wikipediaLinkFr/graph.nt'

    # download from: http://networkrepository.com/wikipedia_link_fr.php
    stanfordReader.readFromToWithLabels(file, graphFile, 64)
