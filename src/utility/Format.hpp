#pragma once

template<typename T>
std::ostream &formatWidth(std::ostream &out, const T &val, int maxChars) {
    auto width = out.width(maxChars);
    auto flags = out.setf(std::ios::left);

    out << val;

    out.flags(flags);
    out.width(width);
    return out;
}

template<typename TVal>
std::ostream &formatThreeDigits(std::ostream &out, TVal val) {
    auto precision = out.precision(3);
    auto width = out.width(4);
    auto flags = out.setf(std::ios::right);
    out.unsetf(std::ios::floatfield);

    out << val;

    out.flags(flags);
    out.precision(precision);
    out.width(width);
    return out;
}

std::ostream &formatMemory(std::ostream &out, uint64_t value);

std::ostream &formatTime(std::ostream &out, double timeInNs);

static std::ostream &formatTime(std::ostream &out, std::chrono::duration<double, std::nano> &duration) {
    return formatTime(out, duration.count());
}

static std::ostream &formatTime(std::ostream &out, std::chrono::duration<double, std::micro> &duration) {
    return formatTime(out, duration.count() * 1000.0);
}

static std::ostream &formatTime(std::ostream &out, std::chrono::duration<double, std::milli> &duration) {
    return formatTime(out, duration.count() * 1000.0 * 1000.0);
}