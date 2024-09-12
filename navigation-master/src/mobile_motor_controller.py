#!/usr/bin/env python  
import math
import rospy
from geometry_msgs.msg import Twist

from ECUTriffidLib.ros2triffidECU import Triffid
from settings import LOG_LEVEL


M_CONST = 60 / 2 / math.pi


class VelocityTransformer:

    def __init__(self, wheal_radius, gear_ratio):
        self.wheel_radius = wheal_radius
        self.gear_ratio = gear_ratio

    def linear_velocity_to_rpm(self, linear_velocity):
        return linear_velocity * M_CONST * self.gear_ratio / self.wheel_radius

    def angular_velocity_to_rpm(self, angular_velocity):
        if angular_velocity > 0:
            return 300
        if angular_velocity < 0:
            return -300
        return 0


# TODO add cmd_vel suscriber, and run TRIFFID on cmd_vel topic using self.move_straight and self.turn functions
class TriffidMotorController:

    def __init__(self):
        self.linear_velocity_x = 0.0
        self.linear_velocity_y = 0.0
        self.angular_velocity_z = 0.0

        self.triffid = Triffid()
        self.triffid.start()
        self.velocity_transformer = VelocityTransformer(0.13, 28.0)
        if not self.triffid:
            rospy.logerr("Can not find Triffid's ECU \nPlease check lan connection between Triffid's mainboard and ECU.")
            rospy.signal_shutdown("Shutdown from triffid_motor_controller...")

        self.sub = rospy.Subscriber("/cmd_vel", Twist, self.callback)

    def callback(self, msg):
        if self.linear_velocity_x != msg.linear.x:
            self.linear_velocity_x = msg.linear.x
            self.triffid.move_straight(int(self.velocity_transformer.linear_velocity_to_rpm(msg.linear.x)))
        if self.linear_velocity_y != msg.linear.y:
            self.linear_velocity_y = msg.linear.y
            self.triffid.move_straight(int(self.velocity_transformer.linear_velocity_to_rpm(msg.linear.y)))
            # print(int(self.velocity_transformer.linear_velocity_to_rpm(msg.linear.x)))
        if self.angular_velocity_z != msg.angular.z:
            self.angular_velocity_z = msg.angular.z
            self.triffid.turn(int(self.velocity_transformer.angular_velocity_to_rpm(msg.angular.z)))
            # print(self.velocity_transformer.angular_velocity_to_rpm(msg.angular.z))


def main():
    rospy.init_node('triffid_motor_controller', log_level=LOG_LEVEL)
    _ = TriffidMotorController()
    try:
        rospy.spin()
    except KeyboardInterrupt:
        rospy.loginfo("Shutting down triffid_motor_controller...")


if __name__ == '__main__':
    main()
