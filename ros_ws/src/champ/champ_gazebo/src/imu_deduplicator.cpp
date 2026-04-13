#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

class ImuDeduplicator : public rclcpp::Node {
public:
  ImuDeduplicator() : Node("imu_deduplicator"), has_last_stamp_(false) {
    const auto input_topic = this->declare_parameter<std::string>("input_topic", "/camera/camera/imu_raw");
    const auto output_topic = this->declare_parameter<std::string>("output_topic", "/camera/camera/imu");

    publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(output_topic, rclcpp::QoS(100));
    subscription_ = this->create_subscription<sensor_msgs::msg::Imu>(
      input_topic,
      rclcpp::QoS(100),
      std::bind(&ImuDeduplicator::callback, this, std::placeholders::_1));
  }

private:
  void callback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    const auto & stamp = msg->header.stamp;
    if (has_last_stamp_ && stamp.sec == last_sec_ && stamp.nanosec == last_nsec_) {
      return;
    }

    has_last_stamp_ = true;
    last_sec_ = stamp.sec;
    last_nsec_ = stamp.nanosec;
    publisher_->publish(*msg);
  }

  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr subscription_;
  bool has_last_stamp_;
  int32_t last_sec_;
  uint32_t last_nsec_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImuDeduplicator>());
  rclcpp::shutdown();
  return 0;
}
