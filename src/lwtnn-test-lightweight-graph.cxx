// Use example for LightweightGraph, the high level wrapper for Graph

#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"

#include "test_utilities.hh"

#include <iostream>
#include <fstream>
#include <cassert>

namespace {
  lwt::LightweightGraph::SeqNodeMap get_sequences(
    const std::vector<lwt::InputNodeConfig>& config);

  int run_on_generated(const lwt::GraphConfig& config);
}


void usage(const std::string& name) {
  std::cout << "usage: " << name << " <nn config>\n"
    "\n"
    "The <nn config> file should be generated by one of the scripts in\n"
    "`converters/`.\n"
    "\n";
}

int main(int argc, char* argv[]) {
  if (argc > 2 || argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  // Read in the configuration.
  std::string in_file_name(argv[1]);
  std::ifstream in_file(in_file_name);
  auto config = lwt::parse_json_graph(in_file);

  run_on_generated(config);
  return 0;
}
namespace {
  int run_on_generated(const lwt::GraphConfig& config) {
    using namespace lwt;
    assert(config.outputs.size() > 0);

    // First build the tagger object. For graphs with multiple outputs
    // you must specify the name of the default output by name (thus
    // the second argument here).
    lwt::LightweightGraph tagger(config, config.outputs.begin()->first);

    // Build dummy input patterns from the configuration object. For
    // inputs that take vectors the inner map corresponds to the named
    // input variables for a node. The outer map indexes the nodes by
    // name. The `ramp` function is defined in the `test_utilities`
    // header.
    std::map<std::string, std::map<std::string, double> > in_nodes;
    for (const auto& input: config.inputs) {
      const size_t total_inputs = input.variables.size();
      std::map<std::string, double> in_vals;
      for (size_t nnn = 0; nnn < total_inputs; nnn++) {
        const auto& var = input.variables.at(nnn);
        double ramp_val = ramp(var, nnn, total_inputs);
        in_vals[var.name] = ramp_val;
      }
      in_nodes[input.name] = in_vals;
    }

    // The inputs for sequences are defined like those for vectors,
    // but rather than a `double` each variable in the inner map is
    // represented by a vector.
    LightweightGraph::SeqNodeMap seq = get_sequences(config.input_sequences);

    // Loop over the output names and compute the output for each
    for (const auto& output: config.outputs) {
      auto out_vals = tagger.compute(in_nodes, seq, output.first);
      std::cout << output.first << ":" << std::endl;
      for (const auto& out: out_vals) {
        std::cout << out.first << " " << out.second << std::endl;
      }
    }
    return 0;
  }

  lwt::LightweightGraph::SeqNodeMap get_sequences(
    const std::vector<lwt::InputNodeConfig>& config) {
    lwt::LightweightGraph::SeqNodeMap nodes;
    for (const auto& input: config) {
      // see the `test_utilities` header for this function.
      nodes[input.name] = get_values_vec(input.variables, 20);
    }
    return nodes;
  }
}
