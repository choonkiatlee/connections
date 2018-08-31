    #include "mbed.h"
    #include "ble/BLE.h"
    #include "ble/GapAdvertisingData.h"
    #include "BLEProtocol.h"

    #include "MMA8652.h"
    #include "ble_message.h"



    /* Optional: Device Name, add for human read-ability */
    const static char     DEVICE_NAME[] = "ChangeMe!!";
    
    uint16_t customServiceUUID      = 0xA000;

    uint16_t deviceMQTTNameUUID     = 0xA001;
    uint16_t deviceDataLayoutUUID   = 0xA002;
    
    /* Set Up custom Characteristics */
    static char deviceMQTTName[30] = "home/microbit/accel_sensor";
    ReadWriteArrayGattCharacteristic<uint8_t, sizeof(deviceMQTTName)> devNameChar(deviceMQTTNameUUID, (uint8_t*)deviceMQTTName);

    static char deviceDataLayout[50] = "accel_x(f)|accel_y(f)|accel_z(f)";
    ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(deviceDataLayout)> devLayoutChar(deviceDataLayoutUUID, (uint8_t*) deviceDataLayout);

    //static char dataFieldNames[30] = "accel_x(4F)|accel_y(4F)|accel_z(4F)";
    //ReadWriteArrayGattCharacteristic<uint8_t, sizeof(dataFieldNames)> readChar(readCharUUID, (uint8_t*) dataFieldNames);

    /* Set up custom service */
    GattCharacteristic *characteristics[] = {&devNameChar, &devLayoutChar};
    GattService        customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));
    
    size_t buffer_length=20;
    char buffer[buffer_length];

    
    MMA8652 acc( P20, P19);

    float acc_data[3];

    static volatile bool  triggerSensorPolling = false; 

    BLEMessage msg;



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
        ble.addService(customService);
    
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

    void readAccelSensors(BLE* ble, float * acc_data)
    {

        acc.ReadXYZ(acc_data); 

        memset(&buffer[0], 0, sizeof(buffer));      //clear out buffer
        //uint8_t total_chars =  sprintf (buffer, "%4.2f %4.2f %4.2f", acc_data[0], acc_data[1], acc_data[2]);

        floatToByteArray(acc_data, (uint8_t*) buffer);

        msg.set_message_data(buffer,buffer_length);

        ble->gap().accumulateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, (uint8_t* ) msg.message, sizeof(msg.message));
        ble->gap().startAdvertising();

        /* Note that the periodicCallback() executes in interrupt context, 
        * so it is safer to do
        * heavy-weight sensor polling from the main thread. */
        triggerSensorPolling = false; // sets TRUE and returns to main()
    }
    
    /*
    *  Main loop
    */
    int main(void)
    {
        /* initialize stuff */        
        BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
        ble.init(bleInitComplete);

        Ticker ticker;
        ticker.attach(periodicCallback, 1); // calls the callback function every second

        msg.set_message_headers(BLEMessage::DATA, deviceMQTTName, deviceDataLayout);

        
        /* SpinWait for initialization to complete. This is necessary because the
        * BLE object is used in the main loop below. */
        while (ble.hasInitialized()  == false) { /* spin loop */ }
    
        /* Infinite loop waiting for BLE interrupt events */
        while (true) {
            
            if (triggerSensorPolling){
                readAccelSensors(&ble , msg, acc_data);
            }
            else{
                ble.waitForEvent(); /* Save power */
            }
            
        }
    }
    
                