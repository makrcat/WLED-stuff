from flask import Flask, request
import os
import logging


app = Flask(__name__)
log = logging.getLogger('werkzeug')
log.setLevel(logging.CRITICAL)

NUM_LEDS = 15
leds = ["000000"] * NUM_LEDS  # all off

def print_leds(leds):
    line = ""
    for h in leds:

        if len(h) != 6:
            h = h.rjust(6, "0") # invalid

        r = int(h[0:2], 16)
        g = int(h[2:4], 16)
        b = int(h[4:6], 16)

        line += f"\033[48;2;{r};{g};{b}m  \033[0m"
    print(f"\r{line}", end="", flush=True)

@app.route("/json", methods=["POST"])
def update_leds():
    try:
        payload = request.get_json()  
        # parse
        updates = payload["seg"][0]["i"]  
        # [index1, hex1, index2, hex2, ...]


        # fill hash map
        for i in range(0, len(updates), 2):
            idx = updates[i]
            hexcode = updates[i + 1]
            leds[idx] = hexcode

        print_leds(leds)

        return "OK"
    except Exception as e:
        print("Error:", e)
        return "Error", 400

if __name__ == "__main__":
    os.system('clear')
    print("Running on http://127.0.0.1:21324/json")
    print("Remember to 'have' enough LEDs to sort.")
    app.run(host="127.0.0.1", port=21324)
