from smbus2 import SMBus

I2C_BUS = 1
LSM6DS3_ADDR = 0x6A
CHIP_ID_ADDR = 0xFA


with SMBus(I2C_BUS) as bus:
    b = bus.read_byte_data(CHIP_ID_ADDR, 8)
    print(f"{b:08b}")