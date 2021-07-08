from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/email/Email-EuAll.txt"
    graphFile = './workload/email/graph.nt'

    # download from: https://snap.stanford.edu/data/email-EuAll.html
    stanfordReader.readFromToWithLabels(file, graphFile, 64)
