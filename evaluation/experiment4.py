from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

def getName(vertexCount, degree, L):
    return 'erV' + str(vertexCount) + 'kD' + str(degree) + 'L' + str(L) + 'exp'


def createGraphs(r, vertexCount, degree):
    rx = r.copy()

    degrees = [3, 5]
    vertexCounts = [100, 500]
    labels = [8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

    for d in degrees:
        for v in vertexCounts:
            if d == degree and v == vertexCount:
                continue

            for l in labels:
                rx.dropGraph(getName(v, d, l))

    for l in labels:
        rx.renameGraph(getName(vertexCount, degree, l), str(l))

    rx.reorderGraphs(["8", "16", "32", "64", "256", "512", "1024", "2048", "4096"])

    rx.replaceErrorsWithNaN()
    fig = rx.create2x3LineCharts()

    if not os.path.exists('./benchmark-plots/'):
        os.mkdir('./benchmark-plots/')

    fig.savefig('./benchmark-plots/plotV' + str(vertexCount) + 'KD' + str(degree) + 'er.png', dpi=100)


if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_3/")
    r_sh = ResultSet("./benchmark-results/ran on server/benchmark_3_sh/")

    k = ResultSet('./benchmark-results/ran on server/benchmark_3_er/')

    r.dropIndex('LI+')
    r.dropIndex('KLC(PLL)')

    r.append(r_sh)
    r.merge(k)

    r.avgQueries()

    r.renameIndex('pruned-2-hop', 'P2H')
    r.renameIndex('sh(klc, pll)', 'KLC/S')
    r.renameIndex('sh(klc-freq, pll)', 'KLC-Freq/S')
    r.renameIndex('sh(klc-bfl)', 'KLC-BFL/S')

    r.reorder(["BFS", "P2H", "KLC/S", "KLC-Freq/S", "KLC-BFL/S"])

    r.mergeQueriesOnResult()
    r.mergeQueriesOnSmallMedLarge()

    r.replaceRealGraphNames()

    r3 = r.copy()
    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:super-scale-creation',
                                          caption='Creation Time when scaling to higher number of labels.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:super-scale-theo-size',
                                caption='Theoretical Index size when scaling to higher number of labels.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:super-scale-phys-size',
                                      caption='Physical Index size when scaling to higher number of labels.'))
    print(r.queryTimings[0].dataFrame.to_latex(longtable=True, escape=False, label='tab:super-scale-query-small',
                                               caption='Average Small Query Time when scaling to higher number of labels.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:super-scale-query-medium',
                                               caption='Average Medium Query Time when scaling to higher number of labels.'))
    print(r.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:super-scale-query-large',
                                               caption='Average Large Query Time when scaling to higher number of labels.'))

    createGraphs(r3, 100, 3)
    createGraphs(r3, 100, 5)

    createGraphs(r3, 500, 3)
    createGraphs(r3, 500, 5)
