import os
import subprocess
import sys

from utility import exeExtension
import glob


class LCRQueryGenerator:
    def __init__(self, randomQueryCount, connectedQueryCount, splitRandomFromConnected):
        self.executable = "./LCRQueryGenerator" + exeExtension()

        self.randomQueryCount = randomQueryCount
        self.splitRandomFromConnected = splitRandomFromConnected
        self.connectedQueryCount = connectedQueryCount

        if not os.path.exists(self.executable):
            self.executable = "./cmake-build-release/LCRQueryGenerator" + exeExtension()

    def generateAndEnumerateQueryModes(self, graphFileNames):
        queryGenModes = set()

        for graphFileName in graphFileNames:
            if not self.generateQueries(graphFileName):
                print("failed generating queries for graph file:", graphFileName)
                sys.exit(1)

            fileWithoutExt = graphFileName[:graphFileName.rfind('.')].replace('\\', '/')
            queryFiles = glob.glob(fileWithoutExt + '.*.csv')

            for f in queryFiles:
                genMode = f.replace('\\', '/').replace(fileWithoutExt, '').replace('.csv', '').replace('.queries-lcr.',
                                                                                                       '')
                queryGenModes.add(genMode)

        return queryGenModes

    def generateQueries(self, graphFile):
        fileWithoutExt = graphFile[:graphFile.rfind('.')]

        queryFiles = glob.glob(fileWithoutExt + '.*.csv')

        if len(queryFiles) > 0:
            return True

        runnable = [self.executable, "--graphFile", graphFile, "--randomQueries", str(self.randomQueryCount),
                    "--connectedQueries", str(self.connectedQueryCount)]

        if self.splitRandomFromConnected:
            runnable.append("--splitRandomFromConnected")

        print("generating queries:", graphFile, flush=True)

        result = subprocess.run(runnable, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        success = result.returncode == 0

        if not success:
            error = result.stderr.decode("utf-8")
            print("error: ")
            print(error)
            print()

        return success
