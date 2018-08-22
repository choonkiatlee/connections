from kafka import KafkaProducer


import paho.mqtt.client as mqtt

mqtthost='lubuntukafka'
mqttuser='ckl41'
mqttpass = 'S!ngapor3'
mqtttopic='test'

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(mqtttopic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    future = producer.send(msg.topic, msg.payload)

def on_publish(client, userdata, mid):
    print("Data published (Mid: %s)" % mid)

client = mqtt.Client()
#client.username_pw_set(mqttuser,mqttpass)
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish
client.connect(mqtthost, 1883, 60)

producer = KafkaProducer(bootstrap_servers=['lubuntukafka:9092'], api_version=(2,0,0))


# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

client.loop_forever()
client.disconnect()