tBleStatus Add_Acc_Service(void);
tBleStatus Free_Fall_Notify(void);
tBleStatus Acc_Update(int16_t *data);
tBleStatus Add_Environmental_Sensor_Service(void);
tBleStatus Temp_Update(int16_t temp);
tBleStatus Press_Update(int32_t press);
tBleStatus Humidity_Update(uint16_t humidity);
void Read_Request_CB(tHalUint16 handle);
