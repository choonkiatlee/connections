    #include "mbed.h"
    #include "ble/BLE.h"
    #include "ble/GapAdvertisingData.h"

    #include "MMA8652.h"

    
    /* Optional: Device Name, add for human read-ability */
    const static char     DEVICE_NAME[] = "ChangeMe!!";
    
    /* You have up to 26 bytes of advertising data to use. */
    const static uint8_t AdvData[] = {0x01,0x02,0x03,0x04,0x05};   /* Example of hex data */
    // const static uint8_t AdvData[] = {"ChangeThisData"};         /* Example of character data */
    
    MMA8652 acc( p20, p19);
    float acc_data[3];
    Serial pc(USBTX, USBRX);


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
    
    /**
     * Callback triggered when the ble initialization process has finished
     */
    void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
    {
        BLE&        ble   = params->ble;
        ble_error_t error = params->error;
    
        if (error != BLE_ERROR_NONE) {
            /* In case of error, forward the error handling to onBleInitError */
            onBleInitError(ble, error);
            return;
        }
    
        /* Ensure that it is the default instance of BLE */
        if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
            return;
        }
        
        /* Set device name characteristic data */
        ble.gap().setDeviceName((const uint8_t *) DEVICE_NAME);
    
        /* Optional: add callback for disconnection */
        ble.gap().onDisconnection(disconnectionCallback);
    
        /* Sacrifice 3B of 31B to Advertising Flags */
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE );
        ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    
        /* Sacrifice 2B of 31B to AdvType overhead, rest goes to AdvData array you define */
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, AdvData, sizeof(AdvData));
    
        /* Optional: Add name to device */
        //ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    
        /* Set advertising interval. Longer interval == longer battery life */
        ble.gap().setAdvertisingInterval(500); /* 100ms */
    
        /* Start advertising */
        ble.gap().startAdvertising();
    }
    
    int main(void)
    {
        BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
    
        /* Initialize BLE baselayer, always do this first! */
        ble.init(bleInitComplete);
    
        /* Infinite loop waiting for BLE events */
        while (true) {
            /* Save power while waiting for callback events */
            //ble.waitForEvent();

            //wait for broadcast a new packet every 10 seconds
            wait(1.0);
            acc.ReadXYZ(acc_data);

            uint8_t buffer[4];
            int n = (int) acc_data[2];

            buffer[0] = (n >> 24) & 0xFF;
            buffer[1] = (n >> 16) & 0xFF;
            buffer[2] = (n >> 8) & 0xFF;
            buffer[3] = n & 0xFF;
            
            pc.printf("MMA8652 ACC: X=%1.4f Y=%1.4f Z=%1.4f\r\n\n", acc_data[0], acc_data[1], acc_data[2]);

            const GapAdvertisingData *add_data_ptr;
            add_data_ptr = ble.gap().getAdvertisingPayload();




        }
    }
    
                