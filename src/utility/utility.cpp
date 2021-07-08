#include "utility/CategorizedStepTimer.hpp"
#include "utility/StepTimer.hpp"

std::ostream &operator <<(std::ostream &out, const NamedStatsContainer &container) {
    formatWidth(out, container.name(), 50);

    if (container.stepCount() <= 0) {
        out << "took: ";
        formatTime(out, 0);
        out << " min: ";
        formatTime(out, 0);
        out << " avg: ";
        formatTime(out, 0);
        out << " max: ";
        formatTime(out, 0);
        out << " std: ";
        formatTime(out, 0);
        out << " over: " << 0;
        return out;
    }

    out << "took: ";
    formatTime(out, container.total());

    out << " min: ";
    formatTime(out, container.min());

    out << " avg: ";
    formatTime(out, container.avg());

    out << " max: ";
    formatTime(out, container.max());

    if (container.stepCount() > 1) {
        out << " std: ";
        formatTime(out, container.std());
    } else {
        out << " std: ";
        formatTime(out, 0);
    }

    out << " over: " << container.stepCount();
    return out;
}

std::ostream &operator <<(std::ostream &out, const CategorizedStepTimer &timer) {
    for (auto &category : timer.categories) {
        out << std::string(timer.indent, ' ');
        out << category.second;
        out << std::endl;
    }

    return out;
}

std::ostream &operator <<(std::ostream &out, const StepTimer &timer) {
    if (timer.stepCount() <= 0) {
        std::cout << "took: ";
        formatTime(out, 0);
        std::cout << " min: ";
        formatTime(out, 0);
        std::cout << " avg: ";
        formatTime(out, 0);
        std::cout << " max: ";
        formatTime(out, 0);
        std::cout << " std: ";
        formatTime(out, 0);
        std::cout << " over: " << 0;
        return out;
    }

    std::cout << "took: ";
    formatTime(out, timer.totalTimeNs());
    std::cout << " min: ";
    formatTime(out, timer.minTimeNs());
    std::cout << " avg: ";
    formatTime(out, timer.avgTimeNs());
    std::cout << " max: ";
    formatTime(out, timer.maxTimeNs());
    std::cout << " std: ";

    if (timer.stepCount() > 1) {
        formatTime(out, timer.stdTimeNs());
    } else {
        formatTime(out, 0);
    }

    std::cout << " over: " << timer.stepCount();

    return out;
}
