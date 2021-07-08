import sys


def exeExtension():
    if sys.platform.startswith("win32"):
        return ".exe"
    else:
        return ""


def parseTime(input):
    split = input.split(' ')

    timeNum = float(split[0])
    timeScale = split[1].strip()

    if timeScale == "ns":
        return timeNum / 1000.0 / 1000.0

    if timeScale == "µs":
        return timeNum / 1000.0

    if timeScale == "ms":
        return timeNum

    if timeScale == "s":
        return timeNum * 1000.0

    if timeScale == "mins":
        return timeNum * 60.0 * 1000.0

    if timeScale == "hours":
        return timeNum * 60.0 * 60.0 * 1000.0

    if timeScale == "days":
        return timeNum * 24.0 * 60.0 * 60.0 * 1000.0

    if timeScale == "weeks":
        return timeNum * 7.0 * 24.0 * 60.0 * 60.0 * 1000.0

    print("found invalid time scale:", timeScale)
    sys.exit(1)


def parseMemory(input):
    split = input.split(' ')

    memoryNum = float(split[0])
    memoryScale = split[1].strip()

    if memoryScale == "B":
        return memoryNum / 1000.0 / 1000.0
    if memoryScale == "KB":
        return memoryNum / 1000.0
    if memoryScale == "MB":
        return memoryNum
    if memoryScale == "GB":
        return memoryNum * 1000.0
    if memoryScale == "TB":
        return memoryNum * 1000.0 * 1000.0
    if memoryScale == "PB":
        return memoryNum * 1000.0 * 1000.0 * 1000.0
    if memoryScale == "EB":
        return memoryNum * 1000.0 * 1000.0 * 1000.0 * 1000.0

    print("found invalid memory scale:", memoryScale)
    sys.exit(1)
