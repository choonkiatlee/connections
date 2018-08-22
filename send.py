import pika

credentials = pika.PlainCredentials('ckl41', 'S!ngapor3')

connection = pika.BlockingConnection(pika.ConnectionParameters('localhost', 5672, '/',credentials))
channel = connection.channel()


channel.queue_declare(queue='test.data')

channel.basic_publish(exchange='amq.topic',
                      routing_key='test.data',
                      body='Hello World!')
print(" [x] Sent 'Hello World!'")

connection.close()
