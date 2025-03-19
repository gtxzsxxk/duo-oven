import os
import builtins

selected_strategy = ""

def sm_get_all():
    for f in os.listdir("strategies"):
        file_path = os.paht.join("strategies", f)
        if os.path.isfile(os.paht.join("strategies", f)):
            fp = open(file_path, "r")
            code = fp.read()
            