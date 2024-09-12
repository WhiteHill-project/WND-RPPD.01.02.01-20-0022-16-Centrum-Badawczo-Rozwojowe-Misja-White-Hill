#!/usr/bin/env python  
import math
import time
import random
import sys

import rospy
import tf

from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Pose, Quaternion, Twist, Vector3, TransformStamped

from triffidMouseROSLib.flw100ROSLib import Flw100
from settings import LOG_LEVEL


class TriffidOdomBroadcaster:

    def __init__(self, rate):
        rospy.loginfo("Creating odom_mouse_footprint publisher...")
        self.rate = rate
        self.odom_pub = rospy.Publisher('odom', Odometry, queue_size=self.rate)
        
        rospy.loginfo("Creating odom <--> mouse_footprint broadcaster...")
        self.odom_broadcaster = tf.TransformBroadcaster()


        self.odom_x = 0
        self.odom_y = 0
        self.odom_yaw = 0
        self.last_x = 0
        self.last_y = 0
        self.last_yaw = 0
        # Getting FLW100 sensor's handler 
        try:
            self.sensor_flw100 = Flw100()
            self.sensor_flw100.set_zero()
        except :
            rospy.logerr("Can not find FLW100 sensor connected to ROS!!! \nPlease check usb connection and usb port privileges (sudo chmod 666 <usbport address>")
            print("Sysytem has stoped process!!!")
            self.sensor_flw100 = None
            sys.exit(1)

    def get_p_r_y(self):
        p_r_y = self.sensor_flw100.get_p_r_y()
        pitch = float(p_r_y[0]) / 1000
        roll = float(p_r_y[1]) / 1000
        yaw = float(p_r_y[2]) / 1000
        
        return pitch, roll, yaw

    def get_x_y(self):
        x_y = self.sensor_flw100.get_x_y()
        x = float(x_y[1]) / 10000
        y = float(x_y[0]) / 10000
        
        return x, y
    
    def get_flw100_offset_per_sec(self, calibration_time):
        rospy.loginfo("Calibraiting FLW100 sensor... It takes at least {} seconds.".format(str(calibration_time)))
        _, _, yaw_offset1 = self.get_p_r_y()
        time.sleep(calibration_time)
        _, _, yaw_offset2 = self.get_p_r_y()
        
        return float(yaw_offset1 - yaw_offset2) / calibration_time

    def broadcast_odom(self):
        # TODO add getting location and velocity from FLW100 sensor using triffidMouseROSLib
        
        rate = rospy.Rate(self.rate)
        _, _, yaw_offset = self.get_p_r_y()
        yaw_offset_rate = self.get_flw100_offset_per_sec(5)
        offset_counter = 0

        last_time = rospy.Time.now()

        while not rospy.is_shutdown():
            current_time = rospy.Time.now()
            dt = (current_time - last_time).to_sec()
            last_time = current_time
            # Get location, and pose from mouse
            x, y = self.get_x_y()
            x *= (-1) 
            y *= (-1) 
            _, _, yaw = self.get_p_r_y()
            # print("Offset rate: {}".format(str(yaw_offset_rate)))
            # print("Cumulated offset: {}".format(str(yaw_offset_rate / self.rate * offset_counter)))
            # print("Yaw with offset reducton: {}".
            #       format(str(yaw - yaw_offset + yaw_offset_rate / self.rate * offset_counter)))
            # print("Yaw without offset reduction: {}".format(str(yaw - yaw_offset)))
            offset_counter += 1
        
            yaw = yaw - yaw_offset + yaw_offset_rate / self.rate * offset_counter

            dx = x - self.last_x
            dy = y - self.last_y
            dyaw = yaw - self.last_yaw

            # if abs(dyaw) < 0.1 and dy > 0.0:
            #     dyaw = 0

            self.last_x = x
            self.last_y = y
            self.last_yaw = yaw

            delta_x = dx * math.cos(math.radians(self.odom_yaw)) - dy * math.sin(math.radians(self.odom_yaw))
            delta_y = dx * math.sin(math.radians(self.odom_yaw)) + dy * math.cos(math.radians(self.odom_yaw))

            self.odom_x += delta_x
            self.odom_y += delta_y
            self.odom_yaw += dyaw

            # self.odom_x += -0.62*(math.cos(math.radians(self.odom_yaw)) - math.cos(math.radians(self.odom_yaw+dyaw)))
            # self.odom_y += -0.62*(math.sin(math.radians(self.odom_yaw)) - math.sin(math.radians(self.odom_yaw+dyaw)))

            odom_quat = tf.transformations.quaternion_from_euler(0, 0, math.radians(self.odom_yaw))
    
            # send the transform
            # rospy.loginfo("Broadcasting transformation odom and mouse_footprint frames...")
            self.odom_broadcaster.sendTransform((self.odom_x, self.odom_y, 0.0),
                                                odom_quat, current_time,
                                                'mouse_footprint', 'odom')

            # publish the odometry message over ROS
            odom = Odometry()
        
            odom.header.stamp = current_time
            odom.header.frame_id = "odom"
            odom.child_frame_id = "mouse_footprint"

            # set the position
            odom.pose.pose = Pose(Point(self.odom_x, self.odom_y, 0.0), Quaternion(*odom_quat))

            # set the velocity
            odom.twist.twist = Twist(Vector3(dx/dt, dy/dt, 0), Vector3(0, 0, math.radians(dyaw)/dt))
   
            #publish the message
            #rospy.loginfo("Publishing /odom_mouse_footprint topic...")
            self.odom_pub.publish(odom)

            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('triffid_odom_publisher', log_level=LOG_LEVEL)
    rospy.loginfo("Initialising TRIFFID FLW100 sensor...")

    triffid_odom_broadcaster = TriffidOdomBroadcaster(10)
    triffid_odom_broadcaster.broadcast_odom()
    # TODO: STOP RPLIDAR
    rospy.spin()
