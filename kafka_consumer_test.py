from kafka import KafkaConsumer, KafkaProducer
from kafka import TopicPartition
import datetime

# hostnames need to be either all caps or all lowercase!
producer = KafkaProducer(bootstrap_servers=['lubuntukafka:9092'], api_version=(2,0,0))

future = producer.send('test', b'raw_bytes')

# To consume latest messages and auto-commit offsets
consumer = KafkaConsumer(bootstrap_servers=['lubuntukafka:9092'],
                         api_version=(2,0,0),
                        #  auto_offset_reset='earliest', 
                         enable_auto_commit=False)


def get_partitions_for_topic(consumer,topic_string):
    return  [TopicPartition(topic_string, partition) for partition in consumer.partitions_for_topic(topic_string)]

topic_partitions = get_partitions_for_topic(consumer,'test')
consumer.assign(topic_partitions)

date_time_to_search_from = (datetime.datetime.now() - datetime.timedelta(days=0.35)).timestamp()*1000

def collect_from_time_offsets(consumer, tp_to_timestamps_dict):
    offset_dict = consumer.offsets_for_times(tp_to_timestamps_dict)

    for tp in offset_dict:
        consumer.seek(tp,offset_dict[tp].offset)
        # print(consumer.position(tp))
    outputs = consumer.poll(100)
    return outputs

print(collect_from_time_offsets(consumer, {topic_partitions[0]:date_time_to_search_from}))


# for msg in consumer:
#     print (msg)

'''
topic_partitions = []
for topic in consumer.subscription():
    for partition in consumer.partitions_for_topic(topic):
        tp = TopicPartition(topic,partition)
        topic_partitions.append(tp)
'''