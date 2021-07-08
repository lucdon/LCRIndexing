from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_2/")
    r_fix = ResultSet("./benchmark-results/ran on server/benchmark_2_fix/")

    r.joinLeft(r_fix)

    r.dropIndex("pruned-2-hop-rec-sec")
    r.dropIndex("LWBF(custom, custom, 32)")

    r.avgQueries()

    r.renameIndex('KLC(PLL)', 'KLC')
    r.renameIndex('KLCFreq(PLL)', 'KLC-Freq')
    r.renameIndex('pruned-2-hop', 'P2H')
    r.renameIndex('KLCBFL(4)', "KLC-BFL")

    r.reorder(["BFS", "LI+", "P2H", "KLC", "KLC-Freq", "KLC-BFL"])

    r.mergeQueriesOnResult()
    r.mergeQueriesOnSmallMedLarge()

    r.replaceRealGraphNames()

    r3 = r.copy()
    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:klc-creation',
                                          caption='Creation Time of KLC based indexes compared to BFS, LI+ and P2H.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:klc-theo-size',
                                caption='Theoretical Index size of KLC based indexes compared to BFS, LI+ and P2H.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:klc-phys-size',
                                      caption='Physical Index size of KLC based indexes compared to BFS, LI+ and P2H.'))
    print(r.queryTimings[0].dataFrame.to_latex(longtable=True, escape=False, label='tab:klc-query-small',
                                               caption='Average Small Query Time of KLC based indexes compared to BFS, LI+ and P2H.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:klc-query-medium',
                                               caption='Average Medium Query Time of KLC based indexes compared to BFS, LI+ and P2H.'))
    print(r.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:klc-query-large',
                                               caption='Average Large Query Time of KLC based indexes compared to BFS, LI+ and P2H.'))

    r3.indexSizes.plot.bar()
    plt.show()