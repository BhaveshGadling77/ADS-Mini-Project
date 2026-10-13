#pragma once

#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>

namespace adsc {

template <typename T, typename Compare = std::less<T>>
class MinHeap {
public:
    explicit MinHeap(Compare comp = Compare()) : comp(comp) {}

    void push(const T& val) {
        data.push_back(val);
        siftUp(data.size() - 1);
    }

    void push(T&& val) {
        data.push_back(std::move(val));
        siftUp(data.size() - 1);
    }

    const T& top() const {
        if (data.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        return data[0];
    }

    void pop() {
        if (data.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        data[0] = std::move(data.back());
        data.pop_back();
        if (!data.empty()) {
            siftDown(0);
        }
    }

    size_t size() const {
        return data.size();
    }

    bool empty() const {
        return data.empty();
    }

    void clear() {
        data.clear();
    }

private:
    std::vector<T> data;
    Compare comp;

    void siftUp(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            // Min-heap: if child is smaller than parent (comp(child, parent)), swap
            if (comp(data[index], data[parent])) {
                std::swap(data[index], data[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t index) {
        size_t n = data.size();
        while (true) {
            size_t smallest = index;
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;

            if (left < n && comp(data[left], data[smallest])) {
                smallest = left;
            }
            if (right < n && comp(data[right], data[smallest])) {
                smallest = right;
            }

            if (smallest != index) {
                std::swap(data[index], data[smallest]);
                index = smallest;
            } else {
                break;
            }
        }
    }
};

} // namespace adsc
