import os
import builtins
from heating_gpio import heating_enable, heating_disable

# 允许执行的内建函数
safe_builtins = {
    "float": builtins.float,
    "len": builtins.len,
    "abs": builtins.abs,
    "str": builtins.str,  # 添加 str，避免 str.split() 报错
    "print": builtins.print,  # 允许打印
}

safe_globals = {
    "__builtins__": safe_builtins,
    "heating_enable": heating_enable,
    "heating_disable": heating_disable,
}


def sm_strategy_load_code(filename):
    fp = open(os.path.join("strategies", filename), "r")
    code = fp.read()
    fp.close()
    return code


def sm_strategy_register(code):
    exec(code, safe_globals)


def sm_strategy_get_desc():
    return safe_globals["heating_description"]()


def sm_strategy_exec(temp_high: float, temp_low: float, working_seconds:float, argument: str):
    return safe_globals["heating_strategy"](temp_high, temp_low, working_seconds, argument)


def sm_strategy_delete(filename):
    try:
        os.remove(os.path.join("strategies", filename))
        return "ok"
    except:
        return "failed"


def sm_strategies_get_all():
    all_desc = []
    try:
        for f in os.listdir("strategies"):
            if os.path.isfile(os.path.join("strategies", f)):
                sm_strategy_register(sm_strategy_load_code(f))
                desc = sm_strategy_get_desc()
                desc["filename"] = f
                all_desc.append(desc)
    except:
        return []

    return all_desc
