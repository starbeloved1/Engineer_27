#include "icp_registration/icp_registration_node.hpp"
#include "vision_core/node_main.hpp"

int main(int argc, char** argv)
{
  return vision_core::runNode<icp_registration::IcpRegistrationNode>(argc, argv,
                                                                          "icp_registration");
}
