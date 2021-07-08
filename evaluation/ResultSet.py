import os
import numpy as np
import pandas as pd
import csv
import format
import matplotlib.pyplot as plt


def __joinPart__(graphs, indexes, left, right, mode):
    if mode == "replace":
        for index in indexes:
            for graph in graphs:
                left.at[graph, index] = right[index][graph]

    if mode == "replace-when-invalid":
        for index in indexes:
            for graph in graphs:
                if left[index][graph] < 0:
                    left.at[graph, index] = right[index][graph]


def mergeForCategory(dfs, graph, category):
    i = 0

    for cat in category:
        if (cat.dataFrame.loc[graph] != -5).all():
            dfs[i].loc[graph] = cat.dataFrame.loc[graph]
            i += 1

            if i > 2:
                break


class ResultSet:
    def __init__(self, obj):
        if type(obj) is ResultSet:
            self.indexSizes = obj.indexSizes.copy()
            self.indexTrainMemory = obj.indexTrainMemory.copy()
            self.indexCreationTimings = obj.indexCreationTimings.copy()

            self.graphNames = obj.graphNames.copy()
            self.indexNames = obj.indexNames.copy()

            self.queryTimings = []

            for query in obj.queryTimings:
                self.queryTimings.append(query.copy())
        else:
            dir = obj

            if not os.path.exists(os.path.join(dir, "./indexSize.csv")):
                print("invalid directory: ", dir)
                exit(1)

            self.indexSizes = pd.read_csv(os.path.join(dir, "./indexSize.csv"), index_col=0)
            self.indexTrainMemory = pd.read_csv(os.path.join(dir, "./indexTrainMemory.csv"), index_col=0)
            self.indexCreationTimings = pd.read_csv(os.path.join(dir, "./indexCreationTime.csv"), index_col=0)

            self.graphNames = self.indexSizes.index.tolist()
            self.indexNames = self.indexSizes.columns.tolist()

            self.queryTimings = []

            for f in os.listdir(dir):
                if "queryTime" in f:
                    fullPath = os.path.join(dir, f)
                    self.queryTimings.append(QueryResult(f, fullPath))

    def mergeQueriesOnType(self):
        processed = set()

        for query in self.queryTimings:
            if query in processed:
                continue

            found = False

            for other in self.queryTimings:
                if other in processed:
                    continue

                if query.label == other.label and query.result == other.result and query.type != other.type:
                    query.mergeOnType(other)

                    processed.add(query)
                    processed.add(other)

                    found = True
                    break

            if not found:
                query.type = 'both'

        temp = self.queryTimings

        self.queryTimings = []

        for query in temp:
            if query.type != 'both':
                continue

            self.queryTimings.append(query)

    def mergeQueriesOnResult(self):
        processed = set()

        for query in self.queryTimings:
            if query in processed:
                continue

            found = False

            for other in self.queryTimings:
                if other in processed:
                    continue

                if query.label == other.label and query.result != other.result and query.type == other.type:
                    query.mergeOnResult(other)

                    processed.add(query)
                    processed.add(other)

                    found = True
                    break

            if not found:
                query.result = 'none'

        temp = self.queryTimings

        self.queryTimings = []

        for query in temp:
            if query.result != 'none':
                continue

            self.queryTimings.append(query)

    def mergeQueriesOnSmallMedLarge(self):
        categories = dict()

        for query in self.queryTimings:
            key = str(query.result) + '.' + str(query.type)

            if key not in categories:
                categories[key] = []

            categories[key].append(query)

        self.queryTimings = []

        for category in categories.values():
            smallDf = category[0].dataFrame.copy()
            mediumDf = category[0].dataFrame.copy()
            largeDf = category[0].dataFrame.copy()

            dfs = [smallDf, mediumDf, largeDf]

            category.sort(key=lambda x: x.label)

            for graph in self.graphNames:
                mergeForCategory(dfs, graph, category)

            category[0].dataFrame = smallDf
            category[0].label = 'small'
            self.queryTimings.append(category[0])

            category[1].dataFrame = mediumDf
            category[1].label = 'medium'
            self.queryTimings.append(category[1])

            category[2].dataFrame = largeDf
            category[2].label = 'large'
            self.queryTimings.append(category[2])

    def joinLeft(self, other, mode='replace'):
        graphs = set(self.graphNames).intersection(other.graphNames)
        indexes = set(self.indexNames).intersection(other.indexNames)

        __joinPart__(graphs, indexes, self.indexSizes, other.indexSizes, mode)
        __joinPart__(graphs, indexes, self.indexTrainMemory, other.indexTrainMemory, mode)
        __joinPart__(graphs, indexes, self.indexCreationTimings, other.indexCreationTimings, mode)

        for query in self.queryTimings:
            for otherQuery in other.queryTimings:
                if query.label == otherQuery.label and query.result == otherQuery.result and query.type == otherQuery.type:
                    __joinPart__(graphs, indexes, query.dataFrame, otherQuery.dataFrame, mode)
                    break

    def avgQueries(self):
        for query in self.queryTimings:
            query.averageTimings(self.graphNames)

    def convertToUnits(self, highlightBest=False):
        if highlightBest:
            bestValues = self.indexSizes[self.indexSizes > 0].min(axis=1)
            self.indexSizes = format.formatDataFrameWithBest(self.indexSizes, bestValues, format.formatMemory)

            bestValues = self.indexTrainMemory[self.indexTrainMemory > 0].min(axis=1)
            self.indexTrainMemory = format.formatDataFrameWithBest(self.indexTrainMemory, bestValues,
                                                                   format.formatMemory)

            bestValues = self.indexCreationTimings[self.indexCreationTimings > 0.001].min(axis=1)
            self.indexCreationTimings = format.formatDataFrameWithBest(self.indexCreationTimings, bestValues,
                                                                       format.formatTime)

            for query in self.queryTimings:
                bestValues = query.dataFrame[query.dataFrame > 0].min(axis=1)
                query.dataFrame = format.formatDataFrameWithBest(query.dataFrame, bestValues, format.formatTime)
        else:
            self.indexSizes = self.indexSizes.applymap(format.formatMemory)
            self.indexTrainMemory = self.indexTrainMemory.applymap(format.formatMemory)
            self.indexCreationTimings = self.indexCreationTimings.applymap(format.formatTime)

            for query in self.queryTimings:
                query.dataFrame = query.dataFrame.applymap(format.formatTime)

    def reorder(self, indexNamesInOrder):
        self.indexSizes = self.indexSizes[indexNamesInOrder]
        self.indexTrainMemory = self.indexTrainMemory[indexNamesInOrder]
        self.indexCreationTimings = self.indexCreationTimings[indexNamesInOrder]

        for query in self.queryTimings:
            query.dataFrame = query.dataFrame[indexNamesInOrder]

    def reorderGraphs(self, graphNamesInOrder):
        self.indexSizes = self.indexSizes.reindex(graphNamesInOrder)
        self.indexTrainMemory = self.indexTrainMemory.reindex(graphNamesInOrder)
        self.indexCreationTimings = self.indexCreationTimings.reindex(graphNamesInOrder)

        for query in self.queryTimings:
            query.dataFrame = query.dataFrame.reindex(graphNamesInOrder)

    def renameGraph(self, graphName, graphNewName):
        if graphName not in self.graphNames:
            return

        if graphNewName in self.graphNames:
            print("graph new name:", graphNewName, "already exists")
            exit(1)

        self.graphNames.remove(graphName)
        self.graphNames.append(graphNewName)

        self.indexSizes.rename(inplace=True, index={graphName: graphNewName})
        self.indexTrainMemory.rename(inplace=True, index={graphName: graphNewName})
        self.indexCreationTimings.rename(inplace=True, index={graphName: graphNewName})

        for query in self.queryTimings:
            query.dataFrame.rename(inplace=True, index={graphName: graphNewName})

    def renameIndex(self, indexName, indexNewName):
        if indexName not in self.indexNames:
            return

        if indexNewName in self.indexNames:
            print("graph new name:", indexNewName, "already exists")
            exit(1)

        self.indexNames.remove(indexName)
        self.indexNames.append(indexNewName)

        self.indexSizes.rename(inplace=True, columns={indexName: indexNewName})
        self.indexTrainMemory.rename(inplace=True, columns={indexName: indexNewName})
        self.indexCreationTimings.rename(inplace=True, columns={indexName: indexNewName})

        for query in self.queryTimings:
            query.dataFrame.rename(inplace=True, columns={indexName: indexNewName})

    def dropGraph(self, graphName):
        if graphName not in self.graphNames:
            return

        self.graphNames.remove(graphName)

        self.indexSizes.drop(inplace=True, index=graphName)
        self.indexTrainMemory.drop(inplace=True, index=graphName)
        self.indexCreationTimings.drop(inplace=True, index=graphName)

        for query in self.queryTimings:
            query.dataFrame.drop(inplace=True, index=graphName)

            if query.isAveraged and graphName in query.averageCounts:
                del query.averageCounts[graphName]

    def dropIndex(self, indexName):
        if indexName not in self.indexNames:
            return

        self.indexNames.remove(indexName)

        self.indexSizes.drop(inplace=True, columns=indexName)
        self.indexTrainMemory.drop(inplace=True, columns=indexName)
        self.indexCreationTimings.drop(inplace=True, columns=indexName)

        for query in self.queryTimings:
            query.dataFrame.drop(inplace=True, columns=indexName)

    def copy(self):
        return ResultSet(self)

    def replaceRealGraphNames(self):
        self.renameGraph('zhishi', 'ZH')
        self.renameGraph('socPokec', 'SP')
        self.renameGraph('wikipediaLinkFr', 'WLF')
        self.renameGraph('dbpedia', 'DP')
        self.renameGraph('swdf', 'SWDF')
        self.renameGraph('reddit', 'RE')
        self.renameGraph('notreDame', 'ND')
        self.renameGraph('wordnet', 'WN')
        self.renameGraph('wikiTalk', 'WT')
        self.renameGraph('patents', 'PT')
        self.renameGraph('wikitionary', 'WTE')
        self.renameGraph('lgd', 'LGD')
        self.renameGraph('robots', 'RT')
        self.renameGraph('advogato', 'ADG')
        self.renameGraph('wikiVote', 'WV')
        self.renameGraph('gnutella', 'GN')
        self.renameGraph('arxiv', 'AX')
        self.renameGraph('biograd', 'BG')
        self.renameGraph('socSlashdot', 'SSD')
        self.renameGraph('socEpinions', 'SE')
        self.renameGraph('email', 'EEU')
        self.renameGraph('stringHS', 'SHS')
        self.renameGraph('stringFC', 'SFC')
        self.renameGraph('webGoogle', 'WG')
        self.renameGraph('webStanford', 'WSF')
        self.renameGraph('webBerkStan', 'WBS')

    def append(self, other):
        self.indexNames = list(dict.fromkeys(self.indexNames + other.indexNames))

        self.indexSizes = self.indexSizes.join(other.indexSizes)
        self.indexTrainMemory = self.indexTrainMemory.join(other.indexTrainMemory)
        self.indexCreationTimings = self.indexCreationTimings.join(other.indexCreationTimings)

        for query in self.queryTimings:
            found = False

            for otherQuery in other.queryTimings:
                if query.result == otherQuery.result and query.type == otherQuery.type and query.label == otherQuery.label:
                    query.dataFrame = query.dataFrame.join(otherQuery.dataFrame)
                    found = True
                    break

            if not found:
                for indexName in self.indexNames:
                    if indexName not in query.dataFrame.columns:
                        query.dataFrame[indexName] = -5

        for query in other.queryTimings:
            found = False

            for otherQuery in self.queryTimings:
                if query.result == otherQuery.result and query.type == otherQuery.type and query.label == otherQuery.label:
                    found = True
                    break

            if not found:
                self.queryTimings.append(query)

                for indexName in self.indexNames:
                    if indexName not in query.dataFrame.columns:
                        query.dataFrame[indexName] = -5

    def merge(self, other):
        self.graphNames = list(dict.fromkeys(self.graphNames + other.graphNames))

        self.indexSizes = self.indexSizes.append(other.indexSizes)
        self.indexTrainMemory = self.indexTrainMemory.append(other.indexTrainMemory)
        self.indexCreationTimings = self.indexCreationTimings.append(other.indexCreationTimings)

        for query in self.queryTimings:
            found = False

            for otherQuery in other.queryTimings:
                if query.result == otherQuery.result and query.type == otherQuery.type and query.label == otherQuery.label:
                    query.dataFrame = query.dataFrame.append(otherQuery.dataFrame)
                    found = True
                    break

            if not found:
                for graphName in self.graphNames:
                    if graphName not in query.dataFrame.index:
                        query.dataFrame.loc[graphName] = -5

        for query in other.queryTimings:
            found = False

            for otherQuery in self.queryTimings:
                if query.result == otherQuery.result and query.type == otherQuery.type and query.label == otherQuery.label:
                    found = True
                    break

            if not found:
                self.queryTimings.append(query)

                for graphName in self.graphNames:
                    if graphName not in query.dataFrame.index:
                        query.dataFrame.loc[graphName] = -5

    def replaceErrorsWithNaN(self):
        self.indexSizes = self.indexSizes.applymap(format.replaceNaN)
        self.indexTrainMemory = self.indexTrainMemory.applymap(format.replaceNaN)
        self.indexCreationTimings = self.indexCreationTimings.applymap(format.replaceNaN)

        for query in self.queryTimings:
            query.dataFrame = query.dataFrame.applymap(format.replaceNaN)

    def create2x3LineCharts(self):
        fig, axes = plt.subplots(2, 3)
        fig.set_size_inches(6.4 * 2, 4.8 * 1.5)

        self.indexCreationTimings.plot(title="construction time", ylabel="time in ms", xlabel='labels', grid=True,
                                       ax=axes[0][0])
        self.indexSizes.plot(title="theoretical index size", ylabel="size in MBs", xlabel='labels', grid=True,
                             ax=axes[0][1])
        self.indexTrainMemory.plot(title="physical index size", ylabel="size in MBs", xlabel='labels', grid=True,
                                   ax=axes[0][2])

        self.queryTimings[0].dataFrame.plot(title="small query time", ylabel="time in ms", xlabel='labels', grid=True,
                                            ax=axes[1][0])
        self.queryTimings[1].dataFrame.plot(title="medium query time", ylabel="time in ms", xlabel='labels', grid=True,
                                            ax=axes[1][1])
        self.queryTimings[2].dataFrame.plot(title="large query time", ylabel="time in ms", xlabel='labels', grid=True,
                                            ax=axes[1][2])

        fig.tight_layout()
        return fig

    def createBarCharts(self, logy=None, rot=None, excludeBFS=None, includeSizesAndConstruction=True,
                        includeQueries=True):
        if logy is None or logy is False:
            logy = [False, False, False, False, False, False]
        elif logy is True:
            logy = [True, True, True, True, True, True]

        if excludeBFS is None or excludeBFS is False:
            excludeBFS = [False, False, False, False, False, False]
        elif excludeBFS is True:
            excludeBFS = [True, True, True, True, True, True]

        if includeSizesAndConstruction and includeQueries:
            fig, axes = plt.subplots(2, 3)
            fig.set_size_inches(6.4 * 2, 4.8 * 1.5)
        else:
            fig, axes = plt.subplots(1, 3)
            fig.set_size_inches(6.4 * 2, 4.8)

        copied = False
        toPlot = self

        if includeSizesAndConstruction and excludeBFS[0]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.indexCreationTimings['BFS'] = np.nan

        if includeSizesAndConstruction and excludeBFS[1]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.indexSizes['BFS'] = np.nan

        if includeSizesAndConstruction and excludeBFS[2]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.indexTrainMemory['BFS'] = np.nan

        idx = 3

        if not includeSizesAndConstruction:
            idx = 0

        if includeQueries and excludeBFS[idx]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.queryTimings[0].dataFrame['BFS'] = np.nan

        if includeQueries and excludeBFS[idx + 1]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.queryTimings[1].dataFrame['BFS'] = np.nan

        if includeQueries and excludeBFS[idx + 2]:
            if not copied:
                copied = True
                toPlot = self.copy()

            toPlot.queryTimings[2].dataFrame['BFS'] = np.nan

        if includeQueries and includeSizesAndConstruction:
            toPlot.indexCreationTimings.plot(title="construction time", logy=logy[0], kind='bar', ylabel="time in ms",
                                             xlabel='labels', rot=rot,
                                             grid=True,
                                             ax=axes[0][0])
            toPlot.indexSizes.plot(title="theoretical index size", logy=logy[1], kind='bar', ylabel="size in MBs",
                                   xlabel='labels', rot=rot,
                                   grid=True,
                                   ax=axes[0][1])
            toPlot.indexTrainMemory.plot(title="physical index size", logy=logy[2], kind='bar', ylabel="size in MBs",
                                         xlabel='labels', rot=rot,
                                         grid=True,
                                         ax=axes[0][2])
        elif includeSizesAndConstruction:
            toPlot.indexCreationTimings.plot(title="construction time", logy=logy[0], kind='bar', ylabel="time in ms",
                                             xlabel='labels', rot=rot,
                                             grid=True,
                                             ax=axes[0])
            toPlot.indexSizes.plot(title="theoretical index size", logy=logy[1], kind='bar', ylabel="size in MBs",
                                   xlabel='labels', rot=rot,
                                   grid=True,
                                   ax=axes[1])
            toPlot.indexTrainMemory.plot(title="physical index size", logy=logy[2], kind='bar', ylabel="size in MBs",
                                         xlabel='labels', rot=rot,
                                         grid=True,
                                         ax=axes[2])

        if includeQueries and includeSizesAndConstruction:
            toPlot.queryTimings[0].dataFrame.plot(title="small query time", logy=logy[3], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[1][0])
            toPlot.queryTimings[1].dataFrame.plot(title="medium query time", logy=logy[4], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[1][1])
            toPlot.queryTimings[2].dataFrame.plot(title="large query time", logy=logy[5], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[1][2])
        elif includeQueries:
            toPlot.queryTimings[0].dataFrame.plot(title="small query time", logy=logy[3], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[0])
            toPlot.queryTimings[1].dataFrame.plot(title="medium query time", logy=logy[4], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[1])
            toPlot.queryTimings[2].dataFrame.plot(title="large query time", logy=logy[5], kind='bar',
                                                  ylabel="time in ms", rot=rot,
                                                  xlabel='labels',
                                                  grid=True,
                                                  ax=axes[2])

        fig.tight_layout()
        return fig


class QueryResult:
    def __init__(self, name, fullName, obj=None):
        self.name = name
        self.fullName = fullName

        if obj is None:
            self.isAveraged = False
            self.averageCounts = dict()
            self.dataFrame = pd.read_csv(fullName, index_col=0)

            splitName = name.replace("queryTime.", '').replace(".csv", '').split('.')

            self.result = 'none'
            self.type = 'both'

            for segment in splitName:
                if segment.startswith('L'):
                    self.label = int(segment.replace('L', ''))

                if segment == 'false':
                    self.result = False

                if segment == 'true':
                    self.result = True

                if segment == 'rnd':
                    self.type = "random"

                if segment == 'cnn':
                    self.type = "connected"
        else:
            self.isAveraged = obj.isAveraged
            self.dataFrame = obj.dataFrame.copy()
            self.result = obj.result
            self.type = obj.type
            self.label = obj.label
            self.averageCounts = obj.averageCounts.copy()

    def copy(self):
        return QueryResult(self.name, self.fullName, self)

    def mergeOnType(self, other):
        self.type = 'both'

        if not self.isAveraged:
            print(self.dataFrame)
            print(other.dataFrame)
            self.dataFrame += other.dataFrame

            self.dataFrame.mask(self.dataFrame == -2, -1, inplace=True)
            self.dataFrame.mask(self.dataFrame == -4, -2, inplace=True)
            self.dataFrame.mask(self.dataFrame == -6, -3, inplace=True)
            self.dataFrame.mask(self.dataFrame == -8, -4, inplace=True)
            self.dataFrame.mask(self.dataFrame == -10, -5, inplace=True)
        else:
            for graphName in self.dataFrame.index:
                if graphName in self.averageCounts and graphName in other.averageCounts:
                    toReplace = self.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= self.averageCounts[graphName]

                    toReplace = other.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= other.averageCounts[graphName]

                    self.averageCounts[graphName] += other.averageCounts[graphName]
                elif graphName in self.averageCounts:
                    toReplace = self.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= self.averageCounts[graphName]
                    other.dataFrame.loc[graphName] = 0
                elif graphName in other.averageCounts:
                    toReplace = other.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= other.averageCounts[graphName]
                    self.dataFrame.loc[graphName] = 0

                    self.averageCounts[graphName] = other.averageCounts[graphName]

            self.dataFrame += other.dataFrame

            self.dataFrame.mask(self.dataFrame == -2, -1, inplace=True)
            self.dataFrame.mask(self.dataFrame == -4, -2, inplace=True)
            self.dataFrame.mask(self.dataFrame == -6, -3, inplace=True)
            self.dataFrame.mask(self.dataFrame == -8, -4, inplace=True)
            self.dataFrame.mask(self.dataFrame == -10, -5, inplace=True)

            for graphName in self.averageCounts.keys():
                toReplace = self.dataFrame.loc[graphName]
                toReplace.loc[toReplace > 0] /= self.averageCounts[graphName]

    def mergeOnResult(self, other):
        if not self.isAveraged:
            self.dataFrame += other.dataFrame

            self.dataFrame.mask(self.dataFrame == -2, -1, inplace=True)
            self.dataFrame.mask(self.dataFrame == -4, -2, inplace=True)
            self.dataFrame.mask(self.dataFrame == -6, -3, inplace=True)
            self.dataFrame.mask(self.dataFrame == -8, -4, inplace=True)
            self.dataFrame.mask(self.dataFrame == -10, -5, inplace=True)
        else:
            for graphName in self.dataFrame.index:
                if graphName in self.averageCounts and graphName in other.averageCounts:
                    toReplace = self.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= self.averageCounts[graphName]

                    toReplace = other.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= other.averageCounts[graphName]

                    self.averageCounts[graphName] += other.averageCounts[graphName]
                elif graphName in self.averageCounts:
                    toReplace = self.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= self.averageCounts[graphName]
                    other.dataFrame.loc[graphName] = 0
                elif graphName in other.averageCounts:
                    toReplace = other.dataFrame.loc[graphName]
                    toReplace.loc[toReplace > 0] *= other.averageCounts[graphName]

                    self.dataFrame.loc[graphName] = 0
                    self.averageCounts[graphName] = other.averageCounts[graphName]

            self.dataFrame += other.dataFrame

            self.dataFrame.mask(self.dataFrame == -2, -1, inplace=True)
            self.dataFrame.mask(self.dataFrame == -4, -2, inplace=True)
            self.dataFrame.mask(self.dataFrame == -6, -3, inplace=True)
            self.dataFrame.mask(self.dataFrame == -8, -4, inplace=True)
            self.dataFrame.mask(self.dataFrame == -10, -5, inplace=True)

            for graphName in self.averageCounts.keys():
                toReplace = self.dataFrame.loc[graphName]
                toReplace.loc[toReplace > 0] /= self.averageCounts[graphName]

        self.result = 'none'

    def averageTimings(self, graphNames):
        self.averageCounts = dict()
        self.isAveraged = True

        for graphName in graphNames:
            if (self.dataFrame.loc[graphName] == -5).any():
                continue

            if (graphName.startswith('erV') or graphName.startswith('ffV') or graphName.startswith(
                    'plV') or graphName.startswith('paV')) and graphName.endswith('exp'):
                self.averageTimingsSynth(graphName)
            else:
                self.averageTimingsReal(graphName)

    def averageTimingsSynth(self, graphName):
        dir = './workload/generated/' + graphName + '.queries-lcr'

        if self.type == 'random':
            dir += '.rnd'
        elif self.type == 'connected':
            dir += '.cnn'

        dir += '.L' + str(self.label)

        if self.result:
            dir += '.true'
        else:
            dir += '.false'

        dir += '.csv'

        self.averageTimingsFromQueriesFile(graphName, dir)

    def averageTimingsReal(self, graphName):
        dir = './workload/' + graphName + '/graph.queries-lcr'

        if self.type == 'random':
            dir += '.rnd'
        elif self.type == 'connected':
            dir += '.cnn'

        dir += '.L' + str(self.label)

        if self.result:
            dir += '.true'
        else:
            dir += '.false'

        dir += '.csv'

        self.averageTimingsFromQueriesFile(graphName, dir)

    def averageTimingsFromQueriesFile(self, graphName, dir):
        if not os.path.exists(dir):
            return

        file = open(dir)
        reader = csv.reader(file)
        count = len(list(reader))

        toReplace = self.dataFrame.loc[graphName]

        toReplace.loc[toReplace > 0] /= count
        self.averageCounts[graphName] = count

    def __repr__(self):
        return str(self.type) + '.' + str(self.label) + "." + str(self.result)
