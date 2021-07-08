from itertools import count
from collections import defaultdict

if __name__ == "__main__":
    m = {'Master': 0, 'Apprentice': 1, 'Journeyer': 2, 'Observer': 3}

    c = count(0)
    id_of_user = defaultdict(lambda: next(c))

    numLabels = len(m)
    edges = set()

    # download from: http://tinyurl.com/gnexfoy
    with open('./workload/robots/robots_net-graph-2014-07-07.dot') as f:
        for line in f:
            line = line.replace('\r', '').replace('\n', '').strip()

            if line.startswith('/*'):
                continue

            if line.startswith('digraph'):
                continue

            if line.startswith('}'):
                continue

            splitLine = line.split(' -> ')

            u = splitLine[0]
            v = splitLine[1].split(' [')[0]

            label = splitLine[1].replace(v, '').strip().replace('[level="', '').replace('"];', '')

            if label not in m:
                print("unknown label:", label)
                continue

            u = id_of_user[u]
            v = id_of_user[v]

            label = m[label]

            edges.add((u, label, v))

    with open('./workload/robots/graph.nt', mode='w', encoding='utf-8') as out:
        out.write(str(len(id_of_user)))
        out.write(',')
        out.write(str(len(edges)))
        out.write(',')
        out.write(str(numLabels))
        out.write('\n')

        for source, label, target in edges:
            out.write(f'{source} {label} {target} .\n')
