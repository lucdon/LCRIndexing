#pragma once

#include "graphs/Definitions.hpp"

/**
 * @brief Calculates the sum of n choose i, for 1 &lt;= i &lt;= k
 * Empty set is excluded.
 */
uint64_t calculateCombinationsUpToK(uint32_t k, uint32_t n);

/**
 * @brief Calculates the sum of n choose i, for 1 &lt;= i &lt;= k
 * Empty set is excluded.
 */
uint64_t calculateCombinationsBetween(uint32_t kMin, uint32_t kMax, uint32_t n);

/**
 * @brief Calculates n choose k.
 */
uint64_t calculateCombinationsAtK(uint32_t k, uint32_t n);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Uses BFS
 */
bool reachabilityBFS(const DiGraph &graph, Vertex source, Vertex target);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Uses DFS
 */
bool reachabilityDFS(const DiGraph &graph, Vertex source, Vertex target);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Uses Bi-directional BFS
 */
bool reachabilityBiBFS(const DiGraph &graph, Vertex source, Vertex target);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Also constructs the found path from target to source.
 * Uses BFS
 */
bool BFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Also constructs the found path from target to source.
 * Uses DFS
 */
bool DFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath);

/**
 * @brief returns true if target is reachable from source in the graph.
 * Also constructs the found path from target to source.
 * Uses Bi-directional BFS
 */
bool BiBFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath);

/**
 * @brief Returns all vertices reachable from source.
 * Discovery algorithm: BFS
 */
void singleSourceReachable(const DiGraph &graph, Vertex source, std::vector<Vertex> &outReachableSet);

/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 */
std::unique_ptr<SCCGraph> tarjanSCC(const DiGraph &graph);

/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 * @param includeComponents Set to true if the scc graph should also include the component to vertices mapping
 * @param includeMainGraph Set to true if the scc graph should also include a reference to the original graph.
 */
std::unique_ptr<SCCGraph> tarjanSCC(const DiGraph &graph, bool includeComponents);

/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 */
std::unique_ptr<SCCGraph> tarjanSCC(const LabeledGraph &graph);

/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 * @param includeComponents Set to true if the scc graph should also include the component to vertices mapping
 */
std::unique_ptr<SCCGraph> tarjanSCC(const LabeledGraph &graph, bool includeComponents);


/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 */
std::unique_ptr<SCCGraph> tarjanSCC(const LabeledEdgeGraph &graph);

/**
 * @brief Returns the strongly connected component graph. Created by using tarjan's algorithm.
 * The SCC graph is guaranteed to be topologically sorted.
 * @param includeComponents Set to true if the scc graph should also include the component to vertices mapping
 */
std::unique_ptr<SCCGraph> tarjanSCC(const LabeledEdgeGraph &graph, bool includeComponents);

/**
 * @brief Counts the number of weakly connected components in the graph.
 */
uint32_t tarjanWCC(const DiGraph &graph);

/**
 * @brief Construct a  undirected graph from a directed graph.
 */
std::unique_ptr<Graph> diGraphToGraph(const DiGraph &diGraph);

/**
 * @brief Construct the compressed labeled graph from a per label graph.
 */
std::unique_ptr<LabeledGraph> compressLabeledGraph(const PerLabelGraph &perLabelGraph);

/**
 * Returns the vertices ordered by multiplied degree.
 */
void vertexOrderByDegree(const DiGraph &graph, std::vector<Vertex> &order);

/**
 * Returns the vertices ordered by multiplied degree.
 */
void vertexOrderByDegree(const LabeledGraph &graph, std::vector<Vertex> &order);

/**
 * Returns the vertices ordered by multiplied degree.
 */
void vertexOrderByDegree(const PerLabelGraph &graph, std::vector<Vertex> &order);

/**
 * Returns the vertices ordered by multiplied degree.
 */
void vertexOrderByDegree(const LabeledEdgeGraph &graph, std::vector<Vertex> &order);

/**
 * @brief Given a per label graph and a labelSet create a single graph only containing the unique edges over the labelSet.
 */
std::unique_ptr<DiGraph>
mergeGraphForLabels(const PerLabelGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats);

/**
 * @brief Given a per label graph and a labelSet create a single graph only containing the unique edges over the labelSet.
 */
std::unique_ptr<DiGraph>
mergeGraphForLabels(const LabeledGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats);

/**
 * @brief Given a per label graph and a labelSet create a single graph only containing the unique edges over the labelSet.
 */
std::unique_ptr<DiGraph>
mergeGraphForLabels(const LabeledEdgeGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats);

/**
 * @brief Creates the inner graphs based on the scc Graph.
 * If a scc component represents a single vertex, then the graph will not be created.
 */
void sccLabeledInnerGraphs(const PerLabelGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<PerLabelGraph>> &innerGraphsOut);

/**
 * @brief Creates the inner graphs based on the scc Graph.
 * If a scc component represents a single vertex, then the graph will not be created.
 */
void sccLabeledInnerGraphs(const LabeledGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<LabeledGraph>> &innerGraphsOut);


/**
 * @brief Creates the inner graphs based on the scc Graph.
 * If a scc component represents a single vertex, then the graph will not be created.
 */
void sccLabeledInnerGraphs(const LabeledEdgeGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<LabeledEdgeGraph>> &innerGraphsOut);

/**
 * @brief Create a graph with numMostFrequentLabels and the rest of the labels reduced to numVirtual virtual labels.
 */
std::unique_ptr<PerLabelGraph>
createVirtualLabelGraph(const PerLabelGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping);

/**
 * @brief Create a graph with numMostFrequentLabels and the rest of the labels reduced to numVirtual virtual labels.
 */
std::unique_ptr<LabeledGraph>
createVirtualLabelGraph(const LabeledGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping);

/**
 * @brief Create a graph with numMostFrequentLabels and the rest of the labels reduced to numVirtual virtual labels.
 */
std::unique_ptr<LabeledEdgeGraph>
createVirtualLabelGraph(const LabeledEdgeGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping);


/**
 * @brief Create a graph with only the labels included.
 */
std::unique_ptr<LabeledEdgeGraph>
splitGraph(const LabeledEdgeGraph &labeledGraph, const LabelSet &labels, const std::vector<Label> &labelMapping);

std::unique_ptr<LabeledGraph>
createLabeledGraph(std::vector<std::tuple<Vertex, Vertex, Label>> &edgeList, uint32_t numVertices, uint32_t numLabels);