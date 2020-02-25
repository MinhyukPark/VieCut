/******************************************************************************
 * configuration.h
 *
 * Source of VieCut.
 *
 ******************************************************************************
 * Copyright (C) 2019 Alexander Noe <alexander.noe@univie.ac.at>
 *
 *****************************************************************************/
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/definitions.h"

class configuration {
 public:
    configuration(configuration const&) = delete;
    void operator = (configuration const&) = delete;

    static std::shared_ptr<configuration> getConfig() {
        static std::shared_ptr<configuration> instance{ new configuration };
        return instance;
    }

    ~configuration() { }

    // Settings - these are public for ease of use
    // don't change in program when not necessary
    std::string graph_filename;
    std::string partition_file = "";
    std::string output_path = "";
    size_t seed = 0;
    bool verbose = false;

    // multiterminal cut parameters
    std::string edge_selection = "heavy_vertex";
    std::string queue_type = "bound_sum";
    std::vector<std::string> term_strings;
    int top_k = 0;
    int random_k = 0;
    size_t bfs_size = 0;
    size_t threads = 1;
    double preset_percentage = 0;
    size_t total_terminals = 0;
    size_t print_cc = 0;
    bool disable_cpu_affinity = false;

    // minimum cut parameters
    bool save_cut = false;
    std::string algorithm;
    std::string sampling_type = "geometric";
    std::string pq = "default";
    size_t num_iterations = 1;
    bool disable_limiting = false;
    double contraction_factor = 0.0;
    bool find_most_balanced_cut = false;
    bool find_lowest_conductance = false;
    bool blacklist = true;
    bool set_node_in_cut = false;
    bool multibranch = true;
    std::string first_branch_path = "";
    bool write_solution = false;
    size_t neighborhood_degrees = 50;
    size_t random_flows = 5;
    double high_distance_factor = 0.9;
    size_t high_distance_flows = 5;

    // karger-stein:
    size_t optimal = 0;

    bool use_ilp = false;
    // this is not what the configuration should be used for - but it's quick
    bool differences_set = false;
    EdgeWeight bound_difference = 0;
    NodeID n = 0;
    NodeID m = 0;

 private:
    configuration() { }
};
