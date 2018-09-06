#ifdef __TEST__
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#endif

#ifndef __BLE_MESSAGE__H__
#define __BLE_MESSAGE__H__


class BLEMessage{

    private:
        const static uint8_t msg_len = 26;

    public:
        
        uint8_t message[msg_len];

        BLEMessage(){
            for (int i=0; i< msg_len; i++){
                message[i] = 0;
            }
        }

        enum MessageType_t{
            HEARTBEAT = 0,
            DATA = 1
        };

        enum MessageDataType_t{
            BOOL = 0,
            UINT_8 = 1,
            INT = 2,
            FLOAT = 3
        };

        enum msg_error_t{
            ERROR_VALUE_ERROR = -1,
            NO_ERROR = 0
        };

        void set_message_headers(BLEMessage::MessageType_t message_type, const char * deviceMQTTName, const char * deviceDataLayout);

        void set_message_headers(BLEMessage::MessageType_t message_type, uint8_t* checksum);

        msg_error_t set_message_data(char * buffer, uint8_t buffer_len);
        
        void clear_message();
        
        void get_message_data(uint8_t *buffer, uint8_t buffer_len);

        uint16_t get_djb2_hash(char* s);

        void store_hash_into_bytes(const char * deviceMQTTName, const char * deviceDataLayout, char* output_buffer);
    
};


void BLEMessage::clear_message()
{
    for (int i=0; i< msg_len; i++){
        message[i] = 1;
    }
}

uint16_t BLEMessage::get_djb2_hash(char* s)
{
    /* https://stackoverflow.com/questions/1579721/why-are-5381-and-33-so-important-in-the-djb2-algorithm */
    uint16_t hash = 5381;
    int c;
 
    while((c = *s++))
    {
        /* hash = hash * 33 ^ c */
        hash = ((hash << 5) + hash) ^ c;
        // the above line is an optimized version of the following line:
        // hash = (hash * 33) xor c
        // which is easier to read and understand...
    }
 
    return hash;
}


void BLEMessage::set_message_headers(BLEMessage::MessageType_t message_type, const char * deviceMQTTName, const char * deviceDataLayout)
{

    uint8_t* msg_ptr = message;
    *msg_ptr = message_type;
    msg_ptr++;

    char * str_store = (char* ) malloc(1 + strlen(deviceMQTTName) + strlen(deviceDataLayout));

    strcat(str_store, deviceMQTTName);
    strcat(str_store, deviceDataLayout);

    uint16_t hash = BLEMessage::get_djb2_hash(str_store);

    for (int i = 0; i< sizeof(hash); i++){
        *msg_ptr = (hash >> 8 * i) & 0xFF;
        msg_ptr++;
    }

    free(str_store);
}

void BLEMessage::set_message_headers(BLEMessage::MessageType_t message_type, uint8_t* checksum) 
{ 
    *message = message_type; 
    message++;
    memcpy(message,checksum,2);
} 

BLEMessage::msg_error_t BLEMessage::set_message_data(char * buffer, uint8_t buffer_len)
{
    // Check for buffer_len
    if ((buffer_len + 6) > msg_len){
        return BLEMessage::ERROR_VALUE_ERROR;
    }
    for (uint8_t i=0; i<buffer_len; i++) {
        message[6+i] = buffer[i];
    }
    return BLEMessage::NO_ERROR;
}

void BLEMessage::get_message_data(uint8_t *buffer, uint8_t buffer_len){

    uint8_t *temp;
    temp = message;
    temp = temp+6;

    memcpy(buffer, &message +6, buffer_len);
}

void BLEMessage::store_hash_into_bytes(const char * deviceMQTTName, const char * deviceDataLayout, char* output_buffer){
    
        uint8_t* msg_ptr = (uint8_t*) output_buffer;
        
        char * str_store = (char* ) malloc(1 + strlen(deviceMQTTName) + strlen(deviceDataLayout));

        strcat(str_store, deviceMQTTName);
        strcat(str_store, deviceDataLayout);

        uint16_t hash = BLEMessage::get_djb2_hash(str_store);

        for (int i = 0; i< sizeof(hash); i++){
            *msg_ptr = (hash >> 8 * i) & 0xFF;
            msg_ptr++;
        }

        free(str_store);
    }

#endif

#ifdef __TEST__
int main() 
{

    BLEMessage msg;

    msg.set_message_headers(BLEMessage::DATA);

    //printf("%d" ,msg.message[0]);

    char buffer[] = "string";

    msg.set_message_data(buffer, 7);

    char output_buf[10];
    msg.get_message_data((uint8_t*) output_buf,10);

    printf("%s\n", msg.message);
    uint8_t *test;
    test = msg.message;
    test = test+6;
    printf("%s\n", test);
   



    return 0;
}

#endif