#pragma once
#include "cyber_core.config.h"
#include "platform/configure.h"
#include <atomic>
#include <utility>

CYBER_BEGIN_NAMESPACE(Cyber)

template <typename T>
class RefCntWeakPtr;

template <typename T>
class RefCntAutoPtr;

/*
 * RefCntAutoPtr - 引用计数智能指针使用示例
 * 
 * 基本使用：
 *   RefCntAutoPtr<MyClass> ptr(new MyClass());
 *   RefCntAutoPtr<MyClass> ptr2 = make_ref_counted<MyClass>(args...);
 *   
 * 数组使用：
 *   auto arr = make_ref_counted_array<int>(100);
 *   auto arr2 = RefCntAutoPtr<int>::create_for_array(new int[50]);
 * 
 * 与C API交互（如DirectX COM接口）：
 *   方法1 - 使用get_address_of()：
 *     RefCntAutoPtr<ID3D12Resource> resource;
 *     device->CreateResource(..., resource.get_address_of());
 *   
 *   方法2 - 使用get_addressof()代理（更安全）：
 *     RefCntAutoPtr<ID3D12Resource> resource;
 *     device->CreateResource(..., resource.get_addressof());
 *   
 *   方法3 - 手动管理：
 *     ID3D12Resource* raw_ptr = nullptr;
 *     device->CreateResource(..., &raw_ptr);
 *     RefCntAutoPtr<ID3D12Resource> resource(raw_ptr);
 * 
 * 注意事项：
 * - 不要重载operator&()返回T**，这会破坏C++的语义
 * - get_address_of()会先释放现有对象，确保不会泄漏
 * - get_addressof()使用代理类，自动管理引用计数
 */

template <typename T>
class RefCountBase
{
public:
    RefCountBase() : m_strong_count(0), m_weak_count(0) {}
    virtual ~RefCountBase() = default;

    void add_strong_ref() CYBER_NOEXCEPT
    {
        m_strong_count.fetch_add(1, std::memory_order_relaxed);
    }

    void add_weak_ref() CYBER_NOEXCEPT
    {
        m_weak_count.fetch_add(1, std::memory_order_relaxed);
    }

    bool release_strong_ref() CYBER_NOEXCEPT
    {
        if (m_strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            destroy_object();
            return release_weak_ref();
        }
        return false;
    }

    bool release_weak_ref() CYBER_NOEXCEPT
    {
        if (m_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete this;
            return true;
        }
        return false;
    }

    uint32_t get_strong_count() const CYBER_NOEXCEPT
    {
        return m_strong_count.load(std::memory_order_relaxed);
    }

    uint32_t get_weak_count() const CYBER_NOEXCEPT
    {
        return m_weak_count.load(std::memory_order_relaxed);
    }

    bool try_add_strong_ref() CYBER_NOEXCEPT
    {
        uint32_t count = m_strong_count.load(std::memory_order_relaxed);
        while (count != 0)
        {
            if (m_strong_count.compare_exchange_weak(count, count + 1, std::memory_order_relaxed))
            {
                return true;
            }
        }
        return false;
    }

protected:
    virtual void destroy_object() = 0;

private:
    std::atomic<uint32_t> m_strong_count;
    std::atomic<uint32_t> m_weak_count;
};

template <typename T>
class RefCountObject : public RefCountBase<T>
{
public:
    RefCountObject(T* ptr) : m_ptr(ptr) {}

protected:
    virtual void destroy_object() override
    {
        delete m_ptr;
        m_ptr = nullptr;
    }

private:
    T* m_ptr;
};

template <typename T>
class RefCountArray : public RefCountBase<T>
{
public:
    RefCountArray(T* ptr) : m_ptr(ptr) {}

protected:
    virtual void destroy_object() override
    {
        delete[] m_ptr;
        m_ptr = nullptr;
    }

private:
    T* m_ptr;
};

template <typename T>
class RefCntAutoPtr
{
public:
    typedef T element_type;

    RefCntAutoPtr() CYBER_NOEXCEPT : m_ptr(nullptr), m_ref_count(nullptr) {}

    RefCntAutoPtr(std::nullptr_t) CYBER_NOEXCEPT : m_ptr(nullptr), m_ref_count(nullptr) {}

    explicit RefCntAutoPtr(T* ptr) : m_ptr(ptr), m_ref_count(nullptr)
    {
        if (m_ptr)
        {
            m_ref_count = new RefCountObject<T>(m_ptr);
            m_ref_count->add_strong_ref();
            m_ref_count->add_weak_ref();
        }
    }

    RefCntAutoPtr(const RefCntAutoPtr& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        if (m_ref_count)
        {
            m_ref_count->add_strong_ref();
        }
    }

    RefCntAutoPtr(RefCntAutoPtr&& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        other.m_ptr = nullptr;
        other.m_ref_count = nullptr;
    }

    template <typename U>
    RefCntAutoPtr(const RefCntAutoPtr<U>& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        if (m_ref_count)
        {
            m_ref_count->add_strong_ref();
        }
    }

    explicit RefCntAutoPtr(const RefCntWeakPtr<T>& weak_ptr);

    ~RefCntAutoPtr()
    {
        reset();
    }

    RefCntAutoPtr& operator=(const RefCntAutoPtr& other) CYBER_NOEXCEPT
    {
        *this = other.m_ptr;
        m_ref_count = other.m_ref_count;
        if (m_ref_count)
        {
            m_ref_count->add_strong_ref();
        }
        return *this;
    }

    RefCntAutoPtr& operator=(RefCntAutoPtr&& other) CYBER_NOEXCEPT
    {
        if (m_ptr != other.m_ptr)
        {
            reset();
            m_ptr = other.m_ptr;
            m_ref_count = other.m_ref_count;
            other.m_ptr = nullptr;
            other.m_ref_count = nullptr;
        }
        return *this;
    }

    template <typename U>
    RefCntAutoPtr& operator=(const RefCntAutoPtr<U>& other) CYBER_NOEXCEPT
    {
        reset();
        m_ptr = other.m_ptr;
        m_ref_count = other.m_ref_count;
        if (m_ref_count)
        {
            m_ref_count->add_strong_ref();
        }
        return *this;
    }
    RefCntAutoPtr& operator=(T* ptr)
    {
        if(m_ptr != ptr)
        {
            reset();
            if (ptr)
            {
                m_ptr = ptr;
            }
        }
        return *this;
    }

    RefCntAutoPtr& operator=(std::nullptr_t) CYBER_NOEXCEPT
    {
        reset();
        return *this;
    }

    void reset() CYBER_NOEXCEPT
    {
        if (m_ref_count)
        {
            m_ref_count->release_strong_ref();
            m_ptr = nullptr;
            m_ref_count = nullptr;
        }
    }

    void reset(T* ptr)
    {
        reset();
        if (ptr)
        {
            m_ptr = ptr;
            m_ref_count = new RefCountObject<T>(ptr);
            m_ref_count->add_strong_ref();
            m_ref_count->add_weak_ref();
        }
    }

    void swap(RefCntAutoPtr& other) CYBER_NOEXCEPT
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_ref_count, other.m_ref_count);
    }

    void attach(T* ptr) CYBER_NOEXCEPT
    {
        reset();
        m_ptr = ptr;
        if (m_ptr)
        {
            m_ref_count = new RefCountObject<T>(m_ptr);
            m_ref_count->add_strong_ref();
            m_ref_count->add_weak_ref();
        }
    }

    T* detach() CYBER_NOEXCEPT
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        if (m_ref_count)
        {
            m_ref_count->release_strong_ref();
            m_ref_count = nullptr;
        }
        return ptr;
    }

    T* get() const CYBER_NOEXCEPT { return m_ptr; }
    const T* get_const() const CYBER_NOEXCEPT { return m_ptr; }

    operator T*() const CYBER_NOEXCEPT { return m_ptr; }

    T& operator*() const CYBER_NOEXCEPT { return *m_ptr; }

    T* operator->() const CYBER_NOEXCEPT { return m_ptr; }

private:
    // 创建一个代理类来安全处理地址获取
    template <typename U>
    class AddressProxy
    {
    public:
        AddressProxy(RefCntAutoPtr& parent) : m_pAutoPtr(std::addressof(parent)), m_old_ptr(nullptr)
        {
            // 保存旧指针，以便检测是否有新值被赋予
            m_old_ptr = m_pAutoPtr->m_ptr;
        }
        
        AddressProxy(AddressProxy&& helper) CYBER_NOEXCEPT
            : m_pAutoPtr(helper.m_pAutoPtr), m_old_ptr(helper.m_old_ptr)
        {
            helper.m_pAutoPtr = nullptr;
            helper.m_old_ptr = nullptr;
        }

        ~AddressProxy()
        {
            // 检查是否有新指针被赋值
            if (m_pAutoPtr && m_pAutoPtr->m_ptr != m_old_ptr)
            {
                // 确保没有引用计数（意味着这是一个新的原始指针）
                if (!m_pAutoPtr->m_ref_count)
                {
                    // 为新指针创建引用计数
                    m_pAutoPtr->attach(m_old_ptr); // 释放旧指针
                    m_pAutoPtr->m_ref_count = new RefCountObject<T>(m_pAutoPtr->m_ptr);
                    m_pAutoPtr->m_ref_count->add_strong_ref();
                    m_pAutoPtr->m_ref_count->add_weak_ref();
                }
            }
        }
        
        U* operator*() CYBER_NOEXCEPT { return m_old_ptr; }
        const U* operator*() const CYBER_NOEXCEPT { return m_old_ptr; }

        // 隐式转换为T**，用于函数参数
        operator T**() CYBER_NOEXCEPT { return &m_old_ptr; }
        operator const T**() const CYBER_NOEXCEPT { return &m_old_ptr; }

    private:
        RefCntAutoPtr* m_pAutoPtr;
        U* m_old_ptr;

        // 禁用拷贝和移动
        AddressProxy(const AddressProxy&) = delete;
        AddressProxy& operator=(const AddressProxy&) = delete;
        AddressProxy& operator=(AddressProxy&&) = delete;
    };
    
public:
    // 使用代理类的安全地址获取
    template <typename U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    AddressProxy<U> get_addressof() CYBER_NOEXCEPT { return AddressProxy<U>(*this); }
    template <typename U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    AddressProxy<U> get_addressof() const CYBER_NOEXCEPT { return AddressProxy<U>(const_cast<RefCntAutoPtr&>(*this)); }

    AddressProxy<T> operator&()
    {
        return get_addressof<T>();
    }

    const AddressProxy<T> operator&() const
    {
        return get_addressof<T>();
    }

    // 简单的地址获取方法（会先释放现有对象）
    T** get_address_of() CYBER_NOEXCEPT 
    { 
        reset();
        return &m_ptr; 
    }

    explicit operator bool() const CYBER_NOEXCEPT { return m_ptr != nullptr; }

    bool operator!() const CYBER_NOEXCEPT { return m_ptr == nullptr; }

    uint32_t use_count() const CYBER_NOEXCEPT
    {
        return m_ref_count ? m_ref_count->get_strong_count() : 0;
    }

    bool unique() const CYBER_NOEXCEPT
    {
        return use_count() == 1;
    }

    static RefCntAutoPtr create_for_array(T* ptr)
    {
        RefCntAutoPtr result;
        if (ptr)
        {
            result.m_ptr = ptr;
            result.m_ref_count = new RefCountArray<T>(ptr);
            result.m_ref_count->add_strong_ref();
            result.m_ref_count->add_weak_ref();
        }
        return result;
    }

    template <typename U>
    friend class RefCntAutoPtr;

    template <typename U>
    friend class RefCntWeakPtr;

private:
    T* m_ptr;
    RefCountBase<T>* m_ref_count;
};

template <typename T>
class RefCntWeakPtr
{
public:
    typedef T element_type;

    RefCntWeakPtr() CYBER_NOEXCEPT : m_ptr(nullptr), m_ref_count(nullptr) {}

    RefCntWeakPtr(std::nullptr_t) CYBER_NOEXCEPT : m_ptr(nullptr), m_ref_count(nullptr) {}

    RefCntWeakPtr(const RefCntWeakPtr& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        if (m_ref_count)
        {
            m_ref_count->add_weak_ref();
        }
    }

    RefCntWeakPtr(RefCntWeakPtr&& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        other.m_ptr = nullptr;
        other.m_ref_count = nullptr;
    }

    template <typename U>
    RefCntWeakPtr(const RefCntWeakPtr<U>& other) CYBER_NOEXCEPT : m_ptr(other.m_ptr), m_ref_count(other.m_ref_count)
    {
        if (m_ref_count)
        {
            m_ref_count->add_weak_ref();
        }
    }

    RefCntWeakPtr(const RefCntAutoPtr<T>& shared_ptr) CYBER_NOEXCEPT : m_ptr(shared_ptr.m_ptr), m_ref_count(shared_ptr.m_ref_count)
    {
        if (m_ref_count)
        {
            m_ref_count->add_weak_ref();
        }
    }

    ~RefCntWeakPtr()
    {
        reset();
    }

    RefCntWeakPtr& operator=(const RefCntWeakPtr& other) CYBER_NOEXCEPT
    {
        if (this != &other)
        {
            reset();
            m_ptr = other.m_ptr;
            m_ref_count = other.m_ref_count;
            if (m_ref_count)
            {
                m_ref_count->add_weak_ref();
            }
        }
        return *this;
    }

    RefCntWeakPtr& operator=(RefCntWeakPtr&& other) CYBER_NOEXCEPT
    {
        if (this != &other)
        {
            reset();
            m_ptr = other.m_ptr;
            m_ref_count = other.m_ref_count;
            other.m_ptr = nullptr;
            other.m_ref_count = nullptr;
        }
        return *this;
    }

    template <typename U>
    RefCntWeakPtr& operator=(const RefCntWeakPtr<U>& other) CYBER_NOEXCEPT
    {
        reset();
        m_ptr = other.m_ptr;
        m_ref_count = other.m_ref_count;
        if (m_ref_count)
        {
            m_ref_count->add_weak_ref();
        }
        return *this;
    }

    RefCntWeakPtr& operator=(const RefCntAutoPtr<T>& shared_ptr) CYBER_NOEXCEPT
    {
        reset();
        m_ptr = shared_ptr.m_ptr;
        m_ref_count = shared_ptr.m_ref_count;
        if (m_ref_count)
        {
            m_ref_count->add_weak_ref();
        }
        return *this;
    }

    RefCntWeakPtr& operator=(std::nullptr_t) CYBER_NOEXCEPT
    {
        reset();
        return *this;
    }

    void reset() CYBER_NOEXCEPT
    {
        if (m_ref_count)
        {
            m_ref_count->release_weak_ref();
            m_ptr = nullptr;
            m_ref_count = nullptr;
        }
    }

    void swap(RefCntWeakPtr& other) CYBER_NOEXCEPT
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_ref_count, other.m_ref_count);
    }

    uint32_t use_count() const CYBER_NOEXCEPT
    {
        return m_ref_count ? m_ref_count->get_strong_count() : 0;
    }

    bool expired() const CYBER_NOEXCEPT
    {
        return use_count() == 0;
    }

    RefCntAutoPtr<T> lock() const CYBER_NOEXCEPT
    {
        if (m_ref_count && m_ref_count->try_add_strong_ref())
        {
            RefCntAutoPtr<T> result;
            result.m_ptr = m_ptr;
            result.m_ref_count = m_ref_count;
            return result;
        }
        return RefCntAutoPtr<T>();
    }

    template <typename U>
    friend class RefCntWeakPtr;

    template <typename U>
    friend class RefCntAutoPtr;

private:
    T* m_ptr;
    RefCountBase<T>* m_ref_count;
};

template <typename T>
RefCntAutoPtr<T>::RefCntAutoPtr(const RefCntWeakPtr<T>& weak_ptr) : m_ptr(nullptr), m_ref_count(nullptr)
{
    if (weak_ptr.m_ref_count && weak_ptr.m_ref_count->try_add_strong_ref())
    {
        m_ptr = weak_ptr.m_ptr;
        m_ref_count = weak_ptr.m_ref_count;
    }
}

template <typename T, typename U>
bool operator==(const RefCntAutoPtr<T>& lhs, const RefCntAutoPtr<U>& rhs) CYBER_NOEXCEPT
{
    return lhs.get() == rhs.get();
}

template <typename T, typename U>
bool operator!=(const RefCntAutoPtr<T>& lhs, const RefCntAutoPtr<U>& rhs) CYBER_NOEXCEPT
{
    return lhs.get() != rhs.get();
}

template <typename T>
bool operator==(const RefCntAutoPtr<T>& lhs, std::nullptr_t) CYBER_NOEXCEPT
{
    return lhs.get() == nullptr;
}

template <typename T>
bool operator==(std::nullptr_t, const RefCntAutoPtr<T>& rhs) CYBER_NOEXCEPT
{
    return rhs.get() == nullptr;
}

template <typename T>
bool operator!=(const RefCntAutoPtr<T>& lhs, std::nullptr_t) CYBER_NOEXCEPT
{
    return lhs.get() != nullptr;
}

template <typename T>
bool operator!=(std::nullptr_t, const RefCntAutoPtr<T>& rhs) CYBER_NOEXCEPT
{
    return rhs.get() != nullptr;
}

template <typename T, typename U>
bool operator==(const RefCntWeakPtr<T>& lhs, const RefCntWeakPtr<U>& rhs) CYBER_NOEXCEPT
{
    return lhs.m_ptr == rhs.m_ptr;
}

template <typename T, typename U>
bool operator!=(const RefCntWeakPtr<T>& lhs, const RefCntWeakPtr<U>& rhs) CYBER_NOEXCEPT
{
    return lhs.m_ptr != rhs.m_ptr;
}

template <typename T, typename... Args>
RefCntAutoPtr<T> make_ref_counted(Args&&... args)
{
    return RefCntAutoPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
RefCntAutoPtr<T> make_ref_counted_array(size_t size)
{
    return RefCntAutoPtr<T>::create_for_array(new T[size]);
}

CYBER_END_NAMESPACE