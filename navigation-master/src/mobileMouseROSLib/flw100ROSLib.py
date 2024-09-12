# !/usr/bin/env python

import serial as serport
from serial.tools import list_ports
import serial as py_ser
from time import sleep


class Flw100:

    def __init__(self):
        self.port = py_ser.Serial(self.find_port())
        self.port.baudrate = 115200
        self._x = 0
        self._y = 0
        self._pitch = 0
        self._roll = 0
        self._yaw = 0
        if not self._check_if_flw():
            print("FLW100 Not found")
            raise IOError

    def __del__(self):
        self.port.close()

    def set_zero(self):
        '''
        Set new point of origin by resetting the counters
        '''
        self.write_read("%ZERO")

    def get_x_y(self):
        '''
        Return count measurements in .1 mm
        '''

        response = self.write_read("?MM 0")
        # print("FLW100 x_y response: {}".format(response))
        if len(response) > 5:
            parameters = response.split(":")
            parameters[0] = parameters[0].split("=")[1]
            self._x = int(parameters[0])
            self._y = int(parameters[1])
            return tuple(int(i) for i in parameters)
        return (self._x, self._y)

    def get_p_r_y(self):
        '''
        Return AHRS output in roll, pitch and yaw
        '''
        response = self.write_read("?EO 0")
        # print("FLW100 p_r_y response: {}".format(response))
        if len(response) > 5:
            parameters = response.split(":")
            parameters[0] = parameters[0].split("=")[1]
            self._pitch = int(parameters[0])
            self._roll = int(parameters[1])
            self._yaw = int(parameters[2])

            return tuple(int(i) for i in parameters)

        return (self._pitch, self._roll, self._yaw)

    def get_quatermion(self):
        '''
        Return AHRS output in quaternions
        '''
        response = self.write_read("?QO 0")
        parameters = response.split(":")
        parameters[0] = parameters[0].split("=")[1]

        return tuple(int(i) for i in parameters)

    def send_command(self, command):
        response = self.write_read(command)
        
        return response

    def get_velocity_x_y(self):
        '''
        Return speed measurements in .1 mm/s
        '''
        response = self.write_read("?SMM 0")
        parameters = response.split(":")
        parameters[0] = parameters[0].split("=")[1]

        return tuple(int(i) for i in parameters)

    def write_read(self, tx):
        self.port.writelines(tx+'\r')
        sleep(0.01)

        return self.port.read_all()

    def calibrate_mouse(self):
        response = False

        return response

    def _check_if_flw(self):
        rez = False
        if self.port.is_open:
            print("start")
            self.write_read("# C")
            resp = self.write_read("?trn")
            if resp.find("FLW100") > -1 :
                rez = True
            print(resp)

        return rez

    @staticmethod
    def find_port():
        port = None
        for i in list_ports.comports():
            print(i.description)
            if i.description.find("STMicroelectronics") > -1 or i.description.find("RoboFlow") > -1:
                port = i.device

        return port


if __name__ == "__main__":

    ms = Flw100()
    ms.set_zero()
    # print(ms.send_command("^ZGYR 0"))
    # print(ms.send_command("^ZACC 0"))
    # print(ms.send_command("^ZMAG 0"))
    # print(ms.send_command("%CLMOD 0"))
    # print(ms.send_command("^EQS 0"))
    # print(ms.send_command("^ZMT 0"))
    #
    # sleep(5)
    while True:
        print(ms.get_x_y())
        print(ms.get_p_r_y())
        sleep(0.1)
