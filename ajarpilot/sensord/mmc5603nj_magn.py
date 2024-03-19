from smbus2 import SMBus

I2C_BUS = 1

MMC5603NJ_I2C_ADDR = 0x30
MMC5603NJ_PRODUCT_ID_REG = 0x39
MMC5603NJ_PRODUCT_ID = 0b00010000

def main():
    with SMBus(I2C_BUS) as bus:
        product_id = bus.read_byte_data(MMC5603NJ_I2C_ADDR, MMC5603NJ_PRODUCT_ID_REG)
        print(product_id, product_id==MMC5603NJ_PRODUCT_ID)

if __name__ == "__main__":
    main()