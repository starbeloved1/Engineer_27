#include "engineer_mtc/vision_target_buffer.hpp"
#include <iostream>
#include <functional>
#include <limits>
using namespace engineer_mtc;
void require(bool condition,const char *message){if(!condition)throw std::runtime_error(message);}
void fails(const std::function<void()> &function,const char *message){try{function();}catch(const std::exception&){return;}throw std::runtime_error(message);}
geometry_msgs::msg::Pose pose(double x,double angle=0){Eigen::Isometry3d T=Eigen::Isometry3d::Identity();T.translation().x()=x;T.linear()=Eigen::AngleAxisd(angle,Eigen::Vector3d::UnitZ()).toRotationMatrix();return transformPose(T);}
geometry_msgs::msg::PoseArray sample(double stamp,double x=0,double angle=0,const std::string &frame="camera"){
 geometry_msgs::msg::PoseArray m;m.header.frame_id=frame;m.header.stamp.sec=static_cast<int32_t>(stamp);m.header.stamp.nanosec=static_cast<uint32_t>(std::llround((stamp-m.header.stamp.sec)*1e9));m.poses.push_back(pose(x,angle));return m;
}
int main(){
 auto mean=averagePoses({pose(.01),pose(.02),pose(.03)});require(std::abs(mean.position.x-.02)<1e-12,"position mean");
 auto a=pose(0,.3),b=a;b.orientation.x*=-1;b.orientation.y*=-1;b.orientation.z*=-1;b.orientation.w*=-1;
 mean=averagePoses({a,b});require(poseTransform(mean).linear().isApprox(poseTransform(a).linear(),1e-12),"quaternion sign invariance");
 mean=averagePoses({pose(0,.2),pose(0,.4)});require(poseTransform(mean).linear().isApprox(poseTransform(pose(0,.3)).linear(),1e-10),"orientation mean");
 fails([]{averagePoses({});},"empty mean rejected");
 fails([]{averagePoses({pose(0,0),pose(0,3.141592653589793)});},"ambiguous orientation rejected");
 auto invalid=pose(0);invalid.position.x=std::numeric_limits<double>::quiet_NaN();fails([&]{averagePoses({invalid});},"invalid pose rejected");
 VisionTargetBuffer buffer;buffer.push(sample(10,.01),0,3,1.5);buffer.push(sample(10.2,.02),0,3,1.5);
 fails([&]{buffer.recent(10.3,1.5,3);},"insufficient samples rejected");
 buffer.push(sample(10.4,.03),0,3,1.5);auto history=buffer.recent(10.5,1.5,3);require(history.size()==3&&history.front().header.stamp.sec==10,"full history retained");
 buffer.push(sample(10.4,.03),0,3,1.5);require(buffer.recent(10.5,1.5,3).size()==3,"duplicate timestamp ignored");
 buffer.push(sample(10.6,.04),0,3,1.5);history=buffer.recent(10.7,1.5,3);require(std::abs(history.front().pose.position.x-.02)<1e-12,"oldest sample dropped");
 fails([&]{buffer.recent(12.5,1.5,3);},"stale history rejected");
 auto missing=sample(10.8);missing.poses.clear();buffer.push(missing,0,3,1.5);fails([&]{buffer.recent(10.9,1.5,1);},"target loss clears history");
 buffer.push(sample(11),0,3,1.5);buffer.push(sample(11.2,0,0,"other_frame"),0,3,1.5);fails([&]{buffer.recent(11.3,1.5,2);},"frame change clears history");
 buffer.push(sample(11.4,0,3.141592653589793,"other_frame"),0,3,1.5);fails([&]{buffer.recent(11.5,1.5,2);},"end reversal clears history");
 buffer.push(sample(9,0,0,"other_frame"),0,3,1.5);fails([&]{buffer.recent(9.1,1.5,2);},"backward timestamp clears history");
 auto multi=sample(9.2,0,0,"other_frame");multi.poses.push_back(pose(1));buffer.push(multi,0,3,1.5);fails([&]{buffer.recent(9.3,1.5,2);},"target count change clears history");
 VisionTargetBuffer single;single.push(sample(20,.1),0,1,1.5);single.push(sample(20.1,.2),0,1,1.5);require(single.recent(20.2,1.5,1)[0].pose.position.x==.2,"single-frame mode");
 VisionTargetBuffer slow;
 for(int i=0;i<5;++i)slow.push(sample(30+.8*i,.001*i),0,5,1.5);
 require(slow.recent(34,5,5,1.5).size()==5,"slow stream uses bounded longer history");
 fails([&]{slow.recent(34,1.5,5,1.5);},"strict historical window still rejects old samples");
 fails([&]{slow.recent(35,5,5,1.5);},"long history never permits stale latest observation");
 std::cout<<"PASS pose averaging and history freshness/reset tests\n";
}
