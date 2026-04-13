#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/imu.hpp"

class RealsensePatchNode : public rclcpp::Node {
public:
  RealsensePatchNode() : Node("realsense_patch_node"), has_last_imu_stamp_(false) {
    imu_input_topic_ = this->declare_parameter<std::string>("imu_input_topic", "/camera/camera/imu_raw");
    imu_output_topic_ = this->declare_parameter<std::string>("imu_output_topic", "/camera/camera/imu");
    infra1_info_input_topic_ = this->declare_parameter<std::string>("infra1_info_input_topic", "/camera/camera/infra1/camera_info_raw");
    infra1_info_output_topic_ = this->declare_parameter<std::string>("infra1_info_output_topic", "/camera/camera/infra1/camera_info");
    infra2_info_input_topic_ = this->declare_parameter<std::string>("infra2_info_input_topic", "/camera/camera/infra2/camera_info_raw");
    infra2_info_output_topic_ = this->declare_parameter<std::string>("infra2_info_output_topic", "/camera/camera/infra2/camera_info");
    stereo_baseline_ = this->declare_parameter<double>("stereo_baseline", 0.05);

    imu_publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(imu_output_topic_, rclcpp::QoS(100));
    infra1_info_publisher_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(infra1_info_output_topic_, rclcpp::QoS(10));
    infra2_info_publisher_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(infra2_info_output_topic_, rclcpp::QoS(10));

    imu_subscription_ = this->create_subscription<sensor_msgs::msg::Imu>(
      imu_input_topic_, rclcpp::QoS(100), std::bind(&RealsensePatchNode::imuCallback, this, std::placeholders::_1));
    infra1_info_subscription_ = this->create_subscription<sensor_msgs::msg::CameraInfo>(
      infra1_info_input_topic_, rclcpp::QoS(10), std::bind(&RealsensePatchNode::infra1InfoCallback, this, std::placeholders::_1));
    infra2_info_subscription_ = this->create_subscription<sensor_msgs::msg::CameraInfo>(
      infra2_info_input_topic_, rclcpp::QoS(10), std::bind(&RealsensePatchNode::infra2InfoCallback, this, std::placeholders::_1));
  }

private:
  void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    const auto & stamp = msg->header.stamp;
    if (has_last_imu_stamp_ && stamp.sec == last_imu_sec_ && stamp.nanosec == last_imu_nsec_) {
      return;
    }

    has_last_imu_stamp_ = true;
    last_imu_sec_ = stamp.sec;
    last_imu_nsec_ = stamp.nanosec;
    imu_publisher_->publish(*msg);
  }

  void infra1InfoCallback(const sensor_msgs::msg::CameraInfo::SharedPtr msg) {
    auto patched = *msg;
    patched.p[3] = 0.0;
    infra1_info_publisher_->publish(patched);
  }

  void infra2InfoCallback(const sensor_msgs::msg::CameraInfo::SharedPtr msg) {
    auto patched = *msg;
    const double fx = patched.k[0];
    patched.p[3] = -fx * stereo_baseline_;
    infra2_info_publisher_->publish(patched);
  }

  std::string imu_input_topic_;
  std::string imu_output_topic_;
  std::string infra1_info_input_topic_;
  std::string infra1_info_output_topic_;
  std::string infra2_info_input_topic_;
  std::string infra2_info_output_topic_;
  double stereo_baseline_;

  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_publisher_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr infra1_info_publisher_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr infra2_info_publisher_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscription_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr infra1_info_subscription_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr infra2_info_subscription_;

  bool has_last_imu_stamp_;
  int32_t last_imu_sec_;
  uint32_t last_imu_nsec_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RealsensePatchNode>());
  rclcpp::shutdown();
  return 0;
}
