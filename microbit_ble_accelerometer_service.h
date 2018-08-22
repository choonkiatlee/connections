#ifndef __BLE_ACCELEROMETER_SERVICE_H__
#define __BLE_ACCELEROMETER_SERVICE_H__
 
class AccelerometerService {
public:
    const static uint16_t ACCEL_SERVICE_UUID              = 0xA000;
    const static uint16_t ACCEL_STATE_CHARACTERISTIC_UUID = 0xA001;
 
    AccelerometerService(BLE &_ble, bool buttonPressedInitial) :
        ble(_ble), accelState(ACCEL_STATE_CHARACTERISTIC_UUID, &buttonPressedInitial, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&accelState};
        GattService         buttonService(AccelerometerService::ACCEL_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(buttonService);
    }
 
    void updateAccelState(bool newState) {
        ble.gattServer().write(accelState.getValueHandle(), (uint8_t *)&newState, sizeof(bool));
    }
 
private:
    BLE                              &ble;
    ReadOnlyGattCharacteristic<bool>  accelState;
};
 
#endif /* #ifndef __BLE_ACCELEROMETER_SERVICE_H__ */