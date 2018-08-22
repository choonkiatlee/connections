import pika

connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
channel = connection.channel()

channel.queue_declare(queue='test.data')


def callback(ch, method, properties, body):
    print(" [x] Received %r" % body)

### To receive from mqtt, we need to add a binding from amq.topic to test.data! 
channel.basic_consume(callback,
                      queue='test.data',
                      no_ack=True)

print(' [*] Waiting for messages. To exit press CTRL+C')
channel.start_consuming()