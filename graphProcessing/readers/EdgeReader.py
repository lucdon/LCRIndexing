def readFromTo(file, graphFile):
    nodeSet = dict()
    labelSet = dict()
    edgeMapping = dict()

    with open(file, mode='r', encoding='utf-8') as f:
        i = 1

        for line in f:
            if line.startswith('#'):
                i += 1
                continue

            split = line.split()

            source = int(split[0])
            target = int(split[1])
            label = int(split[2])

            if source not in nodeSet:
                nodeSet[source] = len(nodeSet)

            if target not in nodeSet:
                nodeSet[target] = len(nodeSet)

            if label not in labelSet:
                labelSet[label] = len(labelSet)

            source = nodeSet[source]
            target = nodeSet[target]
            label = labelSet[label]

            if source not in edgeMapping:
                edgeMapping[source] = dict()

            if label not in edgeMapping[source]:
                edgeMapping[source][label] = set()

            edgeMapping[source][label].add(target)

            i += 1

    nodeCount = len(nodeSet)
    labelCount = len(labelSet)
    edgeCount = 0

    for edgeSet in edgeMapping.values():
        for labelEdgeSet in edgeSet.values():
            edgeCount += len(labelEdgeSet)

    print('number of nodes:', nodeCount)
    print('number of labels:', labelCount)
    print('number of edges:', edgeCount)

    with open(graphFile, mode='w', encoding='utf-8') as f:
        f.write(str(nodeCount))
        f.write(',')
        f.write(str(edgeCount))
        f.write(',')
        f.write(str(labelCount))
        f.write('\n')

        for source, labelSet in edgeMapping.items():
            for label, targetList in labelSet.items():
                for target in targetList:
                    f.write(str(source))
                    f.write(' ')
                    f.write(str(label))
                    f.write(' ')
                    f.write(str(target))
                    f.write(' .\n')
