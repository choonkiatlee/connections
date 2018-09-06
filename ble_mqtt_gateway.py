#from bluepy.btle import Scanner, DefaultDelegate, Peripheral
import bluepy.btle as btle
import binascii

import struct
import re
from collections import namedtuple


class BLESensor:

    search_re = re.compile(b'\|?(?P<field>.+?)\((?P<format>[a-z])\)')

    def __init__(self, device):
        self.device = device
        self.deviceMQTTName = ''

        self.deviceDataFormat = None
        self.struct_unpack_str = ''
    
    @staticmethod
    def strip_trailing_nulls(input_bytes):
        i = input_bytes.find(b'\x00')
        if i == -1:
            return input_bytes
        return input_bytes[:i]

    def interpret_device_data(self,input_bytes):

        # regex match for field
        matches = self.search_re.findall(input_bytes) 
        # matches is a iterable of tuples in this form:
        # [(field, format), (field, format)]

        # Generate usable formats for namedtuple and struct
        fields = ','.join([match[0].decode('ascii') for match in matches])
        self.struct_unpack_str = ''.join([match[1].decode('ascii') for match in matches])

        # create a dataformat in the form of a namedtuple
        self.deviceDataFormat = namedtuple('data',fields)

    def read_device_metadata(self):
        p = btle.Peripheral(self.device.addr)
        readSensor = btle.UUID("0000a000-0000-1000-8000-00805f9b34fb")
        readService = p.getServiceByUUID(readSensor)
        for ch in readService.getCharacteristics():
            if ch.uuid == btle.UUID("0000a001-0000-1000-8000-00805f9b34fb"):
                deviceMQTTName = self.strip_trailing_nulls(ch.read())
                self.deviceMQTTName = deviceMQTTName
            elif ch.uuid == btle.UUID("0000a002-0000-1000-8000-00805f9b34fb"):
                deviceData = self.strip_trailing_nulls(ch.read())
                self.interpret_device_data(deviceData)
            elif ch.uuid == btle.UUID("0000a003-0000-1000-8000-00805f9b34fb"):
                self.checksum = self.strip_trailing_nulls(ch.read())
            elif ch.uuid == btle.UUID("0000a004-0000-1000-8000-00805f9b34fb"):
                ch.write(bytes.fromhex('01'))
        p.disconnect()

    def parse_device_data(self,input_bytes):
        
        if parse_heartbeat(input_bytes):
            self.read_device_metadata()
        else:

            # check checksum
            checksum = input_bytes[1:3]
            print(binascii.hexlify(checksum), self.checksum)

            # obtain message_data
            message_data = input_bytes[6:]

            if self.deviceDataFormat:

                # obtain only data required to unpack
                required_byte_size = struct.calcsize(self.struct_unpack_str)
                data_to_unpack = message_data[:required_byte_size]

                # unpack and return tuple
                unpacked = struct.unpack(self.struct_unpack_str,data_to_unpack)
                return self.deviceDataFormat._make(unpacked)
            else:
                return ''

    @staticmethod
    def parse_heartbeat(input_bytes):

        print(input_bytes[0])

        checksum = input_bytes[1:3]
        if (input_bytes[3:] == 'ble_beacon') and input_bytes[0] == b'1':
            return True
        else:
            return False


def strip_all_nulls(input_bytes):
    output_bytes = ''
    if (len(input_bytes) % 2) != 0:
        raise ValueError("not a multiple of 2!")
    for i in range(0,len(input_bytes),2):
        if input_bytes[i:i+2] != '00':
            output_bytes += input_bytes[i:i+2]
    return output_bytes




class ScanDelegate(btle.DefaultDelegate):

    discovered_devices = {}

    def __init__(self):
        btle.DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        
        if isNewData:
            #print("Received new data from {0}".format(dev.addr))
            # if dev.addr == 'f5:5d:fe:85:6e:24':
            current_device = self.discovered_devices[dev.addr]

            print('Device_name: {0}'.format(current_device.deviceMQTTName))
            for (adtype, desc, value) in dev.getScanData():
                print ("  %s = %s" % (desc, value))
                if desc == 'Manufacturer':
                    data = binascii.unhexlify(value)
                    print(current_device.parse_device_data(data))


        # If a new device is added, add it to the discovered_devices set
        if isNewDev:

            # Need to check that the currently discovered 'new' device has been discovered before
            if dev.addr not in self.discovered_devices:
                print("Added Device {0}".format(dev.addr))
                self.discovered_devices[dev.addr] = BLESensor(dev)

                # if dev.addr == 'f5:5d:fe:85:6e:24': 
                #     self.discovered_devices[dev.addr].read_device_metadata()


        

            #To Do: Check CRC checksum to see if device name / data definitions have changed

scanner = btle.Scanner().withDelegate(ScanDelegate())

while True:

    try:
        devices = scanner.scan(1.0)
    except btle.BTLEException as e:
        if str(e) == 'Device disconnected':
            print('Device has been disconnected successfully')
        else:
            raise e
