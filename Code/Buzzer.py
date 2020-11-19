from gpiozero import Buzzer
from time import sleep
buzzer = Buzzer(17) #GPIO Pin

#To make a buzz for one second
buzzer.on()
sleep(1)
buzzer.off()
