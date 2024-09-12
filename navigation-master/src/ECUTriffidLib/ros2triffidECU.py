# !/usr/bin/env python
import Queue
import threading
import time
import socket
import struct

TCP_IP = '172.16.0.28'  # actual IP adress of triffid
TCP_PORT = 52161  # comunication port
BUFFER_SIZE = 1024


def singleton(class_):
    """ Ensures that will be only one instance of cals"""
    instances = {}

    def getinstance(*args, **kwargs):
        if class_ not in instances:
            instances[class_] = class_(*args, **kwargs)
        return instances[class_]

    return getinstance


@singleton
class Triffid(threading.Thread):
    # Master presence indicator, if there is no other transmission need to be send every 100mS
    TCPIP_COMM_ID_HEART_BEAT = 'H'
    # Write register to triffd
    TCPIP_COMM_ID_WRITE = 'W'
    #READ register to triffd
    TCPIP_COMM_ID_READ = 'R'
    # positive response
    TCPIP_COMM_ID_OK_RESPONSE = 'O'
    # negative response
    TCPIP_COMM_ID_NOK_RESPONSE = 'N'

    (
    TCPIP_STRIGHT_DIR,
    TCPIP_STRIGHT_DIST,
    TCPIP_STRIGHT_VELOCITY,
    TCPIP_TURN_DIR,
    TCPIP_TURN_ANGLE,
    TCPIP_TURN_VELOCITY,
    TCPIP_PERP_DIR,
    TCPIP_PERP_DIST,
    TCPIP_PERP_VELOCITY,
    TCPIP_COLUMN_DIR,
    TCPIP_COLUMN_DIST,
    TCPIP_COLUMN_VELOCITY,
    TCPIP_STATUS) = map(int,xrange(13))

    TCPIP_DRIVE = 0x01
    TCPIP_READY = 0x02
    TCPIP_ERROR = 0x04

    TRIFFID_CONNECTION_CLOSED = 0x00
    TRIFFID_CONNECTION_ESTABLISHED = 0x01
    TRIFFID_READY = 0x10
    TRIFFID_MOVE = 0x20
    TRIFFID_ERROR = 0x40

    TRIFFID_VELOCITY_UPDATE_RATE = 50

    def __init__(self):
        threading.Thread.__init__(self, name="TriffidLibThread")
        self.run_flg = 1
        self.__socket = None
        self.__straight_velo_actual = 0
        self.__perpendicular_velo_actual = 0
        self.__turn_velo_actual = 0
        self.__straight_velo_desired = 0
        self.__perpendicular_velo_desired = 0
        self.__turn_velo_desired = 0
        self.__transmit_buff = Queue.Queue(5)
        self.__receive_buff = Queue.Queue(1)
        self.__status_lock = threading.Lock()
        self.__velocity_lock = threading.Lock()
        self.__actual_processing = 0
        self.__triffid_state = self.TRIFFID_CONNECTION_CLOSED

    def run(self):
        while self.run_flg:
            try:
                self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.__socket.connect((TCP_IP, TCP_PORT))
                self.set_state_bit(self.TRIFFID_CONNECTION_ESTABLISHED)
                data = self.__transmit_buff.get(block=True, timeout=0.1)
                self.__socket.sendall(data)
                data = self.__socket.recv(2048)
                data_id, count, datagram = self.decode_frame(data)
                if self.__receive_buff.full():
                    self.__receive_buff.get()
                self.__receive_buff.put((data_id, count, datagram))
                self.__socket.close()
            except IOError as e:
                if isinstance(e, socket.error):
                    self.set_state(self.TRIFFID_CONNECTION_CLOSED)
                else:
                    print(e)
            except Exception as e:
                if isinstance(e, Queue.Empty):
                    if not self.__send_next_val:
                        self.send_heart_beat()
                else:
                    print(e)

    def get_response(self):
        return self.__receive_buff.get()

    @property
    def __send_next_val(self):
        """
        Function generate velocity slope for smooth triffid operation
        """
        def process(i):
            switcher = {
                0: 'process_straight',
                1: 'process_turn',
                2: 'process_perp'
            }
            method_name = switcher.get(i, lambda: 'Invalid')
            func = getattr(self, method_name, lambda: 'Invalid')
            return func

        self.__velocity_lock.acquire()

        rez = False
        for _ in range(3):
            method = process(self.__actual_processing)
            rez = method()
            self.__actual_processing += 1
            self.__actual_processing = self.__actual_processing % 3
            if rez:
                break

        self.__velocity_lock.release()
        return rez

    def process_perp(self):
        if self.__perpendicular_velo_desired == self.__perpendicular_velo_actual :
            return False
        if self.__perpendicular_velo_desired - self.__perpendicular_velo_actual > \
                self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__perpendicular_velo_actual += self.TRIFFID_VELOCITY_UPDATE_RATE
        elif self.__perpendicular_velo_desired - self.__perpendicular_velo_actual < \
                -self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__perpendicular_velo_actual -= self.TRIFFID_VELOCITY_UPDATE_RATE
        else:
            self.__perpendicular_velo_actual = self.__perpendicular_velo_desired
        self.write_register([(self.TCPIP_PERP_VELOCITY, self.__perpendicular_velo_actual)])
        return True

    def process_turn(self):
        if self.__turn_velo_desired == self.__turn_velo_actual :
            return False
        if self.__turn_velo_desired - self.__turn_velo_actual > self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__turn_velo_actual += self.TRIFFID_VELOCITY_UPDATE_RATE
        elif self.__turn_velo_desired - self.__turn_velo_actual < -self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__turn_velo_actual -= self.TRIFFID_VELOCITY_UPDATE_RATE
        else:
            self.__turn_velo_actual = self.__turn_velo_desired
        self.write_register([(self.TCPIP_TURN_VELOCITY, self.__turn_velo_actual)])
        return True

    def process_straight(self):

        if self.__straight_velo_desired == self.__straight_velo_actual:
            return False
        if self.__straight_velo_desired - self.__straight_velo_actual > self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__straight_velo_actual += self.TRIFFID_VELOCITY_UPDATE_RATE
        elif self.__straight_velo_desired - self.__straight_velo_actual < -self.TRIFFID_VELOCITY_UPDATE_RATE:
            self.__straight_velo_actual -= self.TRIFFID_VELOCITY_UPDATE_RATE
        else:
            self.__straight_velo_actual = self.__straight_velo_desired
        self.write_register([(self.TCPIP_STRIGHT_VELOCITY, self.__straight_velo_actual)])
        print(self.__straight_velo_actual)
        return True

    def send_heart_beat(self):
        self.__socket.sendall(self.encode_frame(self.TCPIP_COMM_ID_HEART_BEAT, [(0xaa, 0xbb)]))
        data = self.__socket.recv(2048)
        data_id, count, datagram = self.decode_frame(data)
        self.analyze_status_response(data_id, count, datagram)

    def analyze_status_response(self, data_id, count, datagram):
        """
        Analyze received response
        :param data_id: Frame Id received from Triffid
        :param count: should be 1 (status response need to be 1)
        :param datagram: datagram contains status register indicator and register values
        """
        if data_id == self.TCPIP_COMM_ID_OK_RESPONSE:
            if count == 1:
                if datagram[0][0] == self.TCPIP_STATUS:
                    if datagram[0][1] & self.TCPIP_ERROR:
                        self.set_state_bit(self.TRIFFID_ERROR)
                    else:
                        self.clear_state_bit(self.TRIFFID_ERROR)
                    if datagram[0][1] & self.TCPIP_DRIVE:
                        self.set_state_bit(self.TRIFFID_MOVE)
                    else:
                        self.clear_state_bit(self.TRIFFID_MOVE)
                    if datagram[0][1] & self.TCPIP_READY:
                        self.set_state_bit(self.TRIFFID_READY)
                    else:
                        self.clear_state_bit(self.TRIFFID_READY)
        else:
            self.set_state_bit(self.TRIFFID_ERROR)

    def close(self):
        self.run_flg = 0

    def decode_frame(self, data):
        """Translate received date to frame
        :param data -data to be translated
        :return tuple of communication frame"""
        try:
            # get data_id amd count from data
            data_id, count, datagram_data = struct.unpack('cB' + str(len(data) - 2) + 's', data)
            datagram = list()
            for i in range(count):
                if i == count-1:
                    # if it is last datagram
                    reg, val = struct.unpack('hh', datagram_data)
                else:
                    # encapsulate all other
                    reg, val, datagram_data = struct.unpack('hh' + str(len(datagram_data) - 4) + 's', datagram_data)
                datagram.append((reg, val))
            return data_id, count, datagram
        except Exception as e:
            print(e)
            return None, None, None

    def encode_frame(self, id, data):
        """Convert frame into data to be send to triffid
        :param id - id of frame
        :param data - list of tuples containing pairs (reg, data) where reg means -register to be read/write and
         data - data to be written or dummy if it is read command
        """
        tmp = struct.pack('cBhh', id, len(data), data[0][0], data[0][1])
        i = 1
        while i < len(data):
            tmp += struct.pack('hh',  data[i][0], data[i][1])
            i += 1
        return tmp

    def __send(self, msg):
        totalsent = 0
        MSGLEN = len(msg)
        while totalsent < MSGLEN:
            sent = self.__socket.send(msg[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def __receive(self):
        chunks = []
        bytes_recd = 0
        MSGLEN = 2
        while bytes_recd < MSGLEN:
            chunk = self.__socket.recv(min(MSGLEN - bytes_recd, 2048))
            if chunk == '':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return ''.join(str(chunks))

    def write_register(self, registers):
        """
        Function write specyfic register to triffd
        :param registers: is list of tuples regiter to be written
        """
        if isinstance(registers, list):
            tmp = self.encode_frame(self.TCPIP_COMM_ID_WRITE, registers)
            self.__transmit_buff.put(tmp)

    def read_register(self, registers):
        """
        Function write specyfic register to triffd
        :param registers: is list of tuples regiter to be written
        """
        if isinstance(registers, list):
            tmp = self.encode_frame(self.TCPIP_COMM_ID_READ, registers)
            self.__transmit_buff.put(tmp)
            return self.__receive_buff.get(True, 1)

    def set_state(self, new_state):
        """
        Set given state
        :param new_state -state to be set
        :return:
        """
        self.__status_lock.acquire()
        self.__triffid_state = new_state
        self.__status_lock.release()

    def clear_state_bit(self, state_bit):
        """
        Function clears specified bit
        :param state_bit - bit to be cleared
        :return:
        """
        self.__status_lock.acquire()
        self.__triffid_state &= ~state_bit
        self.__status_lock.release()

    def set_state_bit(self, state_bit):
        """
        Function stets specified bit
        :param state_bit - bit to be setted
        :return:
        """
        self.__status_lock.acquire()
        self.__triffid_state &= ~state_bit
        self.__status_lock.release()

    def get_state(self):
        self.__status_lock.acquire()
        old_state = self.__triffid_state
        self.__status_lock.release()
        return old_state

    def move_straight(self,vel):
        """
        Sets velocity in straight direction. Positive value means forward, negative backward.
        To stop send 0
        :param vel:
        :return:
        """
        self.__velocity_lock.acquire()
        self.__straight_velo_desired = vel
        self.__velocity_lock.release()

    def move_perpendicular(self,vel):
        """
        Sets velocity in perpendicular direction. Positive value means left, negative right.
        To stop send 0
        :param vel:
        :return:
        """
        self.__velocity_lock.acquire()
        self.__perpendicular_velo_desired = vel
        self.__velocity_lock.release()

    def move_column(self, vel):
        """
        Sets velocity in straight direction. Positive value means forward, negative backward.
        To stop send 0
        :param vel:
        :return:
        """
        self.write_register([(self.TCPIP_COLUMN_VELOCITY, vel)])

    def turn(self, vel):
        """
        Sets turn move,  positive values means CCW, negative CW.
        To stop send 0
        :param vel:
        :return:
        """
        self.__velocity_lock.acquire()
        self.__turn_velo_desired = vel
        self.__velocity_lock.release()
    def e_stop(self):
        """
        Perform immediately stop
        """
        self.__velocity_lock.acquire()
        self.__turn_velo_desired = 0
        self.__perpendicular_velo_desired = 0
        self.__straight_velo_desired = 0
        self.__turn_velo_actual = self.__turn_velo_desired + self.TRIFFID_VELOCITY_UPDATE_RATE
        self.__perpendicular_velo_actual = self.__perpendicular_velo_desired + self.TRIFFID_VELOCITY_UPDATE_RATE
        self.__straight_velo_actual = self.__straight_velo_desired + self.TRIFFID_VELOCITY_UPDATE_RATE
        self.__velocity_lock.release()


if __name__ == "__main__":

    a = Triffid()

    a.start()

    # i = 0
    print("a.move_straight(300)")
    a.move_straight(250)
    time.sleep(15)
    print("a.turn(0)")
    a.move_straight(0)


    # a.turn(-250)
    # time.sleep(1)
    # print("a.turn(0)")
    # a.turn(0)


    a.close()
    a.join()