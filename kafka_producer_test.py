from kafka import KafkaConsumer, KafkaProducer
import math, time
import struct

producer = KafkaProducer(bootstrap_servers=['lubuntukafka:9092'], api_version=(2,0,0))


i = 0.0
while(True):
    val = math.cos(i*math.pi)
    future = producer.send('test', "{0:02f}".format(val).encode('ascii'))
    # future = producer.send('test', struct.pack('>f',val))  
    i+=0.1
    time.sleep(2)