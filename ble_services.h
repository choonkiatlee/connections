#ifndef __BLE_SERVICE_H__
#define __BLE_SERVICE_H__

class BLEService{

    private:
        const static uint16_t customServiceUUID     = 0xA000;
        const static uint16_t deviceMQTTNameUUID    = 0xA001;
        const static uint16_t deviceDataLayoutUUID  = 0xA002;
        const static uint16_t checksumUUID          = 0xA003;
        const static uint16_t startBroadcastUUID    = 0xA004;
        
        char device_MQTT_Name[30];
        char device_Data_Layout[50];

        
        
        GattService* service_ptr;

        bool* startBroadcastBool;

    public:

        char checksum[2];
        
        void setup_custom_service(BLE* ble);

        uint16_t get_djb2_hash(char* s);

        void store_hash_into_bytes(const char * deviceMQTTName, const char * deviceDataLayout, char* output_buffer);

        GattAttribute::Handle_t getBroadcastValueHandle() const;

        GattCharacteristic* characteristics[4];

        BLEService(const char* deviceMQTTName, const char* deviceDataLayout, uint8_t startBroadcast){
            strcpy(device_MQTT_Name,deviceMQTTName);
            strcpy(device_Data_Layout,deviceDataLayout);
            
            /* Set Up custom Characteristics */
            ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(device_MQTT_Name)> devNameChar(deviceMQTTNameUUID, (uint8_t*)device_MQTT_Name);
            ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(device_Data_Layout)> devLayoutChar(deviceDataLayoutUUID, (uint8_t*) device_Data_Layout);
            ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(checksum)> checksumChar(checksumUUID, (uint8_t*) checksum);
            ReadWriteGattCharacteristic<uint8_t> startBroadcastChar(startBroadcastUUID, &startBroadcast);
            
            BLEService::store_hash_into_bytes(device_MQTT_Name,device_Data_Layout,checksum); 
            characteristics[0] = &devNameChar;
            characteristics[1] = &devLayoutChar;
            characteristics[2] = &checksumChar;
            characteristics[3] = &startBroadcastChar;
            
            GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));
            service_ptr = &customService;
        }
};

void BLEService::setup_custom_service(BLE* ble){
    /* Set up custom service */
    //GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));
    ble->addService(*service_ptr);
}

GattAttribute::Handle_t BLEService::getBroadcastValueHandle() const {
    return characteristics[3]->getValueHandle();
}

uint16_t BLEService::get_djb2_hash(char* s)
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

void BLEService::store_hash_into_bytes(const char * deviceMQTTName, const char * deviceDataLayout, char* output_buffer){
    
    uint8_t* msg_ptr = (uint8_t*) output_buffer;
    
    char * str_store = (char* ) malloc(1 + strlen(deviceMQTTName) + strlen(deviceDataLayout));

    strcat(str_store, deviceMQTTName);
    strcat(str_store, deviceDataLayout);

    uint16_t hash = BLEService::get_djb2_hash(str_store);

    for (int i = 0; i< sizeof(hash); i++){
        *msg_ptr = (hash >> 8 * i) & 0xFF;
        msg_ptr++;
    }
    free(str_store);
}

#endif