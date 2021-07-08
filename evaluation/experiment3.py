from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_2/")
    r_fix = ResultSet("./benchmark-results/ran on server/benchmark_2_fix/")
    r_sh = ResultSet("./benchmark-results/ran on server/benchmark_2_sh/")
    r_sh_li = ResultSet("./benchmark-results/ran on server/benchmark_2_sh_li/")

    r.joinLeft(r_fix)

    r.dropIndex("KLC(PLL)")
    r.dropIndex("KLCFreq(PLL)")
    r.dropIndex("KLCBFL(4)")
    r.dropIndex("pruned-2-hop-rec-sec")

    r.renameIndex("LI+", "sh(li+, custom, 20)")
    r.renameIndex("LWBF(custom, custom, 32)", "sh(LWBF, custom, custom, 32)")

    r.append(r_sh)
    r.joinLeft(r_sh_li)

    r.avgQueries()

    r.renameIndex('pruned-2-hop', 'P2H')
    r.renameIndex('sh(li+, custom, 20)', 'LI+/S')
    r.renameIndex('sh(LWBF, custom, custom, 32)', 'LwBF/S')
    r.renameIndex('sh(klc, pll)', 'KLC/S')
    r.renameIndex('sh(klc-freq, pll)', 'KLC-Freq/S')
    r.renameIndex('sh(klc-bfl)', 'KLC-BFL/S')

    r.reorder(["BFS", "P2H", 'LI+/S', 'LwBF/S', 'KLC/S', 'KLC-Freq/S', 'KLC-BFL/S'])

    r.mergeQueriesOnResult()
    r.mergeQueriesOnSmallMedLarge()

    r.replaceRealGraphNames()

    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:scale-creation',
                                          caption='Creation Time of Scaling indexes.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:scale-theo-size',
                                caption='Theoretical Index size of Scaling indexes.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:scale-phys-size',
                                      caption='Physical Index size of Scaling indexes.'))
    print(r.queryTimings[0].dataFrame.to_latex(longtable=True, escape=False, label='tab:scale-query-small',
                                               caption='Average Small Query Time of Scaling indexes.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:scale-query-medium',
                                               caption='Average Medium Query Time of Scaling indexes.'))
    print(r.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:scale-query-large',
                                               caption='Average Large Query Time of Scaling indexes.'))
