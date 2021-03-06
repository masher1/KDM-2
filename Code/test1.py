import time  # We utilize the “time” library so that we can put the script to sleep for a short period
import board  # Designed to quickly know what pins are available on a device
import busio  # We will be using this modules library for handling the I2C serial protocol
import adafruit_adxl34x # This library contains all the code we need for reading information from our ADXL345 accelerometer
import numpy as np  # used for mathematical operations
import RPi.GPIO as GPIO
from statistics import mean
from queue import Queue

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(15, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)

    GPIO.setup(17, GPIO.OUT)
    GPIO.setup(27, GPIO.OUT)
    GPIO.setup(22, GPIO.OUT)
    
def main():
    total = [0, 0, 0]                               #stores totals
    fallthresh = 3.5                                #if the average acceleration exceeds this fall is detected
    axislimit = 3                                   #number of axis required to be over threshold (1-3) for fall to be detected   
    buffersize = 4                       
    buf = Queue(maxsize = buffersize)               #creates buffer of desired size as a queue (first in first out)
    falldetected = False                            #stores if a fall is detected
    GPIO.output(27, GPIO.LOW)                       #blue LED off
    GPIO.output(22, GPIO.LOW)                       #red LED off

    while not falldetected:
        #print("%f %f %f" % accelerometer.acceleration)
        #print("Motion detected: %s" % accelerometer.events["motion"])
        GPIO.output(17, GPIO.HIGH)                  #turns on green LED
        time.sleep(0.5)
        temp = accelerometer.acceleration           #gets accelerometer data

        #buffer is filling
        if (not buf.full()):                        #Buffer is not full
            buf.put(temp)                           #add item to queue
            total[0] += abs(temp[0])                           #add data to running totals
            total[1] += abs(temp[1])
            total[2] += abs(temp[2])
        #buffer is full
        else:
            temprm = buf.get()
            temprm = temprm
            total[0] -= abs(temprm[0])                           #add data to running totals
            total[1] -= abs(temprm[1])
            total[2] -= abs(temprm[2])
            buf.put(temp)                           #add item to queue
            total[0] += abs(temp[0])                           #add data to running totals
            total[1] += abs(temp[1])
            total[2] += abs(temp[2])
  
        #print(total)
        
        numAxisOverThresh = 0                       #stores the number of axis that are over the threshold

        averages = [0, 0, 0]
        i =0
        
        for axistotal in total:                     #looks at each axis in the total
            axisavg = axistotal/buffersize          #computes average of each axis total         
            if(axisavg > fallthresh):               #if we have an axis over the treshold increase
                numAxisOverThresh += 1              #increase the number of axis over threshold
            averages[i] = axisavg
            i += 1
            
        print(averages)

        if(numAxisOverThresh >= axislimit):         #if too many axis are over the threshold
            falldetected = True                     #a fall is detected
        
        if(falldetected):      
            print("FALL HAS BEEN DETECTED!!!!!")    #if fall detected print alert
            time.sleep(5)
            GPIO.output(17, GPIO.LOW)               #turns off green LED
            GPIO.output(27, GPIO.HIGH)              #turns on blue LED
            buttonPress = False                     #stores if button is pressed
            time.sleep(1)
            count = 0
            
            #gives user 5 seconds to press button or authorities alerted
            while(count < 20 and not buttonPress):
                if GPIO.input(15) == GPIO.HIGH:         #button not pressed
                    falldetected = False                #stores fall not detected
                    buttonpressed = True                #button has been pressed
                time.sleep(0.25)
                count += 1
                
            
            GPIO.output(27, GPIO.LOW)           #turns off blue LED
                    
            #alerts authorities
            if falldetected:                             #button not pressed indicating true fall
                GPIO.output(22, GPIO.HIGH)          #turns on red LED
                time.sleep(5)                       #waits 5 seconds on red alert
                print("Call 911")
                #clears buffer
                while(not buf.empty()):
                    buf.get()
                #resets totals
                total = [0,0,0]
                #sets buffer to not fall to continue in loop
                falldetected = False
                GPIO.output(22, GPIO.LOW)
            
            


if __name__ == "__main__":
    i2c = busio.I2C(board.SCL, board.SDA)  # prepare an I2C connection for our current boards SCL and SDA pins
    accelerometer = adafruit_adxl34x.ADXL345(i2c)  # instantiate the ADXL345 library into our “accelerometer” object

    accelerometer.enable_freefall_detection(threshold=10, time=25)
    accelerometer.enable_motion_detection(threshold=18)
    setup()
    main()
