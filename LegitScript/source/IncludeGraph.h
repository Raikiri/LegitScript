#pragma once
#include <vector>
#include <stdexcept>
namespace ls
{
  using NodeIdx = size_t;

  struct GraphNode
  {
    std::vector<NodeIdx> adjacent_nodes;
  };
  using Graph = std::vector<GraphNode>;
  
  void FlattenNode(const Graph& graph, GraphNode &dst_node, NodeIdx curr_idx, size_t depth)
  {
    if(depth > 1024)
      throw std::runtime_error("Can't flatten graph");
      
    for(auto adjacent_idx : graph[curr_idx].adjacent_nodes)
    {
      FlattenNode(graph, dst_node, adjacent_idx, depth + 1);
    }
    if(depth != 0)
      dst_node.adjacent_nodes.push_back(curr_idx);
  }
  
  Graph FlattenGraph(Graph graph)
  {
    Graph res_graph;
    for(NodeIdx node_idx = 0; node_idx < graph.size(); node_idx++)
    {
      GraphNode flattened_node;
      /*for(auto adjacent_idx : graph[node_idx].adjacent_nodes)
      {
        FlattenNode(graph, flattened_node, adjacent_idx, 0);
      }*/
      FlattenNode(graph, flattened_node, node_idx, 0);
      res_graph.push_back(flattened_node);
    }
    return res_graph;
  }
}