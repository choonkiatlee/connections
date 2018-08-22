


var MQTTClientSimplified = function(hostname, port, clientid, topics=[]){
    this.hostname = hostname;
    this.port = port;
    this.clientid = clientid;
    this.connected=false;
    this.topics=topics;
    this.client = new Paho.MQTT.Client(this.hostname, Number(this.port), this.clientid);
    
    // called when the client loses its connection
    // set callback handlers
    this.client.onConnectionLost = function (responseObject) {
        if (responseObject.errorCode !== 0) {
            console.log("onConnectionLost:"+responseObject.errorMessage);
        }
    };

    this.Connect = function(){
        this.connected = true;
        console.log("Connected");
        console.log(this.topics);

        if (this.topics.length != 0){
            for (var i = 0; i < this.topics.length; i++){
                this.client.subscribe(this.topics[i]);
                console.log("Subscribed to: " + this.topics[i]);
            }
        }
    }

    // this.Connect_w_subscribe = function(topics){
    //     this.connected = true;
    //     console.log("Connected");
    //     for (topic in topics){
    //         this.client.subscribe(topic);
    //         console.log("Subscribed to: " + topic);
    //     }
    // }

    // if (this.topic==''){
    //     this.onConnect = this.Connect_w_subscribe.bind(this);
    // }
    // else{
    // onConnect = this.Connect.bind(this);  
    this.client.onMessageArrived = MessageHandler;


    // connect the client
    this.client.connect({onSuccess: this.Connect.bind(this)});

    function MessageHandler(message) {

        var messageEvent = new CustomEvent('msgReceived',{detail: {destinationName: message.destinationName, payloadString: message.payloadString, payloadBytes: message.payloadBytes}});
        window.dispatchEvent(messageEvent);

        // console.log("onMessageArrived:"+message.payloadString);
    }

    

    this.send_mqtt_msg = function(message, topic){
        if (isNaN(message)){
            message = message.toString();
        }
        message = new Paho.MQTT.Message(JSON.stringify(message));
        message.destinationName = topic;
        this.client.send(message);
    }

}