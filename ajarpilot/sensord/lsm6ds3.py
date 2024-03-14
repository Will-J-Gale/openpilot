from smbus2 import SMBus

I2C_BUS = 1
LSM6DS3_ADDR = 0x6A
CHIP_ID_ADDR = 0x0F
CHIP_ID = 0b01101010

#Accelerometer
ACCEL_CTRL_ADDR = 0x10
ACCEL_ENABLE_SETTINGS = 0b01000000 #104hz, +-2g, 
ACCEL_DATA_ADDR = 0x28



with SMBus(I2C_BUS) as bus:
    read_id = bus.read_byte_data(LSM6DS3_ADDR, CHIP_ID_ADDR)
    print(f"{read_id:08b}")
    print(read_id == CHIP_ID)
    
    #Enable accelerometer
    bus.write_byte_data(LSM6DS3_ADDR, ACCEL_CTRL_ADDR, ACCEL_ENABLE_SETTINGS)

    while(True):
        try:
            accel_data = bus.read_i2c_block_data(LSM6DS3_ADDR, ACCEL_DATA_ADDR, 6)
            print(accel_data)
        except KeyboardInterrupt:
            break