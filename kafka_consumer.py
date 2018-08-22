from kafka import KafkaConsumer, TopicPartition

class KafkaConsumerSimplified:

    def __init__(self, bootstrap_servers, topic_strings):
        self.consumer = KafkaConsumer(bootstrap_servers=bootstrap_servers,
                        api_version=(2,0,0),
                        # enable_auto_commit=False,
                        #  auto_offset_reset='earliest', 
                        )

        self.topic_strings = topic_strings
        self.topic_partitions = []
        for topic_string in topic_strings:
            self.topic_partitions.extend(self.get_partitions_for_topic(self.consumer,topic_string))

        self.consumer.assign(self.topic_partitions)

    def collect_from_time_offsets(self,time_stamp=0, topic='',tp_to_timestamps_dict={}, timeout=100):
        
        if time_stamp:
            if tp_to_timestamps_dict:
                print("time_stamp specified. This will overwrite anything in tp_to_timestamps_dict")
            if topic:
                tp_to_timestamps_dict = {tp:time_stamp for tp in self.topic_partitions if tp.topic == topic}
            else:
                tp_to_timestamps_dict = {tp:time_stamp for tp in self.topic_partitions}
        elif not time_stamp and not tp_to_timestamps_dict:
            raise ValueError("Please specify either one of time_stamp or tp_to_timestamps_dict!")
        
        offset_dict = self.consumer.offsets_for_times(tp_to_timestamps_dict)

        for tp in offset_dict:

            if offset_dict[tp] is not None:
                self.consumer.seek(tp,offset_dict[tp].offset)
            else:
                return []
            # print(consumer.position(tp))
        outputs = self.consumer.poll(timeout)
        return outputs


    @staticmethod
    def get_partitions_for_topic(consumer,topic_string):
        return  [TopicPartition(topic_string, partition) for partition in consumer.partitions_for_topic(topic_string)]

    def cancel_subscriptions(self):
        self.consumer.unsubscribe()

    def start_subscriptions(self):
        self.consumer.subscribe(self.topic_strings)



if __name__ == "__main__":
    import datetime

    client = KafkaConsumerSimplified(['lubuntukafka:9092'],['test'])

    date_time_to_search_from = (datetime.datetime.now() - datetime.timedelta(days=0.01)).timestamp()*1000    
    output = client.collect_from_time_offsets(time_stamp = date_time_to_search_from, topic='test')
    print(output)

    client.cancel_subscriptions()
    client.start_subscriptions()

    #print(next(client.consumer))
    for msg in client.consumer:
        print(msg.value)
