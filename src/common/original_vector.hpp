namespace {
    template <typename T, int INIT_CAPACITY = 10>
    struct OriginalVector {
        constexpr static int GROWTH_RATE = 3;
        static_assert(GROWTH_RATE > 1);
        T* data      = new T[INIT_CAPACITY];
        int size     = 0;
        int capacity = INIT_CAPACITY;
        static_assert(INIT_CAPACITY > 0);
        inline bool empty() const { return size == 0; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        inline void push_back(T x) {
            // if (data == nullptr) {
            //     data = new T[capacity];
            // }
            if (size == capacity) {
                capacity *= GROWTH_RATE;
                T* new_data = new T[capacity];
                for (int i = 0; i < size; i++) {
                    new_data[i] = data[i];
                }
                // delete[] data;
                data = new_data;
            }
            data[size++] = x;
        }
#pragma GCC diagnostic pop
        inline void pop_back() { size--; }
        inline void clear() {
            if (data == nullptr) {
                data = new T[capacity];
            }
            size = 0;
        }
        inline T& operator[](int i) { return data[i]; }
        inline const T& operator[](int i) const { return data[i]; }
        inline T* begin() { return data; }
        inline T* end() { return data + size; }
        inline const T* begin() const { return data; }
        inline const T* end() const { return data + size; }
    };
} // namespace
