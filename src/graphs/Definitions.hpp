#pragma once

typedef uint32_t Vertex;

typedef std::vector<Vertex> Path;
typedef std::vector<Vertex> EdgeList;

typedef uint32_t Label;
typedef boost::dynamic_bitset<> LabelSet;
typedef std::pair<Vertex, LabelSet> LabeledEdge;
typedef std::vector<LabeledEdge> LabeledEdgeSet;

class Graph;
class DiGraph;
class PerLabelGraph;
class LabeledGraph;
class LabeledEdgeGraph;
class SCCGraph;

struct Edge {
    Vertex source;
    Vertex target;
    Label label;

    Edge() : source(0), target(0), label(0) { }
    Edge(Vertex source, Vertex target, Label label) : source(source), target(target), label(label) { }
};

struct MergedGraphStats {
    int64_t increase = 0;
    double increasePercentage = 0;
};