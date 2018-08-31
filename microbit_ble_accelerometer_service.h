#include "mbed.h"


#ifndef __BLE_LED_SERVICE_H__
#define __BLE_LED_SERVICE_H__

class MetaDataService {
public:
    
    const static uint16_t CUSTOM_SERVICE_UUID           = 0xA000;
    const static uint16_t DEVICE_MQTT_NAME_UUID         = 0xA001;
    const static uint16_t DEVICE_DATA_LAYOUT_UUID       = 0xA002;
    const static uint16_t READ_BOOL_UUID                = 0xA003;

    MetaDataService(BLEDevice &_ble, char* deviceName, char* deviceDataLayout, bool read_bool) :
        ble(_ble), 
        deviceNameChar(DEVICE_MQTT_NAME_UUID, deviceName),
        deviceDataLayoutChar(DEVICE_DATA_LAYOUT_UUID, deviceDataLayout),
        readChar(READ_BOOL_UUID,read_bool)
    {
        GattCharacteristic *charTable[] = {&deviceNameChar, &deviceDataLayoutChar, &readChar};
        GattService         metadataService(CUSTOM_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(metadataService);
    }

    GattAttribute::Handle_t getValueHandle() const
    {
        return ledState.getValueHandle();
    }

private:
    BLEDevice                         &ble;
    ReadWriteGattCharacteristic<char *> deviceName;
    ReadWriteGattCharacteristic<char *> deviceDataLayout;
    ReadWriteGattCharacteristic<bool> deviceName;
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */
