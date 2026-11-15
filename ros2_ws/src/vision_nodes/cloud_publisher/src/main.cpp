#include "model_cloud_publisher/model_cloud_publisher_node.hpp"
#include "vision_core/node_main.hpp"

int main(int argc, char** argv)
{
  return vision_core::runNode<model_cloud_publisher::ModelCloudPublisherNode>(
      argc, argv, "model_cloud_publisher");
}
