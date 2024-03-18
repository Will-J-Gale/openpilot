from eg25_g import GPS

def main():
    gps = GPS()

    while(True):
        data = gps.read()

if __name__ == "__main__":
    main()

