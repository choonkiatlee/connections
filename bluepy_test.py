from bluepy.btle import Scanner, DefaultDelegate
import binascii

def strip_trailing_nulls(input_bytes):
    output_bytes = []
    i = input_bytes.find(b'\x00')
    if i == -1:
        return input_bytes
    return input_bytes[:i]

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        # if isNewDev:
        #     print ("Discovered device", dev.addr)
        if isNewData:
            print ("Received new data from", dev.addr)
            for (adtype, desc, value) in dev.getScanData():
                print ("  %s = %s" % (desc, value))
                if desc == 'Manufacturer':
                    #print ('{0}'.format(binascii.b2a_uu(binascii.unhexlify(value))))
                    print ('{0}'.format(binascii.unhexlify(value)))
                if dev.addr == 'f5:5d:fe:85:6e:24' and desc == 'Manufacturer':
                    with open('savefile.log', 'a') as outfile:
                        byte_data = strip_trailing_nulls(binascii.unhexlify(value))
                        outfile.write('{0}\n'.format(byte_data.decode('ascii')))

scanner = Scanner().withDelegate(ScanDelegate())

while True:
    devices = scanner.scan(1.0)

'''
for dev in devices:
    print ("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
    for (adtype, desc, value) in dev.getScanData():
        print ("  %s = %s" % (desc, value))
        if desc == 'Manufacturer':
            #print ('{0}'.format(binascii.b2a_uu(binascii.unhexlify(value))))
            print ('{0}'.format(binascii.unhexlify(value)))
        #print ('{0}'.format(value))
    #if desc != 'Complete Local Name':
    #    print ('{0}'.format(binascii.unhexlify('0'+value)))
'''