from urllib.parse import unquote
import codecs
import os


def processNodeLine(subjectLookup, line):
    if not line.startswith('<'):
        return

    idx = line.find('> ')

    if idx == -1:
        return

    sub = line[0:idx + 1]
    sub = codecs.decode(sub, 'unicode-escape')
    sub = unquote(unquote(unquote(sub))).strip()

    sub = sub.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if "upload.wikimedia.org" in sub:
        return

    if "resource/File:" in sub:
        return

    if "resource/Template:" in sub:
        return

    if "org/property/" in sub:
        return

    if  "gadm.geovocab.org" in sub:
        return

    if sub not in subjectLookup:
        subjectLookup[sub] = len(subjectLookup)


def loadNodes(nodesOutFile):
    subjectLookup = {}
    i = 0

    try:
        with open(nodesOutFile, encoding="utf-8", errors="ignore") as f:
            for line in f:
                split = line.rstrip('\n').split('\t')
                subjectLookup[split[1]] = int(split[0])
                i += 1
    except Exception as ex:
        print("Failed processing file:", nodesOutFile, i, ex, '\n', line, flush=True)

    return subjectLookup


def processNodes(filePath, nodesOutFile):
    if os.path.exists(nodesOutFile):
        print("Loading nodes from cache")
        return loadNodes(nodesOutFile)

    subjectLookup = {}
    i = 0

    try:
        with open(filePath, encoding="utf-8", errors="ignore") as f:
            for line in f:
                processNodeLine(subjectLookup, line)
                i += 1
    except Exception as ex:
        print("Failed processing file:", filePath, i, ex, '\n', line, flush=True)

    print('reduced', i, "to", len(subjectLookup), "objects", flush=True)

    with open(nodesOutFile, mode="w", encoding="utf-8") as out:
        for node, nodeId in subjectLookup.items():
            out.write(str(nodeId))
            out.write('\t')
            out.write(node)
            out.write('\n')

    return subjectLookup


def processNodeDoubleSidedLine(subjectLookup, line):
    line = line[0:-3]

    if not line.startswith('<'):
        return

    if not line.endswith('>'):
        return

    idx1 = line.find('> ')
    idx2 = line.rfind(' <')

    if idx1 == -1 or idx2 == -1:
        return

    sub = line[0:idx1 + 1]

    sub = codecs.decode(sub, 'unicode-escape')
    sub = unquote(unquote(unquote(sub))).strip()

    sub = sub.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    indexSub = "upload.wikimedia.org" not in sub \
               and "resource/File:" not in sub \
               and "resource/Template:" not in sub \
               and "org/property/" not in sub \
               and "gadm.geovocab.org" not in sub

    if indexSub and sub not in subjectLookup:
        subjectLookup[sub] = len(subjectLookup)

    if idx2 - idx1 < 3:
        return

    obj = line[idx2:]

    obj = codecs.decode(obj, 'unicode-escape')
    obj = unquote(unquote(unquote(obj))).strip()

    obj = obj.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    indexObj = "upload.wikimedia.org" not in obj \
               and "resource/File:" not in obj \
               and "resource/Template:" not in obj \
               and "org/property/" not in obj \
               and "gadm.geovocab.org" not in obj

    if indexObj and obj not in subjectLookup:
        subjectLookup[obj] = len(subjectLookup)

def processDoubleSidedNodes(filePath, nodesOutFile):
    if os.path.exists(nodesOutFile):
        print("Loading nodes from cache")
        return loadNodes(nodesOutFile)

    subjectLookup = {}
    i = 0

    try:
        with open(filePath, encoding="utf-8", errors="ignore") as f:
            for line in f:
                processNodeDoubleSidedLine(subjectLookup, line)
                i += 1
    except Exception as ex:
        print("Failed processing file:", filePath, i, ex, '\n', line, flush=True)

    print('reduced', i, "to", len(subjectLookup), "objects", flush=True)

    with open(nodesOutFile, mode="w", encoding="utf-8") as out:
        for node, nodeId in subjectLookup.items():
            out.write(str(nodeId))
            out.write('\t')
            out.write(node)
            out.write('\n')

    return subjectLookup


def processLabelLine(subjectLookup, labelLookup, line):
    line = line[0:-3]

    if not line.startswith('<'):
        return 0

    if not line.endswith('>'):
        return 0

    idx1 = line.find('> ')
    idx2 = line.rfind(' <')

    if idx1 == -1 or idx2 == -1:
        return 0

    if idx2 - idx1 < 3:
        return 0

    sub = line[0:idx1 + 1]
    pred = line[idx1 + 1:idx2].strip()
    obj = line[idx2:]

    obj = codecs.decode(obj, 'unicode-escape')
    obj = unquote(unquote(unquote(obj))).strip()

    obj = obj.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if obj not in subjectLookup:
        return 0

    sub = codecs.decode(sub, 'unicode-escape')
    sub = unquote(unquote(unquote(sub))).strip()

    sub = sub.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if sub not in subjectLookup:
        return

    pred = codecs.decode(pred, 'unicode-escape')
    pred = unquote(unquote(unquote(pred))).strip()

    if pred not in labelLookup:
        labelLookup[pred] = len(labelLookup)


def loadLabels(labelsOutFile):
    labelsLookup = {}
    i = 0

    try:
        with open(labelsOutFile, encoding="utf-8", errors="ignore") as f:
            for line in f:
                split = line.rstrip('\n').split('\t')
                labelsLookup[split[1]] = int(split[0])
                i += 1
    except Exception as ex:
        print("Failed processing file:", labelsOutFile, i, ex, '\n', line, flush=True)

    return labelsLookup


def processLabels(filePath, nodesLookup, labelsOutFile):
    if os.path.exists(labelsOutFile):
        print("Loading labels from cache")
        return loadLabels(labelsOutFile)

    labelsLookup = {}
    i = 0

    try:
        with open(filePath, encoding="utf-8", errors="ignore") as f:
            for line in f:
                processLabelLine(nodesLookup, labelsLookup, line)
                i += 1
    except Exception as ex:
        print("Failed processing file:", filePath, i, ex, '\n', line, flush=True)

    print('reduced', i, "to", len(labelsLookup), "labels", flush=True)

    with open(labelsOutFile, mode="w", encoding="utf-8") as out:
        for label, labelId in labelsLookup.items():
            out.write(str(labelId))
            out.write('\t')
            out.write(label)
            out.write('\n')

    return labelsLookup


def processLineForEdges(subjectLookup, labelsLookup, edgeMapping, line):
    line = line[0:-3]

    if not line.startswith('<'):
        return 0

    if not line.endswith('>'):
        return 0

    idx1 = line.find('> ')
    idx2 = line.rfind(' <')

    if idx1 == -1 or idx2 == -1:
        return 0

    sub = line[0:idx1 + 1]
    pred = line[idx1 + 1:idx2].strip()
    obj = line[idx2:]

    obj = codecs.decode(obj, 'unicode-escape')
    obj = unquote(unquote(unquote(obj))).strip()

    obj = obj.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if obj not in subjectLookup:
        return 0

    sub = codecs.decode(sub, 'unicode-escape')
    sub = unquote(unquote(unquote(sub))).strip()

    sub = sub.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if sub == obj:
        return 0

    if sub not in subjectLookup:
        return 0

    pred = codecs.decode(pred, 'unicode-escape')
    pred = unquote(unquote(unquote(pred))).strip()

    if pred not in labelsLookup:
        return 0

    subIdx = subjectLookup[sub]
    predIdx = labelsLookup[pred]
    objIdx = subjectLookup[obj]

    if subIdx not in edgeMapping:
        edgeMapping[subIdx] = dict()

    if predIdx not in edgeMapping[subIdx]:
        edgeMapping[subIdx][predIdx] = set()

    edgeMapping[subIdx][predIdx].add(objIdx)
    return 1


def processEdges(filePath, subjectLookup, labelsLookup, edgesOutFile):
    i = 0
    j = 0

    edgeMapping = {}

    try:
        with open(filePath, encoding="utf-8", errors="ignore") as f:
            for line in f:
                j += processLineForEdges(subjectLookup, labelsLookup, edgeMapping, line)
                i += 1
    except Exception as ex:
        print("Failed processing file:", filePath, i, ex, '\n', line, flush=True)

    print('reduced', i, "to", j, "edges", flush=True)

    with open(edgesOutFile, mode="w", encoding="utf-8") as out:
        out.write(str(len(subjectLookup)))
        out.write(',')
        out.write(str(j))
        out.write(',')
        out.write(str(len(labelsLookup)))
        out.write('\n')

        for source, labelList in edgeMapping.items():
            for label, targetList in labelList.items():
                for target in targetList:
                    out.write(str(source))
                    out.write(' ')
                    out.write(str(label))
                    out.write(' ')
                    out.write(str(target))
                    out.write(' .\n')


def processLineForEdgesNoLabels(subjectLookup, edgeMapping, line):
    line = line[0:-3]

    if not line.startswith('<'):
        return 0

    if not line.endswith('>'):
        return 0

    idx1 = line.find('> ')
    idx2 = line.rfind(' <')

    if idx1 == -1 or idx2 == -1:
        return 0

    sub = line[0:idx1 + 1]
    obj = line[idx2:]

    obj = codecs.decode(obj, 'unicode-escape')
    obj = unquote(unquote(unquote(obj))).strip()

    obj = obj.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if obj not in subjectLookup:
        return 0

    sub = codecs.decode(sub, 'unicode-escape')
    sub = unquote(unquote(unquote(sub))).strip()

    sub = sub.replace("http://linkedgeodata.org/triplify/", "http://linkedgeodata.org/geometry/")

    if sub == obj:
        return 0

    if sub not in subjectLookup:
        return 0

    subIdx = subjectLookup[sub]
    objIdx = subjectLookup[obj]

    if subIdx not in edgeMapping:
        edgeMapping[subIdx] = set()

    edgeMapping[subIdx].add(objIdx)
    return 1


def processEdgesNoLabels(filePath, subjectLookup, edgesOutFile):
    i = 0
    j = 0

    edgeMapping = {}

    try:
        with open(filePath, encoding="utf-8", errors="ignore") as f:
            for line in f:
                j += processLineForEdgesNoLabels(subjectLookup, edgeMapping, line)
                i += 1
    except Exception as ex:
        print("Failed processing file:", filePath, i, ex, '\n', line, flush=True)

    print('reduced', i, "to", j, "edges", flush=True)

    with open(edgesOutFile, mode="w", encoding="utf-8") as out:
        out.write(str(len(subjectLookup)))
        out.write(',')
        out.write(str(j))
        out.write(',1\n')

        for source, labelList in edgeMapping.items():
            for label, targetList in labelList.items():
                for target in targetList:
                    out.write(str(source))
                    out.write(' 0 ')
                    out.write(str(target))
                    out.write(' .\n')


def readFromTo(file, graphFile):
    """"
        Considers subjects as nodes and only creates the labels and edges between the subjects and targets that are
        defined within the subjects.
    """

    nodesOutFile = file.replace('.rdf', '.nodes.tsv')
    labelsOutFile = file.replace('.rdf', '.labels.tsv')

    subjectLookup = processNodes(file, nodesOutFile)

    print("Finished loading nodes")

    labelsLookup = processLabels(file, subjectLookup, labelsOutFile)

    print("Finished loading labels")

    processEdges(file, subjectLookup, labelsLookup, graphFile)


def readFromToDoubleSided(file, graphFile):
    """"
        Same as read from to however it also considers urls on the object side as nodes.
    """
    nodesOutFile = file.replace('.rdf', '.nodes.tsv')
    labelsOutFile = file.replace('.rdf', '.labels.tsv')

    subjectLookup = processDoubleSidedNodes(file, nodesOutFile)

    print("Finished loading nodes")

    labelsLookup = processLabels(file, subjectLookup, labelsOutFile)

    print("Finished loading labels")

    processEdges(file, subjectLookup, labelsLookup, graphFile)


def readFromToNoLabels(file, graphFile):
    nodesOutFile = file.replace('.rdf', '.nodes.tsv')

    subjectLookup = processNodes(file, nodesOutFile)

    print("Finished loading nodes")

    processEdgesNoLabels(file, subjectLookup, graphFile)


def readFromToDoubleSidedNoLabels(file, graphFile):
    nodesOutFile = file.replace('.rdf', '.nodes.tsv')

    subjectLookup = processDoubleSidedNodes(file, nodesOutFile)

    print("Finished loading nodes")

    processEdgesNoLabels(file, subjectLookup, graphFile)
