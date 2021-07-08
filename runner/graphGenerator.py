import sys, random, math, snap, os

def augmentGraphWithLabels(edges, dist, numLabels):
    random.seed()
    mean = int(math.floor(numLabels / 2))
    sd = int(max(1, math.floor(numLabels / 4)))

    dgraph = []
    label = 0

    for source, targetList in edges.items():
        for target in targetList:
            if dist == "norm":
                label = random.normalvariate(mean, sd)
            if dist == "uni":
                label = random.uniform(0, numLabels)
            if dist == "exp":
                label = random.expovariate(1.0 / numLabels / 1.7)

            label = max(0, label)
            label = min(numLabels - 1, label)
            label = int(math.floor(label))

            triple = (source, target, label)
            dgraph.append(triple)

    return dgraph

def generate(V, degree, L, dist, model, fileLocation, skipIfExists=False):
    E = degree * V

    if E > (V * (V - 1)):
        print("too many edges for the number of nodes")
        return

    if L <= 0:
        print("Invalid number of labels")
        return

    if model == "er":
        isDirected = True  # We use Erdos-Renyi
        name = model + "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist + ".nt"
    elif model == "pa":
        name = model + "V" + str(int(V / 1000)) + "kD" + str(degree) + "L" + str(L) + dist + ".nt"
        isDirected = False  # We use preferential attachment
    elif model == "ff":
        name = model + "V" + str(int(V / 1000)) + "k" + str(degree) + "L" + str(L) + dist + ".nt"
        isDirected = True  # We use forest fire
    elif model == "pl":
        alpha = 1.95
        isDirected = False
        name = model + "V" + str(int(V / 1000)) + "ka" + str(alpha) + "L" + str(L) + dist + ".nt"
    else:
        print("Model must be one of: [er, pa, ff, pl]")
        return

    fileName = os.path.join(fileLocation, name)

    if not os.path.exists(fileLocation):
        os.mkdir(fileLocation)

    if os.path.exists(fileName) and skipIfExists:
        # already exists, so do not generate.
        return

    Rnd = snap.TRnd()

    if model == "er":
        UGraph = snap.GenRndGnm(snap.PNGraph, V, E, True, Rnd)
    elif model == "pa":
        UGraph = snap.GenPrefAttach(V, degree, Rnd)
    elif model == "ff":
        UGraph = snap.GenForestFire(V, 0.4, 0.2)
    elif model == "pl":
        UGraph = snap.GenRndPowerLaw(V, alpha)
    else:
        print("Model must be one of: [er, pa, ff, pl]")
        return

    with open(fileName, 'w') as f:
        random.seed()
        mean = int(math.floor(L / 2))
        sd = int(max(1, math.floor(L / 4)))

        dgraph = []

        for EI in UGraph.Edges():
            if dist == "norm":
                label = random.normalvariate(mean, sd)
            if dist == "uni":
                label = random.uniform(0, L)
            if dist == "exp":
                label = random.expovariate(1.0 / L / 1.7)

            label = max(0, label)
            label = min(L - 1, label)
            label = int(math.floor(label))

            if not isDirected:
                direction = random.uniform(0, 1)
            else:
                direction = 0.0

            if direction < 0.5:
                triple = (EI.GetSrcNId(), EI.GetDstNId(), label)
            else:
                triple = (EI.GetDstNId(), EI.GetSrcNId(), label)

            dgraph.append(triple)

        dgraph = sorted(dgraph, key=lambda x: x[0], reverse=False)

        f.write(str(UGraph.GetNodes()))
        f.write(",")
        f.write(str(UGraph.GetEdges()))
        f.write(",")
        f.write(str(L))
        f.write("\n")

        for source, target, label in dgraph:
            f.write(str(source))
            f.write(' ')
            f.write(str(label))
            f.write(' ')
            f.write(str(target))
            f.write(' .\n')
