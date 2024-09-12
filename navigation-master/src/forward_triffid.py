import argparse
import time

from ECUTriffidLib.ros2triffidECU import Triffid


parser = argparse.ArgumentParser(description='Set speed and velocity of Triffid')
parser.add_argument('velocity', type=int, help='Velocity in RPM')
parser.add_argument('time', type=int, help='Time in sec')
args = parser.parse_args()

if __name__ == "__main__":

    a = Triffid()
    a.start()

    print("Velocity: {}".format(args.velocity))
    print("Time: {}".format(args.time))

    a.move_straight(args.velocity)
    time.sleep(args.time)
    a.move_straight(0)

    a.close()
    a.join()
