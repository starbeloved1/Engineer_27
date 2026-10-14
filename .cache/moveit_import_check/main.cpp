#include <moveit_setup_framework/data_warehouse.hpp>
#include <moveit_setup_framework/data/urdf_config.hpp>
#include <moveit_setup_framework/data/srdf_config.hpp>
#include <rclcpp/rclcpp.hpp>
#include <iostream>

int main(int argc, char **argv)
{
  if (argc != 2) return 2;
  rclcpp::init(argc, argv);
  int result = 0;
  {
    auto node = rclcpp::Node::make_shared("engineer27_import_check");
    auto warehouse = std::make_shared<moveit_setup::DataWarehouse>(node);
    warehouse->preloadWithURDFPath(argv[1]);
    auto description = warehouse->get<moveit_setup::URDFConfig>("urdf");
    auto srdf = warehouse->get<moveit_setup::SRDFConfig>("srdf");
    srdf->updateRobotModel();
    auto model = srdf->getRobotModel();
    if (!model) return 3;
    std::size_t shapes = 0;
    for (auto link : model->getLinkModels()) {
      shapes += link->getShapes().size();
    }
    std::cout << "description package: " << description->getURDFPackageName() << '\n'
              << "links: " << model->getLinkModels().size() << '\n'
              << "variables: " << model->getVariableCount() << '\n'
              << "collision shapes: " << shapes << std::endl;
    if (model->getLinkModels().size() != 7 || model->getVariableCount() != 6 || shapes != 7) result = 4;
  }
  rclcpp::shutdown();
  return result;
}
