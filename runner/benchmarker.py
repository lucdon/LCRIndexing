import os
import itertools
import sys
import datetime

import ContinueState
import LCRQueryGenerator
import LCRBenchmarkRunner
import graphGenerator


def toName(V, L, degree, dist, model):
    name = ''

    if model == "er":
        name = model + "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist
    elif model == "pa":
        name = "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist
    elif model == "ff":
        name = model + "V" + str(int(V / 1000)) + "k" + str(degree) + "L" + str(L) + dist
    elif model == "pl":
        alpha = 1.95
        name = model + "V" + str(int(V / 1000)) + "ka" + str(alpha) + "L" + str(L) + dist
    else:
        print("Model must be one of: [er, pa, ff, pl]")
        sys.exit(1)

    return name


def toNameAndGenerate(V, L, degree, dist, model):
    name = ''

    if model == "er":
        name = model + "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist
    elif model == "pa":
        name = model + "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist
    elif model == "ff":
        name = model + "V" + str(int(V / 1000)) + "k" + str(degree) + "L" + str(L) + dist
    elif model == "pl":
        alpha = 1.95
        name = model + "V" + str(int(V / 1000)) + "ka" + str(alpha) + "L" + str(L) + dist
    else:
        print("Model must be one of: [er, pa, ff, pl]")
        sys.exit(1)

    fileName = "./workload/generated/" + name + ".nt"

    if not os.path.exists(fileName):
        print("generating graph:", fileName)
        graphGenerator.generate(V, degree, L, dist, model, "./workload/generated/")

    return name


def runSynth(indexNameAndParamsList, nodeSizes, labelSizes, degrees, models=None, difficulty=1):
    if models is None:
        models = ['er']

    print("start at:", datetime.datetime.now())

    labelDistributions = ["exp"]

    indexNames = [(indexNameAndParams[0] + "(" + str.join(", ", indexNameAndParams[1]) + ")").replace("()", "") for
                  indexNameAndParams in
                  indexNameAndParamsList]

    graphNames = [
        toNameAndGenerate(nodeSize, labelSize, degree, labelDistribution, model)
        for nodeSize, labelSize, degree, labelDistribution, model
        in itertools.product(nodeSizes, labelSizes, degrees, labelDistributions, models)
    ]

    graphFileNames = ["./workload/generated/" + graphName + ".nt" for graphName in graphNames]

    generator = LCRQueryGenerator.LCRQueryGenerator(50000, 50000, False)
    genModes = generator.generateAndEnumerateQueryModes(graphFileNames)

    runner = LCRBenchmarkRunner.LCRBenchmarkRunner(indexNames, graphNames, genModes)
    continueState = ContinueState.ContinueState()

    if sys.platform.startswith("win32"):
        runner.setTimeLimit(30 * 60)  # 30 minutes
        runner.setMemoryLimit(25 * 1000)  # 25 GB
    else:
        runner.setTimeLimit(1.5 * difficulty * 60 * 60)  # 3 hours
        runner.setMemoryLimit(70 * 1000)  # 70 GB

    currentIndex = 0
    currentNodeSize = 0
    currentLabelSize = 0
    currentDegree = 0
    currentLabelDistribution = 0
    currentModel = 0

    if continueState.hasStateToContinue():
        if runner.load():
            state = continueState.loadState()

            currentIndex = int(state["currentIndex"])
            currentNodeSize = int(state["currentNodeSize"])
            currentLabelSize = int(state["currentLabelSize"])
            currentDegree = int(state["currentDegree"])
            currentLabelDistribution = int(state["currentLabelDistribution"])
            currentModel = int(state["currentModel"])
        else:
            continueState.cleanup()

    for i in range(currentIndex, len(indexNameAndParamsList)):
        indexNameAndParams = indexNameAndParamsList[i]
        indexName = indexNameAndParams[0]
        indexParams = indexNameAndParams[1]

        lastReturnStatusCode = -1
        failedAtLabels = -1
        failedAtDegree = -1

        if indexName == "ALC":
            # ACL generates all label combinations
            # which means 2^16 label combinations is the max we try
            # which is already very likely to fail
            failedAtLabels = 17
            failedAtDegree = 1
            lastReturnStatusCode = -2

        for j in range(currentNodeSize, len(nodeSizes)):
            nodeSize = nodeSizes[j]

            for k in range(currentLabelSize, len(labelSizes)):
                labelSize = labelSizes[k]

                for m in range(currentDegree, len(degrees)):
                    degree = degrees[m]

                    for n in range(currentLabelDistribution, len(labelDistributions)):
                        labelDistribution = labelDistributions[n]

                        for o in range(currentModel, len(models)):
                            model = models[o]

                            continueState.saveState({
                                "currentIndex": i,
                                "currentNodeSize": j,
                                "currentLabelSize": k,
                                "currentModel": o,
                                "currentDegree": m,
                                "currentLabelDistribution": n,
                            })

                            graphName = toName(nodeSize, labelSize, degree, labelDistribution, model)
                            graphFileName = os.path.join("./workload/generated/", graphName + ".nt")

                            if failedAtLabels != -1 and failedAtDegree != -1 and labelSize >= failedAtLabels and degree >= failedAtDegree:
                                # if it failed at 25k vertices for 16 labels then no need to check 16 labels for 50k
                                runner.setPreviousFailed(indexName, indexParams, graphFileName, graphName,
                                                         lastReturnStatusCode)
                                break

                            success, statusCode = runner.runAndCollect(indexName, indexParams, graphFileName, graphName)

                            if not success:
                                # if it fails for current labelSize, then no need to continue
                                failedAtLabels = labelSize
                                failedAtDegree = degree
                                lastReturnStatusCode = statusCode
                                break

                        currentModel = 0
                    currentLabelDistribution = 0
                currentDegree = 0
            currentLabelSize = 0
        currentNodeSize = 0

    continueState.cleanup()
    print("finished at:", datetime.datetime.now())


def runReal(indexNameAndParamsList, graphs, graphsOnlyOnSuccess, difficulty=1):
    currentIndex = 0

    indexNames = [(indexNameAndParams[0] + "(" + str.join(", ", indexNameAndParams[1]) + ")").replace("()", "") for
                  indexNameAndParams in
                  indexNameAndParamsList]

    generator = LCRQueryGenerator.LCRQueryGenerator(50000, 50000, True)
    genModes = generator.generateAndEnumerateQueryModes(
        ["./workload/" + graphName + "/graph.nt" for graphName in graphs + graphsOnlyOnSuccess])

    runner = LCRBenchmarkRunner.LCRBenchmarkRunner(indexNames, graphs + graphsOnlyOnSuccess, genModes)
    continueState = ContinueState.ContinueState()

    if sys.platform.startswith("win32"):
        runner.setTimeLimit(30 * 60)  # 30 minutes
        runner.setMemoryLimit(25 * 1000)  # 25 GB
    else:
        runner.setTimeLimit(3 * difficulty * 60 * 60)  # 6 hours
        runner.setMemoryLimit(70 * 1000)  # 70 GB

    currentIndex = 0
    currentGraph = 0

    if continueState.hasStateToContinue():
        if runner.load():
            state = continueState.loadState()

            currentIndex = int(state["currentIndex"])
            currentGraph = int(state["currentGraph"])
        else:
            continueState.cleanup()

    for i in range(currentIndex, len(indexNameAndParamsList)):
        indexNameAndParams = indexNameAndParamsList[i]
        indexName = indexNameAndParams[0]
        indexParams = indexNameAndParams[1]

        lastReturnStatusCode = -1

        if currentGraph < len(graphs):
            for j in range(currentGraph, len(graphs)):
                graphName = graphs[j]
                graphFileName = "./workload/" + graphName + "/graph.nt"

                continueState.saveState({
                    "currentIndex": i,
                    "currentGraph": j
                })

                success, statusCode = runner.runAndCollect(indexName, indexParams, graphFileName, graphName)

                if not success:
                    lastReturnStatusCode = statusCode

            currentGraph = 0
        else:
            currentGraph -= len(graphs)

        for j in range(currentGraph, len(graphsOnlyOnSuccess)):
            graphName = graphsOnlyOnSuccess[j]
            graphFileName = "./workload/" + graphName + "/graph.nt"

            continueState.saveState({
                "currentIndex": i,
                "currentGraph": j + len(graphs)
            })

            if lastReturnStatusCode != -1:
                runner.setPreviousFailed(indexName, indexParams, graphFileName, graphName,
                                         lastReturnStatusCode)
                continue

            runner.runAndCollect(indexName, indexParams, graphFileName, graphName)

        currentGraph = 0

    continueState.cleanup()
    print("finished at:", datetime.datetime.now())
