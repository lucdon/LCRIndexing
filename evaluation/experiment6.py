from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_7/")
    r2 = ResultSet("./benchmark-results/ran on server/benchmark_4/")
    r3 = ResultSet("./benchmark-results/ran on server/benchmark_5/")
    r4 = ResultSet("./benchmark-results/ran on server/benchmark_6/")
    r5 = ResultSet("./benchmark-results/ran on server/benchmark_8_1/")
    r6 = ResultSet("./benchmark-results/ran on server/benchmark_8_2/")
    r7 = ResultSet("./benchmark-results/ran on server/benchmark_10/")

    r2.dropIndex('LI+')
    # r2.dropIndex('KLC(PLL)')
    r2.dropIndex('KLCFreq(PLL)')
    r2.dropIndex('LWBF(custom, custom, 32)')

    r5.dropIndex('sh(klc-bfl, pll)')

    # r3.dropIndex('KLC(PLL)')

    r4.dropGraph('dbpedia')

    r3.joinLeft(r4)

    r4.dropGraph('socPokec')

    r2.merge(r3)
    r2.merge(r4)

    r5.append(r6)

    r.merge(r5)
    r.joinLeft(r7)

    r.append(r2)

    r.avgQueries()

    r.mergeQueriesOnResult()
    r.mergeQueriesOnType()

    r.mergeQueriesOnSmallMedLarge()

    r.replaceRealGraphNames()

    r.renameIndex('pruned-2-hop', 'P2H')
    r.renameIndex('KLC(PLL)', 'KLC')
    r.renameIndex('sh(klc, pll)', 'KLC/S')
    r.renameIndex('sh(klc-freq, pll)', 'KLC-Freq/S')
    r.renameIndex('sh(klc-bfl)', 'KLC-BFL/S')

    r.reorder(["BFS", "P2H", "KLC", "KLC/S", "KLC-Freq/S", "KLC-BFL/S"])

    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:real-all-creation',
                                          caption='Creation Time of P2H and KLC-BFL/S on all real data sets.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:real-all-theo-size',
                                caption='Theoretical Index size of P2H and KLC-BFL/S on all real data sets.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:real-all-phys-size',
                                      caption='Physical Index size of P2H and KLC-BFL/S on all real data sets.'))
    print(r.queryTimings[0].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-all-query-small',
                                               caption='Average Small Query Time of P2H and KLC-BFL/S on all real data sets.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-all-query-medium',
                                               caption='Average Medium Query Time of P2H and KLC-BFL/S on all real data sets.'))
    print(r.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:real-all-query-large',
                                               caption='Average Large Query Time of P2H and KLC-BFL/S on all real data sets.'))

