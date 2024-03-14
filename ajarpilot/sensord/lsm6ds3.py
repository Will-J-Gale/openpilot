from smbus2 import SMBus

I2C_BUS = 1
LSM6DS3_ADDR = 0x6A
CHIP_ID_ADDR = 0x0F
CHIP_ID = 0b01101010


with SMBus(I2C_BUS) as bus:
    read_id = bus.read_byte_data(LSM6DS3_ADDR, CHIP_ID_ADDR, 8)
    print(f"{read_id:08b}")
    print(read_id == CHIP_ID)