import time  # We utilize the “time” library so that we can put the script to sleep for a short period
import board  # Designed to quickly know what pins are available on a device
import busio  # We will be using this modules library for handling the I2C serial protocol
import adafruit_adxl34x # This library contains all the code we need for reading information from our ADXL345 accelerometer
import numpy as np  # used for mathematical operations
from statistics import mean

def main():
    xVal, yVal, zVal = []
    while True:
        print("%f %f %f" % accelerometer.acceleration)
        print("Motion detected: %s" % accelerometer.events["motion"])
        time.sleep(0.5)
        buf = []
        # temp = temp >> 4  #Shift value by 4 bits to reduce range (1023/16) for smoother transition
        if (len(buf) < 4):  # If the bufcount is less than 4, have it equal the value of the potPin
            buf[len(buf)] = accelerometer.acceleration
            # cumulative += buf[len(buf)] #Add and store buf into cumulative
        else:  # If bufcount is now greater than the buf size
            # value = cumulative >> 2     #value is equal to cumulative/4
            # cumulative = 0

            for i in range(len(buf)):
                xVal[i] = buf[i].x
                yVal[i] = buf[i].y
                zVal[i] = buf[i].z

            xValMean = mean(xVal)
            yValMean = mean(yVal)
            zValMean = mean(zVal)

            print((xValMean,yValMean,zValMean))


if __name__ == "__main__":
    i2c = busio.I2C(board.SCL, board.SDA)  # prepare an I2C connection for our current boards SCL and SDA pins
    accelerometer = adafruit_adxl34x.ADXL345(i2c)  # instantiate the ADXL345 library into our “accelerometer” object

    accelerometer.enable_freefall_detection(10, 25)
    accelerometer.enable_motion_detection()
    main()
