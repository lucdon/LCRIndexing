from ResultSet import ResultSet
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == "__main__":
    r = ResultSet("./benchmark-results/ran on server/benchmark_1/")

    r.avgQueries()

    r.dropIndex('pruned-2-hop-rec-sec')
    r.dropIndex('ALC(PLL)')
    r.dropIndex('KLC(PLL)')
    r.dropIndex('KLCFreq(PLL)')
    r.dropIndex('KLCBFL(4)')
    r.dropIndex('BGMI(4)')
    r.dropIndex('BFFLP(4)')
    r.dropIndex('pruned-2-hop')
    r.dropIndex('LI+')

    r.dropGraph('erV10kD3L2exp')
    r.dropGraph('erV10kD5L2exp')
    r.dropGraph('erV25kD3L2exp')
    r.dropGraph('erV25kD5L2exp')

    r.renameIndex('LWBF(custom, custom, 32)', 'LwBF')
    r.renameIndex('BGI(4)', 'BFV')
    r.renameIndex('BPI(4)', 'BFV-OL')
    r.renameIndex('BFI(4)', 'BFV-LF')

    r.reorder(["BFS", "BFV", "BFV-OL", "BFV-LF", "LwBF"])

    r2 = r.copy()
    r.mergeQueriesOnResult()

    r3 = r.copy()
    r4 = r2.copy()
    r2.convertToUnits(highlightBest=True)
    r.convertToUnits(highlightBest=True)

    for i in range(50):
        print()

    print(r.indexCreationTimings.to_latex(longtable=True, escape=False, label='tab:bloom-creation',
                                          caption='Creation Time of the Bloom filter indexing techniques.'))
    print(r.indexSizes.to_latex(longtable=True, escape=False, label='tab:bloom-theo-size',
                                caption='Theoretical Index size of the Bloom filter indexing techniques.'))
    print(r.indexTrainMemory.to_latex(longtable=True, escape=False, label='tab:bloom-phys-size',
                                      caption='Physical Index size of the Bloom filter indexing techniques.'))
    print(r.queryTimings[1].dataFrame.to_latex(longtable=True, escape=False, label='tab:bloom-query',
                                               caption='Average Query Time of the Bloom filter indexing techniques. Conducted at $|L|=2$ with both true and false queries.'))
    print(r2.queryTimings[2].dataFrame.to_latex(longtable=True, escape=False, label='tab:bloom-query-false',
                                                caption='Average Query Time of the Bloom filter indexing techniques. Conducted at $|L|=2$ with only false queries.'))

    r3.mergeQueriesOnSmallMedLarge()
    r3.replaceErrorsWithNaN()
    fig = r3.createBarCharts(logy=[False, True, True, False, False, False], rot=70,
                             excludeBFS=[True, True, True, False, False, False])
    if not os.path.exists('./benchmark-plots/'):
        os.mkdir('./benchmark-plots/')

    fig.savefig('./benchmark-plots/experiment1.png', dpi=100)

    r4.mergeQueriesOnSmallMedLarge()
    r4.replaceErrorsWithNaN()

    fig = r4.createBarCharts(includeSizesAndConstruction=False, rot=70, logy=[False, True, True, False, False, False],
                             excludeBFS=[True, True, True, False, False, False])
    if not os.path.exists('./benchmark-plots/'):
        os.mkdir('./benchmark-plots/')

    fig.savefig('./benchmark-plots/experiment1-false.png', dpi=100)
