#pragma once

#include <functional>
#include <utility>
#include <type_traits>
#include <queue>
#include <limits>
#include <numeric>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <atomic>
#include <cassert>

using namespace std;

template<typename T>
class ListIterator;

template<typename T>
class LinkedList;

template<typename T>
class Node
{
    friend class ListIterator<T>;
    friend class LinkedList<T>;

private:
    Node() = default;
    Node(T _value) : value(_value), ref_count(0), prev(this), next(this) { }
    Node(const Node<T>&) = delete;
    void operator=(const Node<T>&) = delete;

    void release() {
        if (--ref_count != 0) {
            return;
        }
        queue<Node<T>*> nodesToDelete;
        nodesToDelete.push(this);
        while (!nodesToDelete.empty()) {
            Node<T>* currentNode = nodesToDelete.front();
            Node<T>* prev = currentNode->prev;
            Node<T>* next = currentNode->next;
            if (prev) {
                prev->ref_count--;
                if (prev->ref_count == 0) {
                    nodesToDelete.push(prev);
                }
            }
            if (next) {
                next->ref_count--;
                if (next->ref_count == 0) {
                    nodesToDelete.push(next);
                }
            }
            nodesToDelete.pop();
            delete (currentNode);
        }
    }

    void acquire() {
        ref_count++;
    }

    T            value;
    atomic<int>  ref_count = 0;
    atomic<bool> deleted = false;
    Node<T>*     prev;
    Node<T>*     next;
    shared_mutex m;
};

template<typename T>
class ListIterator
{
public:
    friend class LinkedList<T>;

    ListIterator() noexcept = default;
    ListIterator(const ListIterator& other) : node(other.node)
    {
        //unique_lock<shared_mutex> lock(node->m);
        node->acquire();
    }
    ListIterator(Node<T>* _node) : node(_node)
    {
        //unique_lock<shared_mutex> lock(node->m);
        node->acquire();
    }

    ~ListIterator() {
        if (!node) {
            cout << "NODE IS NULL WHEN DELETING ITERATOR";
            return;
        }
        //unique_lock<shared_mutex> lock(node->m);
        node->release();
    }

    ListIterator& operator=(const  ListIterator& other) {
        unique_lock<shared_mutex> lock1(node->m);
        if (node == other.node) {
            return *this;
        }
        shared_lock<shared_mutex> lock2(other.node->m);
        node = other.node;
        node->acquire();
        return *this;
    }

    T& operator*() const {
        shared_lock<shared_mutex> lock(node->m);
        if (node->deleted) throw (out_of_range("Invalid index"));
        return node->value;
    }

    T* operator->() const {
        shared_lock<shared_mutex> lock(node->m);
        if (node->deleted) throw (out_of_range("Invalid index"));
        return &(node->value);
    }

    // Prefix
    ListIterator& operator++() {
        unique_lock<shared_mutex> lock(node->m);
        if (!node->next) throw (out_of_range("Invalid index"));

        Node<T>* prevNode = node;
        Node<T>* newNode = node->next;

        while (newNode->deleted && newNode->next) {
            newNode = newNode->next;
        }

        newNode->acquire();
        node = newNode;
        prevNode->release();

        return *this;
    }

    // Postfix
    ListIterator operator++(int) {
        unique_lock<shared_mutex> lock(node->m);
        if (!node->next) throw (out_of_range("Invalid index"));

        Node<T>* prevNode = node;
        ListIterator new_ptr(prevNode);

        Node<T>* newNode = node->next;

        while (newNode->deleted && newNode->next) {
            newNode = newNode->next;
        }

        newNode->acquire();
        node = newNode;
        prevNode->release();

        return new_ptr;
    }

    // Prefix
    ListIterator& operator--() {
        unique_lock<shared_mutex> lock(node->m);
        if (!node->prev) throw out_of_range("Invalid index");

        Node<T>* prevNode = node;
        Node<T>* newNode = node->prev;

        while (newNode->deleted && newNode->prev) {
            newNode = newNode->prev;
        }

        newNode->acquire();
        node = newNode;
        prevNode->release();

        return *this;
    }

    // Postfix
    ListIterator operator--(int) {
        unique_lock<shared_mutex> lock(node->m);
        if (!node->prev) throw out_of_range("Invalid index");

        Node<T>* prevNode = node;
        ListIterator new_ptr(node);

        Node<T>* newNode = node->prev;
        while (newNode->deleted && newNode->prev) {
            newNode = newNode->prev;
        }
        newNode->acquire();
        node = newNode;
        prevNode->release();

        return new_ptr;
    }

    bool isEqual(const ListIterator<T>& other) const {
        return node == other.node;
    }

    friend bool operator==(const ListIterator<T>& a, const ListIterator<T>& b) {
        return a.isEqual(b);
    }

    friend bool operator!=(const ListIterator<T>& a, const ListIterator<T>& b) {
        return !a.isEqual(b);
    }

    operator bool() {
        shared_lock<shared_mutex> lock(node->m);
        return node;
    }

    int debugRefCount()
    {
        shared_lock<shared_mutex> lock(node->m);
        return node->ref_count;
    }

private:
    Node<T>* node;
};

template<typename T>
class LinkedList
{
public:
    using iterator = ListIterator<T>;

    friend class ListIterator<T>;

    LinkedList() : head(nullptr), tail(nullptr), size(0) {
        tail = new Node<T>();
        head = new Node<T>();
        tail->prev = head;
        head->next = tail;

        head->acquire();
        head->acquire();
        tail->acquire();
        tail->acquire();
    }

    LinkedList(const LinkedList& other) = delete;
    LinkedList(LinkedList&& x) = delete;
    LinkedList(initializer_list<T> l) : LinkedList() {
        iterator it(head);
        for (auto item : l) {
            push_back(item);
        }
    }

    ~LinkedList() {
        Node<T>* nextNode;
        for (auto it = head; it != nullptr; it = nextNode) {
            nextNode = it->next;
            delete it;
        }
    }

    LinkedList& operator=(const LinkedList& other) = delete;
    LinkedList& operator=(LinkedList&& x) = delete;

    iterator erase(iterator it) {

        Node<T>* node = it.node;

        if (node == head || node == tail) return it;

        //cout << "delecting a node " << node->value << endl;

        for (bool retry = true; retry;) {
            retry = false;

            if (node->deleted)
                return nullptr;

            //node->m.lock_shared();

            Node<T>* prev = node->prev;
            assert(prev->ref_count);
            prev->acquire();

            Node<T>* next = node->next;
            if (next == nullptr) {
                cout << "\n\n\n OPAAAASDLA{WWDAWPJAWDHOIAWJO:DKLMAWKD\npwjdpawdawjpo\nowawhdlkawd\n\n\n";
            }
            assert(next->ref_count);
            next->acquire();

            //node->m.unlock_shared();

            // RACES

            // Короче блок ниже можно на вот такие локи мувнуть по идее но нужно 
            // глянуть в каком они порядке будут анлочиться поэтому могут и не работать типо
            //{ 
            //    //unique_lock<shared_mutex> prevLock(prev->m);
            //    //shared_lock<shared_mutex> currentLock(node->m);
            //    //unique_lock<shared_mutex> nextLock(next->m);
            //}
            //prev->m.lock();
            //node->m.lock_shared();
            //next->m.lock();
            if (prev->next == node && next->prev == node) {
                prev->next = next;
                next->acquire();
                next->prev = prev;
                prev->acquire();

                node->deleted = true;
                node->release();
                node->release();
                size--;
                cout << endl << size;
            }
            else {
                retry = true;
            }

            prev->release();
            next->release();

            //prev->m.unlock();
            //node->m.unlock_shared();
            //next->m.unlock();
            //cout << "next to erased element value is " << next->value << endl;
        }
        return iterator(node->next);
    }

    void push_front(T value) {
        //head->m.lock();
        iterator it(head);
        //head->m.unlock();
        insert_after(it, value);
    }

    void push_back(T value) {
        Node<T>* node = tail->prev;
        ////node->m.lock();
        iterator it(node);
        ////node->m.unlock();
        insert_after(it, value);
    }

    // Returns iterator on inserted node
    iterator insert_after(iterator it, T value) {
        Node<T>* prev = it.node;
        if (prev == nullptr) return it;
        prev->m.lock();

        Node<T>* next = prev->next;

        for (bool retry = true; retry;) {
            retry = false;

            next = prev->next;
            next->m.lock();

            if (next->prev != prev) {
                cout << "\nBruteforcing\n";
                retry = true;
                next->m.unlock();
            }
        }

        Node<T>* node = new Node<T>(move(value));
        //Node<T>* node = new Node<T>(value);

        node->prev = prev;
        node->next = next;

        prev->next = node;
        node->acquire();
        next->prev = node;
        node->acquire();

        size++;

        prev->m.unlock();
        next->m.unlock();

        return iterator(node);
    }

    iterator begin() noexcept {
        auto node = head->next;
        //unique_lock<shared_mutex> lock(node->m);
        return iterator(node);
    }

    iterator end() noexcept {
        auto node = tail;
        //unique_lock<shared_mutex> lock(node->m);
        return iterator(node);
    }

    bool empty() noexcept {
        //shared_lock<shared_mutex> lock(m);
        return head->next == tail;
    }

    void clear() noexcept {
        iterator current(head->next);
        while (current.node != tail)
        {
            if (*current % 900 == 0 && *current != 0) {
                cout << ":)\n";
            }
            if (*current % 975 == 0 && *current != 0) {
                cout << ":)\n";
            }
            if (*current % 995 == 0 && *current != 0) {
                cout << ":)\n";
            }
            current = erase(current);
        }
    }

    //size_t size() noexcept {
    //    //shared_lock<shared_mutex> lock(m);
    //    return size;
    //}

    shared_mutex m;

    string debug() {
        string output = "\n";
        Node<T>* current = head;

        while (current != tail) {
            output += "\n[val:" 
                + to_string(current->value) 
                + ",ref:" 
                + to_string(current->ref_count) 
                + ",del:" 
                + to_string(current->deleted) 
                + "]\n";
            current = current->next;
        }
        output += "\n";

        return output;
        
    }

    size_t Size() {
        return this->size;
    }

private:

    //iterator insert_internal(iterator it, T value) {
    //    if (it.node == nullptr) return it;

    //    Node<T>* node = new Node<T>{ move(value), 2 };

    //    node->prev = it.node->prev;
    //    node->next = it.node;
    //    it.node->prev->next = node;
    //    it.node->prev = node;

    //    iterator iter(node, this);
    //    size++;
    //    return iter;
    //}

    //iterator internal_erase(iterator position) {
    //    if (position.node->deleted) {
    //        return position;
    //    }

    //    auto output = iterator(position.node->next, this);

    //    inc_ref_count(position.node->next);
    //    inc_ref_count(position.node->prev);

    //    if (position.node == head->next) {
    //        head->next = position.node->next;
    //    }
    //    else {
    //        position.node->prev->next = position.node->next;
    //    }

    //    if (position.node == tail->prev) {
    //        tail->prev = position.node->prev;
    //    }
    //    else {
    //        position.node->next->prev = position.node->prev;
    //    }

    //    size--;
    //    cout << size << " ";

    //    position.node->deleted = true;
    //    dec_ref_count(position.node);
    //    dec_ref_count(position.node);

    //    return output;
    //}


    Node<T>* head;
    Node<T>* tail;
    atomic<size_t> size;
};