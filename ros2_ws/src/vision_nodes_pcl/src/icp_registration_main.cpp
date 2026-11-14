#include "vision_nodes_pcl/icp_registration_node.hpp"
#include "vision_nodes_pcl/node_main.hpp"

int main(int argc, char** argv)
{
  return vision_nodes_pcl::runNode<vision_nodes_pcl::IcpRegistrationNode>(argc, argv,
                                                                          "icp_registration");
}
