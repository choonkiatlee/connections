import ble_mqtt_gateway as 


from collections import namedtuple
import re


test_str = 'accel_x(f)|accel_y(f)|accel_z(f)'

fields = []

struct_unpack_str = ''


#regex explanation:
# \|?                   => optionally escape a | character at the start of the sequence
# (?P<field>.+?)        => named group that matches one or more string characters. Non greedy (?)
# \((?P<format>[a-z])\) => named group that matches the struct character within the brackets

matches = re.findall(r'\|?(?P<field>.+?)\((?P<format>[a-z])\)', test_str)

fields = ','.join([match[0] for match in matches])
struct_unpack_str = ''.join([match[1] for match in matches])


data_format = namedtuple('data',fields)

print(struct_unpack_str, fields)


    