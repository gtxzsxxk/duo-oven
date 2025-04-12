from flask import Flask, request, abort
from oven_lib import oven_init, oven_temp_internal_buffered, oven_temp_external_buffered, camera_capture_base64, heating_state_str, oven_strategy_start, oven_strategy_stop, oven_is_heating
from strategy_manager import sm_strategies_get_all, sm_strategy_delete
import os

app = Flask(__name__)
oven_init()

@app.route('/')
def ui():
    ui_fp = open("ui/index.html", "r")
    html = ui_fp.read()
    ui_fp.close()
    return html


@app.route('/css/<css_file_name>')
def ui_css(css_file_name):
    try:
        ui_fp = open("ui/css/" + css_file_name, "r")
        html = ui_fp.read()
        ui_fp.close()
        return html
    except:
        abort(404)


@app.route('/js/<js_file_name>')
def ui_js(js_file_name):
    try:
        ui_fp = open("ui/js/" + js_file_name, "r")
        html = ui_fp.read()
        ui_fp.close()
        return html
    except:
        abort(404)


@app.route('/temperature/intenal')
def temp_internal():
    return {
        "value": oven_temp_internal_buffered()
    }


@app.route('/temperature/external')
def temp_external():
    return {
        "value": oven_temp_external_buffered()
    }


@app.route('/packed_env_info')
def packed_env_info():
    return {
        "temp": {
            "internal": oven_temp_internal_buffered(),
            "external": oven_temp_external_buffered(),
        },
        "heating": {
            "tube_state": heating_state_str(),
            "is_heating": oven_is_heating(),
        },
    }


@app.route('/camera/capture')
def capture():
    return {
        "base64": camera_capture_base64()
    }


@app.route('/heating/get')
def heating_state():
    return {
        "tube_state": heating_state_str(),
        "is_heating": oven_is_heating()
    }


@app.route('/strategy/get')
def strategy_get():
    return {
        "data": sm_strategies_get_all()
    }


@app.route('/strategy/upload', methods=['POST', 'GET'])
def strategy_upload():
    if request.method == 'POST':
        file = request.files['file']
        base_path = os.path.dirname(__file__)
        upload_path = os.path.join(base_path, 'strategies', file.filename)
        file.save(upload_path)
        return {
            "msg": "ok"
        }

    return {
        "msg": "failed"
    }


@app.route('/strategy/delete/<filename>')
def strategy_delete(filename):
    return {
        "msg": sm_strategy_delete(filename)
    }


@app.route('/strategy/enable/<filename>/<arg>')
def strategy_enable(filename, arg):
    oven_strategy_start(filename, arg)
    return {
        "msg": "ok"
    }


@app.route('/strategy/disable')
def strategy_disable():
    oven_strategy_stop()
    return {
        "msg": "ok"
    }


if __name__ == '__main__' :
    app.run(host='0.0.0.0',port=9950,debug=False)
