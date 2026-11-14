#include "vision_nodes_pcl/main_visualizer_node.hpp"
#include "vision_nodes_pcl/node_main.hpp"

int main(int argc, char** argv)
{
  return vision_nodes_pcl::runNode<vision_nodes_pcl::MainVisualizerNode>(argc, argv,
                                                                         "main_visualizer");
}
