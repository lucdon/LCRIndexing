from runner.graphGenerator import augmentGraphWithLabels


def readFromTo(file, graphFile):
    nodeSet = dict()
    edgeMapping = dict()

    with open(file, mode='r', encoding='utf-8') as f:
        i = 1

        for line in f:
            if line.startswith('#') or line.startswith('%'):
                i += 1
                continue

            split = line.split()

            source = int(split[0])
            target = int(split[1])

            if source not in nodeSet:
                nodeSet[source] = len(nodeSet)

            if target not in nodeSet:
                nodeSet[target] = len(nodeSet)

            source = nodeSet[source]
            target = nodeSet[target]

            if source not in edgeMapping:
                edgeMapping[source] = set()

            edgeMapping[source].add(target)

            i += 1

    nodeCount = len(nodeSet)
    edgeCount = 0

    for edgeSet in edgeMapping.values():
        edgeCount += len(edgeSet)

    print('number of nodes:', nodeCount)
    print('number of edges:', edgeCount)

    with open(graphFile, mode='w', encoding='utf-8') as f:
        f.write(str(nodeCount))
        f.write(',')
        f.write(str(edgeCount))
        f.write(',1\n')

        for source, targetList in edgeMapping.items():
            for target in targetList:
                f.write(str(source))
                f.write(' 0 ')
                f.write(str(target))
                f.write(' .\n')

def readFromToWithLabels(file, graphFile, numLabels):
    nodeSet = dict()
    edgeMapping = dict()

    with open(file, mode='r', encoding='utf-8') as f:
        i = 1

        for line in f:
            if line.startswith('#') or line.startswith('%'):
                i += 1
                continue

            split = line.split()

            source = int(split[0])
            target = int(split[1])

            if source not in nodeSet:
                nodeSet[source] = len(nodeSet)

            if target not in nodeSet:
                nodeSet[target] = len(nodeSet)

            source = nodeSet[source]
            target = nodeSet[target]

            if source not in edgeMapping:
                edgeMapping[source] = set()

            edgeMapping[source].add(target)

            i += 1

    nodeCount = len(nodeSet)
    edgeCount = 0

    for edgeSet in edgeMapping.values():
        edgeCount += len(edgeSet)

    print('number of nodes:', nodeCount)
    print('number of edges:', edgeCount)
    print('number of labels:', numLabels)

    edges = augmentGraphWithLabels(edgeMapping, "exp", numLabels)

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
