#include "io/graph_io.h"
#include <gtest/gtest.h>
#include <random>
#include <type_traits>

#include "algorithms/flow/push_relabel.h"
#include "data_structure/graph_access.h"
#include "tools/vector.h"

TEST(PushRelabelTest, EmptyGraph) {

    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();
    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;
    std::vector<NodeID> src;
    src.push_back(0);
    src.push_back(1);
    std::vector<NodeID> src_block;
    FlowType f = pr.solve_max_flow_min_cut(fG, src, 0, false, src_block);
    ASSERT_EQ(f, -1);
}

TEST(PushRelabelTest, TooLargeSrc) {

    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

    G->start_construction(10, 0);
    for (NodeID i = 0; i < 10; ++i)
        G->new_node();
    G->finish_construction();

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;
    std::vector<NodeID> src;
    src.push_back(0);
    src.push_back(10);
    std::vector<NodeID> src_block;
    FlowType f = pr.solve_max_flow_min_cut(fG, src, 0, false, src_block);
    ASSERT_EQ(f, -1);
}

TEST(PushRelabelTest, NoEdges) {
    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

    G->start_construction(10, 0);
    for (NodeID i = 0; i < 10; ++i)
        G->new_node();
    G->finish_construction();

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;
    std::vector<NodeID> src;
    src.push_back(0);
    src.push_back(9);
    std::vector<NodeID> src_block;
    FlowType f = pr.solve_max_flow_min_cut(fG, src, 0, true, src_block);
    ASSERT_EQ(f, 0);
    ASSERT_EQ(src_block.size(), 1);
    ASSERT_EQ(src_block[0], 0);
}

TEST(PushRelabelTest, DisconnectedCliques) {
    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

    G->start_construction(20, 0);
    for (NodeID clique = 0; clique < 2; ++clique) {
        for (NodeID i = 0; i < 10; ++i) {
            G->new_node();
            for (NodeID j = 0; j < 10; ++j) {
                if (i == j)
                    continue;
                G->new_edge((clique * 10) + i, (clique * 10) + j);
            }
        }
    }
    G->finish_construction();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(0, 9);

    for (size_t test = 0; test < 5; ++test) {

        std::vector<NodeID> src;
        src.push_back(distribution(eng));
        src.push_back(10 + distribution(eng));

        for (size_t src_v = 0; src_v < 2; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
            ASSERT_EQ(f, 0);
            ASSERT_EQ(src_block.size(), 10);
        }
    }
}

TEST(PushRelabelTest, CliqueSingleSink) {
    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

    G->start_construction(10, 0);
    for (NodeID i = 0; i < 10; ++i) {
        G->new_node();
        for (NodeID j = 0; j < 10; ++j) {
            if (i == j)
                continue;
            G->new_edge(i, j);
        }
    }
    G->finish_construction();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(0, 9);

    for (size_t test = 0; test < 5; ++test) {

        std::vector<NodeID> src;
        src.push_back(distribution(eng));
        NodeID tgt;
        do {
            tgt = distribution(eng);
        }
        while (tgt == src[0]);

        src.push_back(tgt);

        for (size_t src_v = 0; src_v < 2; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
            ASSERT_EQ(f, 9);
            ASSERT_EQ(src_block.size(), 9);
        }
    }
}

TEST(PushRelabelTest, CliqueMultipleSinks) {
    std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

    G->start_construction(10, 0);
    for (NodeID i = 0; i < 10; ++i) {
        G->new_node();
        for (NodeID j = 0; j < 10; ++j) {
            if (i == j)
                continue;
            G->new_edge(i, j);
        }
    }
    G->finish_construction();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(0, 9);

    for (size_t test = 0; test < 5; ++test) {

        std::vector<NodeID> src;
        src.push_back(distribution(eng));
        NodeID tgt;
        for (size_t sinks = 0; sinks < 3; ++sinks) {
            do {
                tgt = distribution(eng);
            } while (vector::contains(src, tgt));
            src.push_back(tgt);
        }

        for (size_t src_v = 0; src_v < 4; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
            ASSERT_EQ(f, 9);
            ASSERT_EQ(src_block.size(), 1);
            ASSERT_EQ(src_block[0], src[src_v]);
        }
    }
}

TEST(PushRelabelTest, UnweightedGraphOneSink) {

    std::shared_ptr<graph_access> G =
        graph_io::readGraphWeighted(std::string(VIECUT_PATH) + "/graphs/small.metis");

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(0, 3);

    for (size_t test = 0; test < 5; ++test) {

        std::vector<NodeID> src;
        src.push_back(distribution(eng));
        src.push_back(distribution(eng) + 4);

        for (size_t src_v = 0; src_v < 2; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
            ASSERT_EQ(f, 2);
            ASSERT_EQ(src_block.size(), 4);

            for (unsigned int v = 0; v < 8; ++v) {
                if ((v / 4) == (src[src_v] / 4)) {
                    ASSERT_TRUE(vector::contains(src_block, v));
                }
                else {
                    ASSERT_FALSE(vector::contains(src_block, v));
                }
            }
        }
    }
}

TEST(PushRelabelTest, WeightedGraphOneSink) {

    std::shared_ptr<graph_access> G =
        graph_io::readGraphWeighted(std::string(VIECUT_PATH) + "/graphs/small-wgt.metis");

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(0, 3);

    for (size_t test = 0; test < 5; ++test) {

        std::vector<NodeID> src;
        src.push_back(distribution(eng));
        src.push_back(distribution(eng) + 4);

        for (size_t src_v = 0; src_v < 2; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
            ASSERT_EQ(f, 3);
            ASSERT_EQ(src_block.size(), 4);

            for (unsigned int v = 0; v < 8; ++v) {
                if ((v / 4) == (src[src_v] / 4)) {
                    ASSERT_TRUE(vector::contains(src_block, v));
                }
                else {
                    ASSERT_FALSE(vector::contains(src_block, v));
                }
            }
        }
    }
}

TEST(PushRelabelTest, UnweightedGraphMultipleSinks) {

    std::shared_ptr<graph_access> G =
        graph_io::readGraphWeighted(std::string(VIECUT_PATH) + "/graphs/small.metis");

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;
    std::vector<NodeID> src;
    src.push_back(0);
    src.push_back(3);
    src.push_back(4);
    src.push_back(7);

    for (size_t src_v = 0; src_v < 4; ++src_v) {
        std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
        push_relabel pr;
        std::vector<NodeID> src_block;
        FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
        ASSERT_EQ(f, 4);
        ASSERT_EQ(src_block.size(), 3);
    }
}

TEST(PushRelabelTest, WeightedGraphMultipleSinks) {

    std::shared_ptr<graph_access> G =
        graph_io::readGraphWeighted(std::string(VIECUT_PATH) + "/graphs/small-wgt.metis");

    std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);

    push_relabel pr;
    std::vector<NodeID> src;
    src.push_back(0);
    src.push_back(3);
    src.push_back(4);
    src.push_back(7);

    for (size_t src_v = 0; src_v < 4; ++src_v) {
        std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
        push_relabel pr;
        std::vector<NodeID> src_block;
        FlowType f = pr.solve_max_flow_min_cut(fG, src, src_v, true, src_block);
        if (src_v == 0) {
            ASSERT_EQ(f, 10);
            ASSERT_EQ(src_block.size(), 1);
        }
        if (src_v == 1) {
            ASSERT_EQ(f, 11);
            ASSERT_EQ(src_block.size(), 3);
        }
        if (src_v == 2) {
            ASSERT_EQ(f, 11);
            ASSERT_EQ(src_block.size(), 2);
        }
        if (src_v == 3) {
            ASSERT_EQ(f, 12);
            ASSERT_EQ(src_block.size(), 2);
        }
    }
}

TEST(PushRelabelTest, BlocksOnMulticutUnequalGraph) {

    std::vector<size_t> sizes = { 1, 10, 50, 100 };
    for (size_t cluster_size : sizes) {
        std::shared_ptr<graph_access> G = std::make_shared<graph_access>();

        G->start_construction(8 * cluster_size, 2 * cluster_size * (cluster_size - 1) * 8 + 40);

        for (size_t i = 0; i < 8; ++i) {
            for (size_t j = 0; j < 8; ++j) {
                if (i / 2 == j / 2 && i != j && i / 2) {
                    EdgeID e = G->new_edge(i * cluster_size, j * cluster_size);
                    G->setEdgeWeight(e, 3);
                }

                if (i != j && i % 2 == 1 && j % 2 == 1 && i + j != 8) {
                    EdgeID e = G->new_edge(i * cluster_size, j * cluster_size);
                    G->setEdgeWeight(e, 2);
                }
            }

            for (size_t j = 0; j < cluster_size; ++j) {
                for (size_t k = 0; k < cluster_size; ++k) {
                    if (j != k) {
                        NodeID base = cluster_size * i;
                        G->new_edge(base + j, base + k);
                    }
                }
            }
        }

        G->finish_construction();

        std::vector<NodeID> terminals;

        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distribution(0, cluster_size - 1);
        for (size_t i = 1; i < 4; ++i) {
            terminals.emplace_back((2 * i) * cluster_size + distribution(eng));
        }

        for (size_t src_v = 0; src_v < 3; ++src_v) {
            std::shared_ptr<flow_graph> fG = graph_io::createFlowGraph(G);
            push_relabel pr;
            std::vector<NodeID> src_block;
            FlowType f = pr.solve_max_flow_min_cut(fG, terminals, src_v, true, src_block);

            ASSERT_EQ(f, (FlowType)3);
            ASSERT_EQ(src_block.size(), cluster_size);
        }
    }
}