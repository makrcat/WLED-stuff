import requests
import time
import json
import random

from utils import *

url = 'http://127.0.0.1:21324/'
#"http://wled-001.local/json/"

default_brightness = 0.5
wifi = True

class Item():
    def __init__(self, number, full):

        self.l = default_brightness
        self.s = 0.6
        self.number = number
        self.hue = number / full
        self.hex = numbertoHEX(self.hue, self.l, self.s)

    def updateHEX(self):
        self.hex = numbertoHEX(self.hue, self.l, self.s)

    # set
    def glow(self):
        self.l += 0.25
        self.s += 0.25
        self.updateHEX()

    def unglow(self):
        self.l = default_brightness
        self.s = 0.6
        self.updateHEX()

    def setHue(self, hue):
        self.hue = hue
        self.updateHEX()
        return True
    
    #get
    def getNum(self):
        return self.number
    
    def getHEX(self):
        return self.hex
    

def update(lights):

    hexcodes = []

    for light in lights:
        hexcodes.append(light.getHEX())

        print(
            visualizer(" {} ".format(light.number), 
            light.hex), 
            end=""
            )
        
    print()
    
    if not wifi: return

    payload = {
        "on": True,
        "bri": 255,
        "seg":{"i":hexcodes}
    }
    
    try:
        effect_response = requests.post(url, json=payload, headers={'Content-Type': 'application/json'})
        effect_response.raise_for_status()
        print(effect_response.text)
    except requests.exceptions.RequestException as e:
        print(f"oh no {e}")
 
# make random list (rainbow)
length = int(input("how many"))
# ar = generateRandomArray(length)
ar = [Item(i, length) for i in range(length)]
random.shuffle(ar)

def runBubbleSort(ar):
    n = len(ar)
    
    for i in range(n - 1):
        for j in range(n - i - 1):
            if ar[j].getNum() > ar[j + 1].getNum():
                ar[j], ar[j + 1] = ar[j + 1], ar[j] 
            
                update(ar) #

def runBSlight(ar):
    n = len(ar)
    update(ar)
    
    for i in range(n - 1):

        for j in range(n - i - 1):

            a = ar[j]
            b = ar[j + 1]

            a.glow()
            update(ar) #

            b.glow()
            update(ar)

            if a.getNum() > b.getNum():

                ar[j], ar[j + 1] = ar[j + 1], ar[j] 
                update(ar)
                
            a.unglow()
            b.unglow()
            update(ar)

runBSlight(ar)