import os


class ContinueState:
    def __init__(self):
        self.file = "./benchmark-state/last.tsv"

        if not os.path.exists("./benchmark-state/"):
            os.mkdir("./benchmark-state/")

        pass

    def saveState(self, namedPairList):
        with open(self.file, "w") as f:
            for name, value in namedPairList.items():
                f.write(str(name))
                f.write("\t")
                f.write(str(value))
                f.write("\n")

    def hasStateToContinue(self):
        return os.path.exists(self.file)

    def loadState(self):
        namedPairList = {}

        with open(self.file, "r") as f:
            for line in f:
                pair = line.strip().strip("\n").split('\t')
                namedPairList[pair[0]] = pair[1]

        return namedPairList

    def cleanup(self):
        os.remove(self.file)
