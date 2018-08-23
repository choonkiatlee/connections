from bluepy.btle import Scanner, DefaultDelegate
import binascii

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
                        outfile.write('{0}\n'.format(binascii.unhexlify(value)))

scanner = Scanner().withDelegate(ScanDelegate())

while True:
    devices = scanner.scan(5.0)

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