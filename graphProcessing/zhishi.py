from graphProcessing.readers import stanfordReader

if __name__ == "__main__":
    file = "./workload/zhishi/out.zhishi-all"
    graphFile = './workload/zhishi/graph.nt'

    # download from: http://konect.cc/files/download.tsv.zhishi-all.tar.bz2
    stanfordReader.readFromToWithLabels(file, graphFile, 64)
