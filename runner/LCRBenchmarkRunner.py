import os
import subprocess
import sys

import pandas as pd
from utility import parseTime, parseMemory, exeExtension


class LCRBenchmarkRunner:
    def __init__(self, indexNames, graphNames, labelGenModes):
        self.executable = "./MasterThesis" + exeExtension()

        if not os.path.exists(self.executable):
            self.executable = "./cmake-build-release/MasterThesis" + exeExtension()

        self.memoryLimit = 128 * 1000  # 128 GB
        self.timeout = 6 * 60 * 60  # 6 hours

        self.graphNames = graphNames
        self.indexNames = indexNames

        self.labelGenModes = labelGenModes
        self.queryTimings = dict()

        self.reset()

    def reset(self):
        # columns are index Names
        # rows are graphNames
        self.indexSizes = pd.DataFrame(index=self.graphNames, columns=self.indexNames)
        self.indexTrainMemory = pd.DataFrame(index=self.graphNames, columns=self.indexNames)
        self.indexCreationTimings = pd.DataFrame(index=self.graphNames, columns=self.indexNames)

        for labelGenMode in self.labelGenModes:
            self.queryTimings[labelGenMode] = pd.DataFrame(index=self.graphNames, columns=self.indexNames)

        # -1 means not filled in
        # -2 means hit memory limit
        # -3 means hit time limit
        # -4 unknown error
        self.indexSizes.fillna(-1.0, inplace=True)
        self.indexTrainMemory.fillna(-1.0, inplace=True)
        self.indexCreationTimings.fillna(-1.0, inplace=True)

        for labelGenMode in self.labelGenModes:
            self.queryTimings[labelGenMode].fillna(-1.0, inplace=True)

    def setTimeLimit(self, timeLimitInSeconds):
        self.timeLimit = timeLimitInSeconds

    def setMemoryLimit(self, memoryLimitInMegaBytes):
        self.memoryLimit = memoryLimitInMegaBytes

    def save(self):
        if not os.path.exists("./benchmark-results/"):
            os.mkdir("./benchmark-results/")

        self.indexSizes.to_csv("./benchmark-results/indexSize.csv")
        self.indexTrainMemory.to_csv("./benchmark-results/indexTrainMemory.csv")
        self.indexCreationTimings.to_csv("./benchmark-results/indexCreationTime.csv")

        for labelGenMode in self.labelGenModes:
            self.queryTimings[labelGenMode].to_csv("./benchmark-results/queryTime." + labelGenMode + ".csv")

    def load(self):
        if not os.path.exists("./benchmark-results/indexSize.csv"):
            return False

        if not os.path.exists("./benchmark-results/indexTrainMemory.csv"):
            return False

        if not os.path.exists("./benchmark-results/indexCreationTime.csv"):
            return False

        self.indexSizes = pd.read_csv("./benchmark-results/indexSize.csv", index_col=0)
        self.indexTrainMemory = pd.read_csv("./benchmark-results/indexTrainMemory.csv", index_col=0)
        self.indexCreationTimings = pd.read_csv("./benchmark-results/indexCreationTime.csv", index_col=0)

        for labelGenMode in self.labelGenModes:
            path = "./benchmark-results/queryTime." + labelGenMode + ".csv"

            if not os.path.exists(path):
                return False

            self.queryTimings[labelGenMode] = pd.read_csv(path, index_col=0)

        if len(self.indexSizes.columns) != len(self.indexNames):
            self.reset()
            return False

        for column in self.indexSizes.columns:
            if column not in self.indexNames:
                self.reset()
                return False

        for index in self.indexSizes.index:
            if index not in self.graphNames:
                self.reset()
                return False

        return True

    def runAndCollect(self, index, indexParams, graphFile, graphName):
        indexName = (index + "(" + str.join(", ", indexParams) + ")").replace("()", "")
        runnable = [self.executable, "lcr", "--index", index]

        runnable.append("--indexParams")

        for indexParam in indexParams:
            runnable.append(str(indexParam))

        runnable.append("--graphFile")
        runnable.append(graphFile)

        for labelGenMode in self.labelGenModes:
            queryFile = graphFile.replace(".nt", ".queries-lcr." + labelGenMode + ".csv")

            if os.path.exists(queryFile):
                runnable.append("--queryFile")
                runnable.append(queryFile)

        runnable.append("--timeLimit")
        runnable.append(str(self.timeLimit))
        runnable.append("--memoryLimit")
        runnable.append(str(self.memoryLimit))

        print("running:", indexName, graphName, flush=True)

        result = subprocess.run(runnable, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        success = result.returncode == 0
        text = result.stdout.decode("utf-8")

        if not success:
            error = result.stderr.decode("utf-8")
            errorCode = -4.0

            if "memory limit" in error:
                errorCode = -2.0
            elif "time limit" in error:
                errorCode = -3.0
            else:
                print("statusCode: ")
                print(result.returncode)
                print("\nstandard: ")
                print(text)
                print("\nerror: ")
                print(error)
                print()
                sys.exit(1)

            self.indexSizes.at[graphName, indexName] = errorCode
            self.indexTrainMemory.at[graphName, indexName] = errorCode
            self.indexCreationTimings.at[graphName, indexName] = errorCode

            for labelGenMode in self.labelGenModes:
                self.queryTimings[labelGenMode].at[graphName, indexName] = errorCode

            # save the results, for when we want to quit mid-run in the future.
            self.save()
            return success, errorCode

        trainingTimingIndex = text.find("Training timings:")
        indexUsedIndex = text.find("Indexes used:")
        queryTimingsIndex = text.find("Query timings:")

        text1 = text[trainingTimingIndex:indexUsedIndex].replace("Training timings:", "").strip()
        text2 = text[indexUsedIndex:queryTimingsIndex].replace("Indexes used:", "").strip()
        text3 = text[queryTimingsIndex:].strip()

        text1 = text1[text1.find("Took:"):]
        text2 = text2[text2.find("size:"):]

        text1MemoryIndex = text1.find("Memory:")

        trainingTime = parseTime(text1[:text1MemoryIndex].replace("Took:", "").strip())
        trainingMemory = parseMemory(text1[text1MemoryIndex:].replace("Memory:", "").strip())

        indexSize = parseMemory(text2.replace("size:", "").strip())

        self.indexSizes.at[graphName, indexName] = indexSize
        self.indexTrainMemory.at[graphName, indexName] = trainingMemory
        self.indexCreationTimings.at[graphName, indexName] = trainingTime

        for labelGenMode in self.labelGenModes:
            queryFileName = graphFile.replace(".nt", ".queries-lcr." + labelGenMode + ".csv")

            queryTimingIndex = text3.find("Query timings: " + queryFileName)

            if queryTimingIndex == -1:
                self.queryTimings[labelGenMode].at[graphName, indexName] = -5
                continue

            querySectionStart = text3[queryTimingIndex:].replace("\r", "")
            querySection = querySectionStart[:querySectionStart.find("\n\n")] \
                .replace("Query timings: " + queryFileName, "") \
                .replace("\n", "")

            querySection = querySection[querySection.find("took:"):]
            queryTime = parseTime(querySection[:querySection.find("min:")].replace("took:", "").strip())
            self.queryTimings[labelGenMode].at[graphName, indexName] = queryTime

        # save the results, for when we want to quit mid-run in the future.
        self.save()
        return success, -1

    def setPreviousFailed(self, index, indexParams, graphFile, graphName, errorCode):
        indexName = (index + "(" + str.join(", ", indexParams) + ")").replace("()", "")

        self.indexSizes.at[graphName, indexName] = errorCode
        self.indexTrainMemory.at[graphName, indexName] = errorCode
        self.indexCreationTimings.at[graphName, indexName] = errorCode

        for labelGenMode in self.labelGenModes:
            queryFileName = graphFile.replace(".nt", ".queries-lcr." + labelGenMode + ".csv")

            if os.path.exists(queryFileName):
                self.queryTimings[labelGenMode].at[graphName, indexName] = errorCode
            else:
                self.queryTimings[labelGenMode].at[graphName, indexName] = -5

        self.save()
