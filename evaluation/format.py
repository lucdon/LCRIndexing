import numpy as np
import pandas as pd

max_time = [
    1.0,
    1.0 * 1000,
    1.0 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000 * 60,
    1.0 * 1000 * 1000 * 1000 * 60 * 60,
    1.0 * 1000 * 1000 * 1000 * 60 * 60 * 24,
    1.0 * 1000 * 1000 * 1000 * 60 * 60 * 24 * 7
]

max_memory = [
    1.0,
    1.0 * 1000,
    1.0 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000 * 1000 * 1000,
    1.0 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000
]

suffixes_memory = ["B ", "KB", "MB", "GB", "TB", "PB"]
suffixes_time = ["ns", "µs", "ms", "s ", "mins", "hours", "days"]

def replaceNaN(value):
    if value < 0:
        return np.nan
    return value

def formatTime(timeInMs):
    timeInMs = formatErrors(timeInMs)

    if type(timeInMs) != float or timeInMs < 0 or np.isnan(timeInMs):
        return timeInMs

    timeInNs = timeInMs * 1000.0 * 1000.0

    for i in range(len(suffixes_time)):
        if timeInNs > max_time[i + 1]:
            continue

        rounded = timeInNs / max_time[i]

        return str(round(rounded, 2)) + " " + suffixes_time[i]

    rounded = timeInNs / max_time[len(suffixes_time)]
    return str(round(rounded, 2)) + " weeks"


def formatMemory(memoryInMBs):
    memoryInMBs = formatErrors(memoryInMBs)

    if type(memoryInMBs) != float or memoryInMBs < 0 or np.isnan(memoryInMBs):
        return memoryInMBs

    memoryInBs = memoryInMBs * 1000.0 * 1000.0

    for i in range(len(suffixes_memory)):
        if memoryInBs > max_memory[i + 1]:
            continue

        rounded = memoryInBs / max_memory[i]

        return str(round(rounded, 2)) + " " + suffixes_memory[i]

    rounded = memoryInBs / max_memory[len(suffixes_memory)]
    return str(round(rounded, 2)) + " EB"


def formatErrors(value):
    if value == -1:
        return "did not run"

    if value == -2:
        return "mem"

    if value == -3:
        return "time"

    if value == -4:
        return "unknown error"

    if value == -5:
        return "did not run"

    if np.isnan(value):
        return "did not run"

    return value


def formatValueWithBest(value, best, formatFunc):
    if value == best:
        return "\\textbf{" + formatFunc(value) + "}"

    return formatFunc(value)


def formatSeriesWithBest(series, bestValues, formatFunc):
    return series.apply(lambda x: formatValueWithBest(x, bestValues[series.name], formatFunc))


def formatDataFrameWithBest(dataFrame, bestValues, formatFunc):
    return dataFrame.apply(lambda x: formatSeriesWithBest(x, bestValues, formatFunc), axis=1)
