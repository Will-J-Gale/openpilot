from smbus2 import SMBus

from ajarpilot.sensors.utils import decode_bytes

I2C_BUS = 1
LSM6DS3_ADDR = 0x6A
CHIP_ID_ADDR = 0x0F
CHIP_ID = 0b01101010

#Accelerometer
ACCEL_CTRL_ADDR = 0x10
ACCEL_ENABLE_SETTINGS = 0b01000000 #104hz, +-2g,
ACCEL_DATA_ADDR = 0x28

GYRO_CTRL_ADDR = 0x11
GYRO_ENABLE_SETTINGS = 0b01000000 #104hz
GYRO_DATA_ADDR = 0x22

def read_accel(bus):
    accel_data = bus.read_i2c_block_data(LSM6DS3_ADDR, ACCEL_DATA_ADDR, 6)
    scale = 9.81 * 2.0 / 32768
    x = decode_bytes(accel_data[0], accel_data[1]) * scale
    y = decode_bytes(accel_data[2], accel_data[3]) * scale
    z = decode_bytes(accel_data[4], accel_data[5]) * scale

    return x, y, z

def read_gyro(bus):
    gyro_data = bus.read_i2c_block_data(LSM6DS3_ADDR, GYRO_DATA_ADDR, 6)
    scale = 8.75 / 1000.0
    x = decode_bytes(gyro_data[0], gyro_data[1]) * scale
    y = decode_bytes(gyro_data[2], gyro_data[3]) * scale
    z = decode_bytes(gyro_data[4], gyro_data[5]) * scale

    return x, y, z

def main():
    with SMBus(I2C_BUS) as bus:
        read_id = bus.read_byte_data(LSM6DS3_ADDR, CHIP_ID_ADDR)
        assert(read_id == CHIP_ID)

        #Enable accel + gyro
        bus.write_byte_data(LSM6DS3_ADDR, ACCEL_CTRL_ADDR, ACCEL_ENABLE_SETTINGS)
        bus.write_byte_data(LSM6DS3_ADDR, GYRO_CTRL_ADDR, GYRO_ENABLE_SETTINGS)

        while(True):
            try:

                accel_x, accel_y, accel_z = read_accel(bus)
                gyro_x, gyro_y, gyro_z = read_gyro(bus)

            except KeyboardInterrupt:
                break

if __name__ == "__main__":
    main()