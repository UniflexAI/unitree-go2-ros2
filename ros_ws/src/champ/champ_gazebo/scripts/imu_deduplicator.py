#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu


class ImuDeduplicator(Node):
    def __init__(self):
        super().__init__('imu_deduplicator')
        self.declare_parameter('input_topic', '/camera/camera/imu_raw')
        self.declare_parameter('output_topic', '/camera/camera/imu')

        input_topic = self.get_parameter('input_topic').get_parameter_value().string_value
        output_topic = self.get_parameter('output_topic').get_parameter_value().string_value

        self.last_sec = None
        self.last_nsec = None
        self.publisher = self.create_publisher(Imu, output_topic, 10)
        self.subscription = self.create_subscription(Imu, input_topic, self.callback, 100)

    def callback(self, msg: Imu):
        sec = msg.header.stamp.sec
        nsec = msg.header.stamp.nanosec
        if self.last_sec == sec and self.last_nsec == nsec:
            return

        self.last_sec = sec
        self.last_nsec = nsec
        self.publisher.publish(msg)


def main():
    rclpy.init()
    node = ImuDeduplicator()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
