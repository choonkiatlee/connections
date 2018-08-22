from flask import Flask,Response
from flask_cors import CORS

from kafka_consumer import KafkaConsumerSimplified


app = Flask(__name__)
cors = CORS(app, resources={r"/*": {"origins": "*"}})


@app.route("/")
def hello():
    return "Hello World!"

@app.route("/test/")
def return_test():
    return "helloworld"

def format_SSE(input_string):
    return 'data {0}\n\n'.format(input_string)



@app.route('/topic/<topicname>', methods=['GET'])
def get_messages(topicname):
    import datetime
    import time, json
    client = KafkaConsumerSimplified(['lubuntukafka:9092'],[topicname])
    date_time_to_search_from = (datetime.datetime.now() - datetime.timedelta(days=0.01)).timestamp()*1000    
    def events():
        output = client.collect_from_time_offsets(time_stamp = date_time_to_search_from, topic=topicname)
        for topic_partition in output:
            for msg in output[topic_partition]:
                # print(msg.value)
                yield 'data: {0}|{1}\n\n'.format(msg.timestamp, msg.value.decode('ascii'))
                # yield format_SSE(json.dumps({'data':msg.value.decode('ascii')}))
        # completed getting all historical events
        # Subscribe and yield from events
        # time.sleep(0.1)
        client.cancel_subscriptions()
        client.start_subscriptions()
        for msg in client.consumer:
            # print(msg.value)
            yield 'data: {0}|{1}\n\n'.format(msg.timestamp, msg.value.decode('ascii'))
            # yield format_SSE(json.dumps({'data':msg.value.decode('ascii')}))

        #yield from client.consumer

    return Response(events(), mimetype="text/event-stream")

if __name__ == "__main__":
    app.run()