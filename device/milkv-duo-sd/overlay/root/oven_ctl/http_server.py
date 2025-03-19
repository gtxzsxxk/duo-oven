from flask import Flask
from oven_lib import oven_init, oven_temp_internal, oven_temp_external, camera_capture_base64
import random

app = Flask(__name__)
oven_init()

@app.route('/')
def ui():
    ui_fp = open("ui/index.html", "r")
    html = ui_fp.read()
    ui_fp.close()
    return html

@app.route('/temperature/intenal')
def temp_internal():
    return {
        "value": oven_temp_internal()
    }

@app.route('/temperature/external')
def temp_external():
    return {
        "value": oven_temp_external()
    }

@app.route('/camera/capture')
def capture():
    return {
        "base64": camera_capture_base64()
    }

if __name__ == '__main__' :
    app.run(host='0.0.0.0',port=9950,debug=False)
