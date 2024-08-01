#pragma once

namespace Astranox
{
    /**
     * A reference-counted object.
     * This class is used to manage the lifetime of objects.
     */
    class RefCounted
    {
    public:
        RefCounted() = default;
        virtual ~RefCounted() = default;

        /**
         * Increment the reference count.
         */
        void addRef() { m_RefCount++; }

        /**
         * Decrement the reference count.
         * If the reference count reaches zero, the object is deleted.
         */
        void releaseRef()
        {
            m_RefCount--;
            if (m_RefCount == 0)
            {
                delete this;
            }
        }

        uint32_t getRefCount() const { return m_RefCount; }

    private:
        uint32_t m_RefCount = 0;
    };


    /**
     * A smart pointer for reference-counted objects.
     */
    template <typename T>
    class Ref final
    {
        template <typename U>
        friend class Ref;
    public:
        Ref() = default;

        Ref(T* ptr) : m_Ptr(ptr) {
            static_assert(std::derived_from<T, RefCounted>, "T must derive from RefCounted");
            if (m_Ptr)
            {
                m_Ptr->addRef();
            }
        }

        Ref(const Ref<T>& other) : m_Ptr(other.m_Ptr) {
            if (m_Ptr)
            {
                m_Ptr->addRef();
            }
        }

        Ref(Ref<T>&& other) noexcept : m_Ptr(other.m_Ptr) {
            other.m_Ptr = nullptr;
        }

        template <typename U>
        Ref(const Ref<U>& other) : m_Ptr((T*)other.m_Ptr) {
            if (m_Ptr)
            {
                m_Ptr->addRef();
            }
        }

        template <typename U>
        Ref(Ref<U>&& other) noexcept : m_Ptr((T*)other.m_Ptr) {
            other.m_Ptr = nullptr;
        }

        ~Ref() {
            if (m_Ptr)
            { 
                m_Ptr->releaseRef();
            }
        }

        Ref<T>& operator=(T* ptr)
        {
            if (m_Ptr == ptr)
            {
                return *this;
            }

            if (m_Ptr)
            {
                m_Ptr->releaseRef();
            }

            m_Ptr = ptr;
            if (m_Ptr)
            {
                m_Ptr->addRef();
            }

            return *this;
        }

        Ref<T>& operator=(const Ref<T>& other)
        {
            if (m_Ptr == other.m_Ptr)
            {
                return *this;
            }

            if (m_Ptr)
            {
                m_Ptr->releaseRef();
            }

            m_Ptr = other.m_Ptr;
            if (m_Ptr)
            {
                m_Ptr->addRef();
            }

            return *this;
        }

        Ref<T>& operator=(Ref<T>&& other) noexcept
        {
            if (m_Ptr == other.m_Ptr)
            {
                return *this;
            }

            if (m_Ptr)
            {
                m_Ptr->releaseRef();
            }

            m_Ptr = other.m_Ptr;
            other.m_Ptr = nullptr;

            return *this;
        }

        template <typename... Args>
        static Ref<T> create(Args&&... args)
        {
            return Ref<T>(new T(std::forward<Args>(args)...));
        }

        T* operator->() { return m_Ptr; }
        const T* operator->() const { return m_Ptr; }

        T& operator*() { return *m_Ptr; }
        const T& operator*() const { return *m_Ptr; }

        operator bool() const { return m_Ptr != nullptr; }

        T* raw() { return m_Ptr; }
        const T* raw() const { return m_Ptr; }

        template <typename U>
        Ref<U> as() const
        {
            return Ref<U>(static_cast<U*>(m_Ptr));
        }

    private:
        T* m_Ptr = nullptr;
    };
}
