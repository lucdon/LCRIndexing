#pragma once

template<typename ContainerType>
void swapAndPop(ContainerType &container, size_t index) {
    // Swap the element with the back element.
    // As long as it is not the back element itself.
    if (index + 1 != container.size()) {
        std::swap(container[index], container.back());
    }

    // Pop the back of the container, deleting our old element.
    container.pop_back();
}

inline bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}