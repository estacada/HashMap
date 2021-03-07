#pragma once
#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType> > 
class HashMap {
private:
    size_t K = 4;
    size_t A = 1e7 + 32;
    size_t B = 2e12 + 13;
    size_t MOD = 1e9 + 9;
    Hash hasher;
    size_t sz = 1;
    size_t cnt = 0;
    size_t cap = 1;
    size_t gethash(const KeyType &z) const {
        size_t x = hasher(z);
        x %= MOD;
        x = (x * A) % MOD;
        x = (x + B) % MOD;
        return x;
    }
    std::vector<std::vector<std::pair<KeyType, ValueType>>> t;
    void init() {
        t.resize(2);
        t[0].clear();
        t[1].clear();
        t.shrink_to_fit();
        sz = 1;
        cap = 1;
    }
    void realloc() {
        if (cnt != cap && (cnt == 0 || std::max(cnt, cap) >= std::min(cnt, cap) * K)) {
            if (cnt == 0) {
                init();
                return;
            }
            sz = cap = cnt;
            std::vector<std::vector<std::pair<KeyType, ValueType>>> tmp;
            tmp.resize(sz + 1);
            for (size_t i = 0; i < t.size(); i++) {
                for (const auto &x : t[i]) {
                    size_t value = gethash(x.first);
                    size_t block = value / (MOD / sz + (MOD % sz != 0));
                    tmp[block].push_back(x);
                }
            }
            t = std::move(tmp);
        }
    }
public:
    using viter = typename std::vector<std::pair<KeyType, ValueType>>::iterator;
    using vciter = typename std::vector<std::pair<KeyType, ValueType>>::const_iterator;
    class iterator {
    private:
        std::pair<size_t, viter>x;
        HashMap<KeyType, ValueType, Hash>* ptr;
    public:
        iterator() : ptr(nullptr) {}
        iterator(const iterator& other) {
            x = other.x;
            ptr = other.ptr;
        }
        iterator(size_t st, viter vi, HashMap<KeyType, ValueType, Hash>* hm) : x(std::make_pair(st, vi)), ptr(hm) {}
        std::pair<const KeyType, ValueType>* operator->() const {
            return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&(*x.second));
        }
        std::pair<const KeyType, ValueType>& operator*() {
            return *operator->();
        }
        bool operator==(const iterator& other) const {
            return x == other.x;
        }

        bool operator!=(const iterator& other) const {
            return !(x == other.x);
        }
        iterator& operator++() {
            ++x.second;
            if (x.second == (*ptr).t[x.first].end()) {
                ++x.first;
                while (x.first != (*ptr).sz && (*ptr).t[x.first].empty()) ++x.first;
                x.second = (*ptr).t[x.first].begin();
            }
            return *this;
        }

        iterator operator++(int) {
            auto old(*this);
            ++x.second;
            if (x.second == (*ptr).t[x.first].end()) {
                ++x.first;
                while (x.first != (*ptr).sz && (*ptr).t[x.first].empty()) ++x.first;
                x.second = (*ptr).t[x.first].begin();
            }
            return old;
        }
    };
    class const_iterator {
    private:
        std::pair<size_t, vciter>x;
        const HashMap<KeyType, ValueType, Hash>* ptr;
    public:
        const_iterator() : ptr(nullptr) {}
        const_iterator(const const_iterator &other) {
            x = other.x;
            ptr = other.ptr;
        }
        const_iterator(size_t st, vciter vci, const HashMap<KeyType, ValueType, Hash>* hm) : x(std::make_pair(st, vci)), ptr(hm) {} 
        const std::pair<const KeyType, ValueType>* operator->() const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&(*x.second));
        }
        const std::pair<const KeyType, ValueType>& operator*() const {
            return *operator->();
        }
        bool operator==(const const_iterator& other) const {
            return x == other.x;
        }
        bool operator!=(const const_iterator& other) const {
            return !(x == other.x);
        }

        const_iterator& operator++() {
            ++x.second;
            if (x.second == (*ptr).t[x.first].cend()) {
                ++x.first;
                while (x.first != (*ptr).sz && (*ptr).t[x.first].empty()) ++x.first;
                x.second = (*ptr).t[x.first].cbegin();
            }
            return *this;
        }

        const_iterator operator++(int) {
            auto old(*this);
            ++x.second;
            if (x.second == (*ptr).t[x.first].cend()) {
                ++x.first;
                while (x.first != (*ptr).sz && (*ptr).t[x.first].empty()) ++x.first;
                x.second = (*ptr).t[x.first].cbegin();
            }
            return old;
        }
    };
    HashMap(Hash x = Hash()) : hasher(x) {
        init();
    }
    HashMap(const HashMap& other, Hash x = Hash()) : hasher(x) {
        t = other.t;
        sz = other.sz;
        cnt = other.cnt;
        cap = other.cap;
    }
    HashMap(iterator seqbeg, iterator seqend, Hash x = Hash()) : hasher(x) {
        init();
        for (auto x = seqbeg; x != seqend; ++x) {
            insert(*x);
            realloc();
        }
    }
    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>&il, Hash x = Hash()) : hasher(x) {
        init();
        for (auto x = il.begin(); x != il.end(); ++x) {
            insert(*x);
            realloc();
        }
    }
    size_t size() const {
        return cnt;
    }
    bool empty() const {
        return (cnt == 0);
    }
    Hash hash_function() const {
        return hasher;
    }
    void insert(const std::pair<KeyType, ValueType> &kv) {
        size_t value = gethash(kv.first);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (auto &x : t[block]) {
            if (x.first == kv.first) {
                return;
            }
        }
        ++cnt;
        t[block].push_back(kv);
        realloc();
    }
    iterator end() {
        return iterator(sz, t[sz].begin(), this);
    }
    const_iterator end() const {
        return const_iterator(sz, t[sz].begin(), this);
    }
    iterator begin() {
        for (size_t i = 0; i < sz; ++i) {
            if (!t[i].empty()) {
                return iterator(i, t[i].begin(), this);
            }
        }
        return end();
    }
    const_iterator begin() const {
        for (size_t i = 0; i < sz; ++i) {
            if (!t[i].empty()) {
                return const_iterator(i, t[i].begin(), this);
            }
        }
        return end();
    }
    void erase(const KeyType& key) {
        size_t value = gethash(key);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (viter x = t[block].begin(); x != t[block].end(); ++x) {
            if (x->first == key) {
                --cnt;
                t[block].erase(x);
                realloc();
                return;
            }
        }
    }
    const_iterator find(const KeyType& key) const {
        size_t value = gethash(key);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (vciter x = t[block].begin(); x != t[block].end(); ++x) {
            if (x->first == key) return const_iterator(block, x, this);
        }
        return end();
    }
    iterator find(const KeyType& key) {
        size_t value = gethash(key);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (viter x = t[block].begin(); x != t[block].end(); ++x) {
            if (x->first == key) return iterator(block, x, this);
        }
        return end();
    }
    ValueType& operator[](const KeyType& key) {
        realloc();
        size_t value = gethash(key);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (viter x = t[block].begin(); x != t[block].end(); ++x) {
            if (x->first == key) return x->second;
        }
        ++cnt;
        t[block].push_back({key, ValueType()});
        return t[block].back().second;
    }
    const ValueType& at(const KeyType& key) const {
        size_t value = gethash(key);
        size_t block = value / (MOD / sz + (MOD % sz != 0));
        for (vciter x = t[block].begin(); x != t[block].end(); ++x) {
            if (x->first == key) return x->second;
        }
        throw std::out_of_range("ERROR");
    }
    void clear() {
        cnt = 0;
        init();
    }

    ~HashMap() {
        clear();
    }
};