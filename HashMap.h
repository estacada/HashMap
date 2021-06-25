#pragma once
#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

// Хэш-таблица на основе перестройки методом удвоения при росте, работает за O(1) (амортизированно).

template <class KeyType, class ValueType, class Hash = std::hash<KeyType> > 
class HashMap {
private:
    Hash hasher;
    size_t map_size = 1;
    size_t object_count = 0;
    size_t capacity = 1;
    std::vector<std::vector<std::pair<KeyType, ValueType>>> data;

    size_t GetHash(const KeyType &z) const {
        return hasher(z);
    }

    void init() {
        data.resize(2);
        data[0].clear();
        data[1].clear();
        data.shrink_to_fit();
        map_size = 1;
        capacity = 1;
    }

    void realloc() {
        if (object_count != capacity && (object_count == 0 || std::max(object_count, capacity) >= std::min(object_count, capacity) * 2)) {
            if (object_count == 0) {
                init();
                return;
            }
            map_size = capacity = object_count;
            std::vector<std::vector<std::pair<KeyType, ValueType>>> tmp;
            tmp.resize(map_size + 1);
            for (size_t i = 0; i < data.size(); i++) {
                for (const auto &val : data[i]) {
                    size_t value = GetHash(val.first);
                    size_t block = value % map_size;
                    tmp[block].push_back(val);
                }
            }
            data = std::move(tmp);
        }
    }


public:
    using VIter = typename std::vector<std::pair<KeyType, ValueType>>::iterator;
    using VConstIter = typename std::vector<std::pair<KeyType, ValueType>>::const_iterator;
    using KVPair = std::pair<const KeyType, ValueType>;

    // Итератор

    class iterator {
    private:
        std::pair<size_t, VIter> obj;
        HashMap<KeyType, ValueType, Hash>* ptr;

    public:
        iterator() : ptr(nullptr) {}

        iterator(const iterator& other) {
            obj = other.obj;
            ptr = other.ptr;
        }
        
        iterator(size_t st, VIter vi, HashMap<KeyType, ValueType, Hash>* hm) : obj(std::make_pair(st, vi)), ptr(hm) {}

        KVPair* operator->() const {
            return reinterpret_cast<KVPair*>(&(*obj.second));
        }

        KVPair& operator*() {
            return *operator->();
        }

        bool operator==(const iterator& other) const {
            return obj == other.obj;
        }

        bool operator!=(const iterator& other) const {
            return !(obj == other.obj);
        }

        iterator& operator++() {
            ++obj.second;
            if (obj.second == (*ptr).data[obj.first].end()) {
                ++obj.first;
                while (obj.first != (*ptr).map_size && (*ptr).data[obj.first].empty()) ++obj.first;
                obj.second = (*ptr).data[obj.first].begin();
            }
            return *this;
        }

        iterator operator++(int) {
            auto old(*this);
            ++obj.second;
            if (obj.second == (*ptr).data[obj.first].end()) {
                ++obj.first;
                while (obj.first != (*ptr).map_size && (*ptr).data[obj.first].empty()) ++obj.first;
                obj.second = (*ptr).data[obj.first].begin();
            }
            return old;
        }
    };

    // Константный итератор

    class const_iterator {
    private:
        std::pair<size_t, VConstIter> obj;
        const HashMap<KeyType, ValueType, Hash>* ptr;


    public:
        const_iterator() : ptr(nullptr) {}

        const_iterator(const const_iterator &other) {
            obj = other.obj;
            ptr = other.ptr;
        }

        const_iterator(size_t st, VConstIter vci, const HashMap<KeyType, ValueType, Hash>* hm) : obj(std::make_pair(st, vci)), ptr(hm) {} 

        const KVPair* operator->() const {
            return reinterpret_cast<const KVPair*>(&(*obj.second));
        }

        const KVPair& operator*() const {
            return *operator->();
        }

        bool operator==(const const_iterator& other) const {
            return obj == other.obj;
        }

        bool operator!=(const const_iterator& other) const {
            return !(obj == other.obj);
        }

        const_iterator& operator++() {
            ++obj.second;
            if (obj.second == (*ptr).data[obj.first].cend()) {
                ++obj.first;
                while (obj.first != (*ptr).map_size && (*ptr).data[obj.first].empty()) ++obj.first;
                obj.second = (*ptr).data[obj.first].cbegin();
            }
            return *this;
        }

        const_iterator operator++(int) {
            auto old(*this);
            ++obj.second;
            if (obj.second == (*ptr).data[obj.first].cend()) {
                ++obj.first;
                while (obj.first != (*ptr).map_size && (*ptr).data[obj.first].empty()) ++obj.first;
                obj.second = (*ptr).data[obj.first].cbegin();
            }
            return old;
        }
    };

    // Конструкторы

    HashMap(Hash hs = Hash()) : hasher(hs) {
        init();
    }

    HashMap(const HashMap& other, Hash hs = Hash()) : hasher(hs) {
        data = other.data;
        map_size = other.map_size;
        object_count = other.object_count;
        capacity = other.capacity;
    }

    HashMap(iterator seq_begin, iterator seq_end, Hash hs = Hash()) : hasher(hs) {
        init();
        for (auto it = seq_begin; it != seq_end; ++it) {
            insert(*it);
            realloc();
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>&il, Hash hs = Hash()) : hasher(hs) {
        init();
        for (auto it = il.begin(); it != il.end(); ++it) {
            insert(*it);
            realloc();
        }
    }

    // Размер хэш-таблицы (сложность O(1))

    size_t size() const {
        return object_count;
    }

    // Проверка на пустоту (сложность тоже O(1))

    bool empty() const {
        return (object_count == 0);
    }

    // Возвращает хэшер (сложность O(1))

    Hash hash_function() const {
        return hasher;
    }

    // Вставка в хэш-таблицу (сложность амортизированно O(1))

    void insert(const std::pair<KeyType, ValueType> &kv) {
        size_t value = GetHash(kv.first);
        size_t block = value % map_size;
        for (auto &val : data[block]) {
            if (val.first == kv.first) {
                return;
            }
        }
        ++object_count;
        data[block].push_back(kv);
        realloc();
    }

    // Итераторы begin, end

    iterator end() {
        return iterator(map_size, data[map_size].begin(), this);
    }

    const_iterator end() const {
        return const_iterator(map_size, data[map_size].begin(), this);
    }

    iterator begin() {
        for (size_t i = 0; i < map_size; ++i) {
            if (!data[i].empty()) {
                return iterator(i, data[i].begin(), this);
            }
        }
        return end();
    }

    const_iterator begin() const {
        for (size_t i = 0; i < map_size; ++i) {
            if (!data[i].empty()) {
                return const_iterator(i, data[i].begin(), this);
            }
        }
        return end();
    }

    // Удаление ключа из хэш-мапы, работает за длину блока с данным элементом (то есть, амортизированно за O(1), в худшем случае за O(n))

    void erase(const KeyType& key) {
        size_t value = GetHash(key);
        size_t block = value % map_size;
        for (VIter it = data[block].begin(); it != data[block].end(); ++it) {
            if (it->first == key) {
                --object_count;
                data[block].erase(it);
                realloc();
                return;
            }
        }
    }

    // Поиск итератора (константный и нет), асимптотика аналогична удалению ключа

    const_iterator find(const KeyType& key) const {
        size_t value = GetHash(key);
        size_t block = value % map_size;
        for (VConstIter it = data[block].begin(); it != data[block].end(); ++it) {
            if (it->first == key) return const_iterator(block, it, this);
        }
        return end();
    }
    
    iterator find(const KeyType& key) {
        size_t value = GetHash(key);
        size_t block = value % map_size;
        for (VIter it = data[block].begin(); it != data[block].end(); ++it) {
            if (it->first == key) return iterator(block, it, this);
        }
        return end();
    }

    // Проверка элемента на присутствие, если есть --- возвращает элемент, иначе --- создает элемент с таким ключом и нулевым значением и возвращает его

    ValueType& operator[](const KeyType& key) {
        realloc();
        size_t value = GetHash(key);
        size_t block = value % map_size;
        for (VIter it = data[block].begin(); it != data[block].end(); ++it) {
            if (it->first == key) return it->second;
        }
        ++object_count;
        data[block].push_back({key, ValueType()});
        return data[block].back().second;
    }

    // Проверка элемента на присутствие, если есть --- возвращает элемент, иначе --- кидает исключение

    const ValueType& at(const KeyType& key) const {
        size_t value = GetHash(key);
        size_t block = value % map_size;
        for (VConstIter it = data[block].begin(); it != data[block].end(); ++it) {
            if (it->first == key) return it->second;
        }
        throw std::out_of_range("ERROR");
    }

    // Очистка таблицы

    void clear() {
        object_count = 0;
        init();
    }

    // Деструктор

    ~HashMap() {
        clear();
    }
};
