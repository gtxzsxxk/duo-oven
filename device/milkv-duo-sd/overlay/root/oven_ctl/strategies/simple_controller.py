def heating_strategy(temp: float, working_seconds:float, argument: str):
    args = argument.split(' ')
    if len(args) != 3:
        return -1 # 策略执行失败
    target = float(args[0])
    heating_time = float(args[1])
    sensitivity = float(args[2])
    if temp >= target:
        heating_disable(0)
        heating_disable(1)
    else:
        heating_enable(0)
        heating_enable(1)

    if working_seconds > heating_time:
        return 2 # 加热时间满足要求，结束逻辑 

    if abs(target - temp) <= sensitivity:
        return 0 # 达到目标温度
    else:
        return 1 # 未达到目标温度

def heating_description():
    return {
        "name": "简单恒温逻辑",
        "argument": "目标温度 恒温保持时间 敏感度",
        "trusted": True
    }
