from graphGenerator import augmentGraphWithLabels


def processTsvLine(nodeToId, line):
    split = line.split('\t')

    # split[0] is source
    source = split[0]

    # split[1] is target
    target = split[1]

    if source not in nodeToId:
        nodeToId[source] = len(nodeToId)

    if target not in nodeToId:
        nodeToId[target] = len(nodeToId)

    return nodeToId[source], nodeToId[target]


def processFile(nodeToId, edgeMapping, file):
    with open(file, mode='r', encoding='utf-8') as f:
        i = 1

        for line in f:
            # skip header
            if i == 1:
                i += 1
                continue

            (source, target) = processTsvLine(nodeToId, line)

            if source not in edgeMapping:
                edgeMapping[source] = set()

            edgeMapping[source].add(target)

            i += 1


if __name__ == "__main__":
    # download from: https://snap.stanford.edu/data/soc-RedditHyperlinks.html
    file1 = "./workload/reddit/soc-redditHyperlinks-body.tsv"
    file2 = "./workload/reddit/soc-redditHyperlinks-title.tsv"

    nodeToIdFile = './workload/reddit/nodeLookup.tsv'
    graphFile = './workload/reddit/graph.nt'

    nodeToId = dict()
    edgeMapping = dict()

    processFile(nodeToId, edgeMapping, file1)
    processFile(nodeToId, edgeMapping, file2)

    nodeCount = len(nodeToId)
    edgeCount = 0

    labelDist = "exp"
    numLabels = 128

    for edgeSet in edgeMapping.values():
        edgeCount += len(edgeSet)

    print('number of nodes:', nodeCount)
    print('number of edges:', edgeCount)
    print('number of labels:', numLabels)

    with open(nodeToIdFile, mode='w', encoding='utf-8') as f:
        for node, nodeId in nodeToId.items():
            f.write(str(nodeId))
            f.write('\t')
            f.write(node)
            f.write('\n')

    edges = augmentGraphWithLabels(edgeMapping, labelDist, numLabels)

    with open(graphFile, mode='w', encoding='utf-8') as f:
        f.write(str(nodeCount))
        f.write(',')
        f.write(str(edgeCount))
        f.write(',')
        f.write(str(numLabels))
        f.write('\n')

        for source, target, label in edges:
            f.write(str(source))
            f.write(' ')
            f.write(str(label))
            f.write(' ')
            f.write(str(target))
            f.write(' .\n')


