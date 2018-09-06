    #include "mbed.h"
    #include "ble/BLE.h"
    #include "ble/GapAdvertisingData.h"
    #include "BLEProtocol.h"

    #include "MMA8652.h"
    #include "ble_message.h"
    #include "ble_services.h"
      
    Serial pc(USBTX, USBRX); // tx, rx

    const size_t buffer_length=20;
    char buffer[buffer_length];

    MMA8652 acc( P20, P19);
    float acc_data[3];

    static volatile bool  triggerSensorPolling = false; 
    BLEMessage msg;

    // const char * deviceMQTTName = "/home/device";
    // const char * deviceDataLayout = "accel_x(f)|accel_y(f)|accel_z(f)";

    // //BLEService service(deviceMQTTName, deviceDataLayout);

    // uint16_t customServiceUUID      = 0xA000;
    // uint16_t deviceMQTTNameUUID     = 0xA001;
    // uint16_t deviceDataLayoutUUID   = 0xA002;
    // uint16_t checksumUUID           = 0xA003;
    // uint16_t broadcastUUID          = 0xA004;

    // static char device_MQTT_Name[30] = "/home/device";
    // static char device_Data_Layout[50] = "accel_x(f)|accel_y(f)|accel_z(f)";
    // static char checksum[2];
    // static bool broadcast = 0;

    // /* Set Up custom Characteristics */

    // ReadWriteArrayGattCharacteristic<uint8_t, sizeof(device_MQTT_Name)> devNameChar(deviceMQTTNameUUID, (uint8_t*)device_MQTT_Name);
    // ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(deviceDataLayout)> devLayoutChar(deviceDataLayoutUUID, (uint8_t*) device_Data_Layout);    
    // ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(checksum)> checksumChar(checksumUUID, (uint8_t*) checksum);
    // WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(broadcast)> broadcastChar(broadcastUUID, (uint8_t*) broadcast);

    // //static char dataFieldNames[30] = "accel_x(4F)|accel_y(4F)|accel_z(4F)";
    // //ReadWriteArrayGattCharacteristic<uint8_t, sizeof(dataFieldNames)> readChar(readCharUUID, (uint8_t*) dataFieldNames);            

    // GattCharacteristic* characteristics[4] = {&devNameChar,&devLayoutChar,&checksumChar, &broadcastChar};
    // GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

    const char * device_MQTT_Name = "/home/device";
    const char * device_Data_Layout = "accel_x(f)|accel_y(f)|accel_z(f)";
    static volatile uint8_t startBroadcastBool = 0;

    BLEService* service_ptr;

    /* Optional: Restart advertising when peer disconnects */
    void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
    {
        BLE::Instance().gap().startAdvertising();
    }

    /**
     * This function is called when the ble initialization process has failed
     */
    void onBleInitError(BLE &ble, ble_error_t error)
    {
        /* Avoid compiler warnings */
        (void) ble;
        (void) error;
        
        /* Initialization error handling should go here */
    }    


    void forceBLEAddressTypePublic(BLE *ble){
        uint8_t address1[10];
        BLEProtocol::AddressType_t addr_type;
        ble->gap().getAddress(&addr_type, address1);
        ble->gap().setAddress(BLEProtocol::AddressType::PUBLIC, address1);
    }

    /**
     * This callback allows the LEDService to receive updates to the ledState Characteristic.
     *
     * @param[in] params
     *     Information about the characteristic being updated.
     */
    void onDataWrittenCallback(const GattWriteCallbackParams *params) {
        // if ((params->handle == service_ptr->getBroadcastValueHandle()) && (params->len == 1)) {
        if (params->len == 1){
            startBroadcastBool = *(params->data);
        }
        // }
        // pc.printf("%d\t%d",params->handle,service_ptr->getBroadcastValueHandle() );
    }

    
    /*
    * Initialization callback
    */
    void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
    {
        BLE &ble          = params->ble;
        ble_error_t error = params->error;
        
        if (error != BLE_ERROR_NONE) {
            return;
        }
        
        forceBLEAddressTypePublic(&ble);
    
        ble.gap().onDisconnection(disconnectionCallback);
    
        /* Setup advertising */
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
        ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
        // ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
        ble.gap().setAdvertisingInterval(1000); // 100ms.
    
        /* Add our custom service */
        // ble.addService(customService);

        BLEService service(device_MQTT_Name, device_Data_Layout, startBroadcastBool);
        service_ptr = &service;
        service.setup_custom_service(&ble);

        ble.gattServer().onDataWritten(onDataWrittenCallback);

        /* Start advertising */
        ble.gap().startAdvertising();
    }

    
    


    void periodicCallback(void)
    {
        /* Note that the periodicCallback() executes in interrupt context, 
        * so it is safer to do
        * heavy-weight sensor polling from the main thread. */
        triggerSensorPolling = true; // sets TRUE and returns to main()
    }

    void floatToByteArray(float floats[], size_t floatArraySize, uint8_t * bytes) {
        
        // if (sizeof(bytes) < 4){
        //     throw 10;
        // }
        
        float f;
        int i,j;
        
        uint8_t sizeof_float;
        
        sizeof_float = sizeof(float);
        
        for (i=0; i<floatArraySize;i++){
            f = floats[i];
        
            unsigned int asInt = *((int*)&f);       // Cast f as a unsigned Int so that we can do bit shifting on f
            
            // iterate through the bytes (4 in this case)
            for (j = 0; j < sizeof_float; j++) {
                
                // For each byte, right shift asInt by the appropriate amount (1 byte at a time) then and it with 0xFF to collect the byte we want
                bytes[i*sizeof_float+j] = (asInt >> 8 * j) & 0xFF;
            }
        }
    }


    void sendAccelData(BLE* ble, float * acc_data)
    {
        
        acc.ReadXYZ(acc_data); 

        memset(&buffer[0], 0, sizeof(buffer));      //clear out buffer
        //uint8_t total_chars =  sprintf (buffer, "%4.2f %4.2f %4.2f", acc_data[0], acc_data[1], acc_data[2]);

        floatToByteArray(acc_data, 3, (uint8_t*) buffer);

        msg.set_message_headers(BLEMessage::DATA, (uint8_t*) service_ptr->checksum);
        msg.set_message_data(buffer,buffer_length);

        ble->gap().accumulateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, (uint8_t* ) msg.message, sizeof(msg.message));
        ble->gap().startAdvertising();

        /* Note that the periodicCallback() executes in interrupt context, 
        * so it is safer to do
        * heavy-weight sensor polling from the main thread. */
        triggerSensorPolling = false; // sets TRUE and returns to main()
    }

    void sendHeartBeat(BLE* ble){
        msg.set_message_headers(BLEMessage::HEARTBEAT, (uint8_t*) service_ptr->checksum);
        ble->gap().accumulateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, (uint8_t* ) msg.message, sizeof(msg.message));
        ble->gap().startAdvertising();
        triggerSensorPolling = false;
    
    }
        
    /*
    *  Main loop
    */
    int main(void)
    {
        /* initialize stuff */
        // BLEMessage::store_hash_into_bytes(device_MQTT_Name,device_Data_Layout,checksum);        
        BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
        ble.init(bleInitComplete);

        Ticker ticker;
        ticker.attach(periodicCallback, 1); // calls the callback function every second
        
        // msg.set_message_headers(BLEMessage::DATA, device_MQTT_Name, device_Data_Layout);
        // msg.set_message_headers(BLEMessage::DATA, (uint8_t*) checksum);
        
        /* SpinWait for initialization to complete. This is necessary because the
        * BLE object is used in the main loop below. */
        while (ble.hasInitialized()  == false) { /* spin loop */ }
    
        /* Infinite loop waiting for BLE interrupt events */
        while (true) {

            if (triggerSensorPolling && (startBroadcastBool != 0)){
                sendAccelData(&ble, acc_data);
                pc.printf("%d",startBroadcastBool);
            }
            else if (triggerSensorPolling && (startBroadcastBool == 0)){
                sendHeartBeat(&ble);
            }
            else{
                ble.waitForEvent(); /* Save power */
            }
            
        }
    }
    
                