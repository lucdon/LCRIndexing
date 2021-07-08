#include "gtest/gtest.h"
#include "graphs/SCCGraph.hpp"

TEST(tarjanSCC, simpleGraph) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(1, 0);
    graph->addEdge(0, 2);
    graph->addEdge(2, 1);
    graph->addEdge(0, 3);
    graph->addEdge(3, 4);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 3) << "Should have 3 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2));
    EXPECT_TRUE(sccGraph->isSingleComponent(3));
    EXPECT_TRUE(sccGraph->isSingleComponent(4));
}

TEST(tarjanSCC, simpleGraph2) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(4);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 4) << "Should have 4 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0));
    EXPECT_TRUE(sccGraph->isSingleComponent(1));
    EXPECT_TRUE(sccGraph->isSingleComponent(2));
    EXPECT_TRUE(sccGraph->isSingleComponent(3));
}

TEST(tarjanSCC, simpleGraph3) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(7);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 0);
    graph->addEdge(1, 3);
    graph->addEdge(1, 4);
    graph->addEdge(1, 6);
    graph->addEdge(3, 5);
    graph->addEdge(4, 5);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 5) << "Should have 5 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2));
    EXPECT_TRUE(sccGraph->isSingleComponent(3));
    EXPECT_TRUE(sccGraph->isSingleComponent(4));
    EXPECT_TRUE(sccGraph->isSingleComponent(5));
    EXPECT_TRUE(sccGraph->isSingleComponent(6));
}

TEST(tarjanSCC, simpleGraph4) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(11);

    graph->addEdge(0, 1);
    graph->addEdge(0, 3);
    graph->addEdge(1, 2);
    graph->addEdge(1, 4);
    graph->addEdge(2, 0);
    graph->addEdge(2, 6);
    graph->addEdge(3, 2);
    graph->addEdge(4, 5);
    graph->addEdge(4, 6);
    graph->addEdge(5, 6);
    graph->addEdge(5, 7);
    graph->addEdge(5, 8);
    graph->addEdge(5, 9);
    graph->addEdge(6, 4);
    graph->addEdge(7, 9);
    graph->addEdge(8, 9);
    graph->addEdge(9, 8);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 5) << "Should have 5 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3));
    EXPECT_TRUE(sccGraph->isSingleComponent(4, 5, 6));
    EXPECT_TRUE(sccGraph->isSingleComponent(7));
    EXPECT_TRUE(sccGraph->isSingleComponent(8, 9));
    EXPECT_TRUE(sccGraph->isSingleComponent(10));
}

TEST(tarjanSCC, simpleGraph5) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(2, 4);
    graph->addEdge(3, 0);
    graph->addEdge(4, 2);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph6) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(8);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 0);
    graph->addEdge(3, 1);
    graph->addEdge(3, 2);
    graph->addEdge(3, 4);
    graph->addEdge(4, 3);
    graph->addEdge(4, 5);
    graph->addEdge(5, 2);
    graph->addEdge(5, 6);
    graph->addEdge(6, 5);
    graph->addEdge(7, 4);
    graph->addEdge(7, 6);
    graph->addEdge(7, 7);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 4) << "Should have 4 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2));
    EXPECT_TRUE(sccGraph->isSingleComponent(3, 4));
    EXPECT_TRUE(sccGraph->isSingleComponent(5, 6));
    EXPECT_TRUE(sccGraph->isSingleComponent(7));
}

TEST(tarjanSCC, simpleGraph7) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 1);
    graph->addEdge(3, 0);
    graph->addEdge(0, 4);
    graph->addEdge(4, 3);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph8) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(0, 1);
    graph->addEdge(4, 3);
    graph->addEdge(0, 4);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 1);
    graph->addEdge(3, 0);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph9) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(0, 4);
    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 1);
    graph->addEdge(3, 0);
    graph->addEdge(4, 3);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph10) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    graph->addEdge(0, 1);
    graph->addEdge(0, 4);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 1);
    graph->addEdge(3, 0);
    graph->addEdge(4, 3);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph11) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(13);

    graph->addEdge(0, 1);
    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 4);

    graph->addEdge(0, 5);
    graph->addEdge(5, 6);
    graph->addEdge(6, 7);
    graph->addEdge(7, 8);

    graph->addEdge(0, 9);
    graph->addEdge(9, 10);
    graph->addEdge(10,  11);
    graph->addEdge(11, 12);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 13) << "Should have 13 components";
    // EXPECT_TRUE(sccGraph->isSingleComponent(0, 1, 2, 3, 4));
}

TEST(tarjanSCC, simpleGraph12) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(13);

    graph->addEdge(1, 2);
    graph->addEdge(2, 3);
    graph->addEdge(3, 4);
    graph->addEdge(4, 1);

    graph->addEdge(5, 6);
    graph->addEdge(6, 7);
    graph->addEdge(7, 8);
    graph->addEdge(8, 5);

    graph->addEdge(9, 10);
    graph->addEdge(10,  11);
    graph->addEdge(11, 12);
    graph->addEdge(12, 9);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 4) << "Should have 4 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0));
    EXPECT_TRUE(sccGraph->isSingleComponent(1, 2, 3, 4));
    EXPECT_TRUE(sccGraph->isSingleComponent(5, 6, 7, 8));
    EXPECT_TRUE(sccGraph->isSingleComponent(9, 10, 11, 12));
}

TEST(tarjanSCC, GraphWithNoEdges) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(5);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 5) << "Should have 5 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0));
    EXPECT_TRUE(sccGraph->isSingleComponent(1));
    EXPECT_TRUE(sccGraph->isSingleComponent(2));
    EXPECT_TRUE(sccGraph->isSingleComponent(3));
    EXPECT_TRUE(sccGraph->isSingleComponent(4));
}

TEST(tarjanSCC, EmptyGraph) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 0) << "Should have 0 components";
}

TEST(tarjanSCC, GraphWithSingleVertex) {
    // Arrange
    auto graph = std::make_unique<DiGraph>();

    graph->setVertices(1);

    // Act
    auto sccGraph = tarjanSCC(*graph, true);

    // Assert
    ASSERT_EQ(sccGraph->getComponentGraph().getVertexCount(), 1) << "Should have 1 components";
    EXPECT_TRUE(sccGraph->isSingleComponent(0));
}