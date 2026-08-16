def decode_bytes(lowByte, highByte):
    value = (highByte<<8) + lowByte
    if value >= (256*256)//2:
        value = value - (256*256)
    return value