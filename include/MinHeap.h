/**
 * Smart Traffic Route Optimizer
 * Min-Heap Priority Queue Implementation
 * 
 * Used for optimized Dijkstra's algorithm
 * Time Complexity: O(log n) for insert/extract
 */

#ifndef MINHEAP_H
#define MINHEAP_H

#include <vector>
#include <stdexcept>
#include <functional>
#include <unordered_map>

template <typename T, typename Priority = double>
class MinHeap {
private:
    struct HeapNode {
        T data;
        Priority priority;
        HeapNode(const T& d, Priority p) : data(d), priority(p) {}
    };

    std::vector<HeapNode> heap;
    std::unordered_map<T, size_t> indexMap; // For decrease key operation

    // Get parent index
    size_t parent(size_t i) const { return (i - 1) / 2; }

    // Get left child index
    size_t leftChild(size_t i) const { return 2 * i + 1; }

    // Get right child index
    size_t rightChild(size_t i) const { return 2 * i + 2; }

    // Swap two elements and update index map
    void swapNodes(size_t i, size_t j) {
        indexMap[heap[i].data] = j;
        indexMap[heap[j].data] = i;
        std::swap(heap[i], heap[j]);
    }

    // Sift up - move element up to maintain heap property
    void siftUp(size_t i) {
        while (i > 0 && heap[parent(i)].priority > heap[i].priority) {
            swapNodes(i, parent(i));
            i = parent(i);
        }
    }

    // Sift down - move element down to maintain heap property
    void siftDown(size_t i) {
        size_t minIndex = i;
        
        size_t left = leftChild(i);
        if (left < heap.size() && heap[left].priority < heap[minIndex].priority) {
            minIndex = left;
        }
        
        size_t right = rightChild(i);
        if (right < heap.size() && heap[right].priority < heap[minIndex].priority) {
            minIndex = right;
        }
        
        if (i != minIndex) {
            swapNodes(i, minIndex);
            siftDown(minIndex);
        }
    }

public:
    MinHeap() = default;

    // Insert element with priority - O(log n)
    void insert(const T& data, Priority priority) {
        // If element exists, use decrease key instead
        auto it = indexMap.find(data);
        if (it != indexMap.end()) {
            decreaseKey(data, priority);
            return;
        }

        heap.push_back(HeapNode(data, priority));
        size_t index = heap.size() - 1;
        indexMap[data] = index;
        siftUp(index);
    }

    // Push (alias for insert)
    void push(const T& data, Priority priority) {
        insert(data, priority);
    }

    // Extract minimum element - O(log n)
    T extractMin() {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }

        T minData = heap[0].data;
        indexMap.erase(minData);

        if (heap.size() == 1) {
            heap.pop_back();
        } else {
            heap[0] = heap.back();
            indexMap[heap[0].data] = 0;
            heap.pop_back();
            siftDown(0);
        }

        return minData;
    }

    // Pop (alias for extractMin)
    T pop() {
        return extractMin();
    }

    // Get minimum without removing - O(1)
    const T& getMin() const {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        return heap[0].data;
    }

    // Get minimum priority - O(1)
    Priority getMinPriority() const {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        return heap[0].priority;
    }

    // Peek at top element
    std::pair<T, Priority> top() const {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        return {heap[0].data, heap[0].priority};
    }

    // Decrease priority of an element - O(log n)
    void decreaseKey(const T& data, Priority newPriority) {
        auto it = indexMap.find(data);
        if (it == indexMap.end()) {
            // Element not found, insert it
            insert(data, newPriority);
            return;
        }

        size_t index = it->second;
        if (newPriority < heap[index].priority) {
            heap[index].priority = newPriority;
            siftUp(index);
        }
    }

    // Check if element exists
    bool contains(const T& data) const {
        return indexMap.find(data) != indexMap.end();
    }

    // Get priority of an element
    Priority getPriority(const T& data) const {
        auto it = indexMap.find(data);
        if (it == indexMap.end()) {
            throw std::runtime_error("Element not found");
        }
        return heap[it->second].priority;
    }

    // Remove specific element - O(log n)
    bool remove(const T& data) {
        auto it = indexMap.find(data);
        if (it == indexMap.end()) {
            return false;
        }

        size_t index = it->second;
        indexMap.erase(it);

        if (index == heap.size() - 1) {
            heap.pop_back();
        } else {
            heap[index] = heap.back();
            indexMap[heap[index].data] = index;
            heap.pop_back();
            
            // May need to sift up or down
            if (index > 0 && heap[index].priority < heap[parent(index)].priority) {
                siftUp(index);
            } else {
                siftDown(index);
            }
        }
        return true;
    }

    // Check if heap is empty - O(1)
    bool empty() const { return heap.empty(); }
    bool isEmpty() const { return heap.empty(); }

    // Get number of elements - O(1)
    size_t size() const { return heap.size(); }

    // Clear all elements
    void clear() {
        heap.clear();
        indexMap.clear();
    }

    // Build heap from vector - O(n)
    void buildHeap(const std::vector<std::pair<T, Priority>>& elements) {
        clear();
        for (const auto& elem : elements) {
            heap.push_back(HeapNode(elem.first, elem.second));
            indexMap[elem.first] = heap.size() - 1;
        }
        
        // Heapify from bottom up
        for (int i = heap.size() / 2 - 1; i >= 0; --i) {
            siftDown(i);
        }
    }

    // Get all elements (for debugging)
    std::vector<std::pair<T, Priority>> getAll() const {
        std::vector<std::pair<T, Priority>> result;
        for (const auto& node : heap) {
            result.push_back({node.data, node.priority});
        }
        return result;
    }
};

#endif // MINHEAP_H
