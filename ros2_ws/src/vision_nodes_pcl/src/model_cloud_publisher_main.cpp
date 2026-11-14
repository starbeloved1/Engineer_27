#include "vision_nodes_pcl/model_cloud_publisher_node.hpp"
#include "vision_nodes_pcl/node_main.hpp"

int main(int argc, char** argv)
{
  return vision_nodes_pcl::runNode<vision_nodes_pcl::ModelCloudPublisherNode>(
      argc, argv, "model_cloud_publisher");
}
