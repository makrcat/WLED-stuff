import colorsys
import random

# standalone functions

def visualizer(text, hexcode):
    hexint = int(hexcode, 16)
    
    r = hexint >> 16
    g = (hexint >> 8) & 0xFF
    b = hexint & 0xFF
    
    return f'\033[38;2;255;255;255;48;2;{r};{g};{b}m{text}\033[0m'

def numbertoHEX(h, l, s):
    
    r, g, b = [int(c * 255) for c in colorsys.hls_to_rgb(h, l, s)]
    return f'{r:02x}{g:02x}{b:02x}'

def generateRandomArray(length):
    a = [i for i in range(length)]
    random.shuffle(a)
    return a