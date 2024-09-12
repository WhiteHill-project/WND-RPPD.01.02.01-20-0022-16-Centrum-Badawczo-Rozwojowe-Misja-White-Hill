#!/usr/bin/env python
import math
import time
import random

import rospy
import tf
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Pose, Quaternion, Twist, Vector3, TransformStamped

from settings import LOG_LEVEL


class TriffidOdomPublisher:

    def __init__(self, rate):
        rospy.loginfo("Creating odom_base_footprint publisher...")
        self.rate = rate
        self.odom_pub = rospy.Publisher('odom_bug', Odometry, queue_size=self.rate)

        rospy.loginfo("Creating transform odom base_footprint frames listener...")
        self.listener = tf.TransformListener()
        self.listener.waitForTransform('/odom', '/base_footprint', rospy.Time(), rospy.Duration(20.0))
        self._x = 0
        self._y = 0

    def publish_odom(self):

        rate = rospy.Rate(self.rate)

        while not rospy.is_shutdown():
            try:
                (trans, rot) = self.listener.lookupTransform('/odom', '/base_footprint', rospy.Time(0))
            except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
                continue

            # publish the odometry message over ROS
            odom = Odometry()

            current_time = rospy.Time.now()
            odom.header.stamp = current_time
            odom.header.frame_id = "odom"
            odom.child_frame_id = "base_footprint"

            # set the position
            odom.pose.pose = Pose(Point(trans[0], trans[1], trans[2]), Quaternion(*rot))

            # set the velocity
            odom.twist.twist = Twist(Vector3(0, 0, 0), Vector3(0, 0, 0))
   
            #publish the message
            # rospy.loginfo("Publishing /odom topic...")
            self.odom_pub.publish(odom)

            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('triffid_odom_publisher', log_level=LOG_LEVEL)

    triffid_odom_publisher = TriffidOdomPublisher(10)
    triffid_odom_publisher.publish_odom()
    rospy.spin()
