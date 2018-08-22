import paho.mqtt.client as mqtt

mqtthost='localhost'
mqttuser='ckl41'
mqttpass = 'S!ngapor3'
mqtttopic='test.data'

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(mqtttopic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

def on_publish(client, userdata, mid):
    print("Data published (Mid: %s)" % mid)

client = mqtt.Client()
#client.username_pw_set(mqttuser,mqttpass)
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish
client.connect(mqtthost, 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

client.loop_start()

for i in range(5):
    client.publish(mqtttopic,"Hello World",1)

import time
time.sleep(4)
client.loop_stop()
client.disconnect()