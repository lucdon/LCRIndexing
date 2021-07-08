from itertools import count
from collections import defaultdict

from sys import stdin

if __name__ == "__main__":
    c = count(0)
    d = defaultdict(lambda: next(c))
    m = ['activation', 'catalysis', 'expression', 'inhibition', 'reaction', 'binding', 'ptmod']
    m = dict(zip(m, range(len(m))))

    numLabels = len(m)
    edges = set()

    # download from: http://version10.string-db.org/download/protein.actions.v10/9606.protein.actions.v10.txt.gz
    with open('./workload/stringHS/9606.protein.actions.v10.txt') as f:
        for line in f:

            if len(line.split()) == 6:
                continue

            source, target, mode, a_is_acting, _ = line.split()

            if a_is_acting == 't':
                pass
            elif a_is_acting == 'f':
                source, target = target, source
            else:
                print('a_is_acting is', a_is_acting, ', unexpected')
                exit(1)

            source = d[source]
            target = d[target]
            label = m[mode]

            edges.add((source, label, target))

    with open('./workload/stringHS/graph.nt', mode='w', encoding='utf-8') as out:
        out.write(str(len(d)))
        out.write(',')
        out.write(str(len(edges)))
        out.write(',')
        out.write(str(numLabels))
        out.write('\n')

        for source, label, target in edges:
            out.write(f'{source} {label} {target} .\n')
