from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_4/")

    r.avgQueries()

    r.renameIndex('KLC(PLL)', 'KLC')
    r.renameIndex('KLCFreq(PLL)', 'KLC-Freq')
    r.renameIndex('pruned-2-hop', 'P2H')
    r.renameIndex('LWBF(custom, custom, 32)', 'LwBF')

    r.reorder(["BFS", "LI+", "P2H", "KLC", "KLC-Freq", "LwBF"])

    r.mergeQueriesOnResult()
    r.mergeQueriesOnType()
    r.mergeQueriesOnSmallMedLarge()

    r.replaceRealGraphNames()

    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:real-creation',
                                          caption='Creation Time of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:real-theo-size',
                                caption='Theoretical Index size of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:real-phys-size',
                                      caption='Physical Index size of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))
    print(r.queryTimings[0].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-query-small',
                                               caption='Average Small Query Time of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-query-medium',
                                               caption='Average Medium Query Time of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))
    print(r.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-query-large',
                                               caption='Average Large Query Time of LwBF, KLC, KLC-Freq, LI+ and P2H on real data sets.'))