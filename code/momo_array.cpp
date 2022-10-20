
// TODO(Momo): Remove Init()? Just use Create().
// Arrays can be 'invalid' by checking data == nullptr or count == 0
//
template<typename T>
inline auto&
Array<T>::operator[](u32 index) {
    Assert(index < count); 
    return data[index];
}

template<typename T>
static inline Array<T>
Array_Create(T* buffer, u32 count) {
    Array<T> ret = {};
    ret.data = buffer;
    ret.count = count; 
    return ret;
}


template<typename T>
static inline b8
Array_Init(Array<T>* a, T* buffer, u32 count) {
    if (!buffer || count == 0) {
        return false;
    }
    a->data = buffer;
    a->count = count;
    return true;
}

template<typename T>
static inline b8
Array_Alloc(Array<T>* a, Arena* arena, u32 count) {
    T* buffer = Arena_PushArray<T>(arena, count);
    return Array_Init(a, buffer, count);
}

template<typename T>
static inline T*
Array_Get(Array<T>* a, u32 index) {
    if(index < a->count) {
        return a->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T, typename Callback>
static inline T*
Array_Find(Array<T>* a, Callback callback) {
    for (u32 i = 0; i < a->count; ++i) {
        T* item = a->data + i;
        if (callback(item)) {
            return item;
        }
    }
    return nullptr;
}

template<typename T>
static inline T* 
operator+(Array<T> a, u32 index) {
    return Array_Get(&a, index);
}

