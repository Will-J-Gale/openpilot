import time

from smbus2 import SMBus

I2C_BUS = 1
MMC5603NJ_I2C_ADDR = 0x30

#Registers
MMC5603NJ_PRODUCT_ID_REG = 0x39
MMC5603NJ_INTERNAL_CONTROL_1 = 0x1B
MMC5603NJ_INTERNAL_CONTROL_2 = 0x1C
MMC5603NJ_DATA_REG = 0x00

#Settings
MMC5603NJ_PRODUCT_ID = 0b00010000
MMC5603NJ_TAKE_MEASUREMENT = 0b00000001
MMC5603NJ_SET = 0b00001000
MMC5603NJ_RESET = 0b00010000
MMC5603NJ_50_TO_150HZ = 0b00000001


def decode_bytes(lowByte, highByte):
    value = (highByte<<8) + lowByte
    if value >= (256*256)//2:
        value = value - (256*256)
    return value

def read_mag_data(bus:SMBus):
    mag_data = bus.read_i2c_block_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_DATA_REG, 6)
    x = decode_bytes(mag_data[0], mag_data[1])
    y = decode_bytes(mag_data[2], mag_data[3])
    z = decode_bytes(mag_data[4], mag_data[5])
    return x, y, z


def main():
    with SMBus(I2C_BUS) as bus:
        product_id = bus.read_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_PRODUCT_ID_REG)
        assert(product_id, product_id == MMC5603NJ_PRODUCT_ID)

        bus.write_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_INTERNAL_CONTROL_2, MMC5603NJ_50_TO_150HZ)

        while(True):
            try:
                #Set
                bus.write_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_INTERNAL_CONTROL_1, MMC5603NJ_SET)
                time.sleep(0.005)
                bus.write_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_INTERNAL_CONTROL_1, MMC5603NJ_TAKE_MEASUREMENT)
                time.sleep(0.005)

                x, y, z = read_mag_data(bus)

                #Reset
                bus.write_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_INTERNAL_CONTROL_1, MMC5603NJ_RESET)
                time.sleep(0.005)
                bus.write_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_INTERNAL_CONTROL_1, MMC5603NJ_TAKE_MEASUREMENT)
                time.sleep(0.005)

                reset_x, reset_y, reset_z = read_mag_data(bus)

                print(f"SET: {x} - {y} - {z} RESET: {reset_x} - {reset_y} - {reset_z}")

            except KeyboardInterrupt:
                break

if __name__ == "__main__":
    main()