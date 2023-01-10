#ifndef SERIAL_TUNING_H
#define SERIAL_TUNING_H

#if !defined(SERIAL_TUNING_NO_PROFILE_HEADER) && defined(__has_include)
#if !__has_include("tuning_profile.h")
#define TUNING_NO_PROFILE_HEADER
#endif
#endif

#if !defined(SERIAL_TUNING_NO_PROFILE_HEADER)
#include "tuning_profile.h"
#endif

#include <Arduino.h>

#ifdef SERIAL_TUNING_USE_ETL_UNORDERED_MAP
#include <Embedded_Template_Library.h> // Mandatory for Arduino IDE.
#include <etl/unordered_map.h>
#else
#include <array>
#endif
#include <type_traits>

#ifndef SERIAL_TUNING_DEFAULT_MAX_ITEMS
#define SERIAL_TUNING_DEFAULT_MAX_ITEMS 32
#endif

// TODO: 64-bit ints don't (always) work with Arduino's String::toInt.

// Use an x-macro to avoid repetition/typos.
#define TYPE_MAP(X)     \
    X(INT8, int8_t)     \
    X(INT16, int16_t)   \
    X(INT32, int32_t)   \
    X(INT64, int64_t)   \
    X(UINT8, uint8_t)   \
    X(UINT16, uint16_t) \
    X(UINT32, uint32_t) \
    X(UINT64, uint64_t) \
    X(FLOAT, float)     \
    X(DOUBLE, double)   \
    X(STRING, String)

#define X_ENUM(E, T) E,
#define X_CONSTRUCTOR(E, T) \
    TuneItem(T& data) : type{E}, data{reinterpret_cast<void*>(&data)} {}
#define X_CAST(E, T)                                                                              \
    case E:                                                                                       \
        std::is_same<T, String>::value                                                            \
            ? (*reinterpret_cast<String*>(data) = value, 0)                                       \
            : (std::is_integral<T>::value ? (*reinterpret_cast<T*>(data) = value.toInt(), 0)      \
                                          : (*reinterpret_cast<T*>(data) = value.toDouble(), 0)); \
        break;

/**
 * Tagged union-like object containing a pointer storing a value to tune.
 */
class TuneItem
{
public:
    enum Type
    {
        TYPE_MAP(X_ENUM)
    };

    TuneItem() = default;
    TYPE_MAP(X_CONSTRUCTOR)

    void set(const String& value)
    {
        switch (type) {
            TYPE_MAP(X_CAST)
        }
    }

private:
    Type type;
    void* data = nullptr;
};

#ifdef SERIAL_TUNING_USE_ETL_UNORDERED_MAP
namespace etl
{
    template <>
    class hash<String>
    {
    public:
        size_t operator()(const String& s) const
        {
            // djb2
            size_t hash = 5381;
            for (char c : s)
                hash = ((hash << 5) + hash) + c;
            return hash;
        }
    };
} // namespace etl

namespace detail
{
    template <size_t MAX_ITEMS>
    class container
    {
    public:
        void insert(const String& name, const TuneItem& item)
        {
            if (m_items.size() == MAX_ITEMS)
                return;
            m_items[name] = item;
        }

        void set(const String& name, const String& value)
        {
            auto it = m_items.find(name);
            if (it != m_items.end())
                it->second.set(value);
        }

    private:
        etl::unordered_map<String, TuneItem, MAX_ITEMS> m_items;
    };
} // namespace detail

#else

namespace detail
{
    template <size_t MAX_ITEMS>
    class container
    {
    public:
        void insert(const String& name, const TuneItem& item)
        {
            if (m_size == MAX_ITEMS)
                return;
            m_names[m_size] = name;
            m_items[m_size] = item;
            m_size++;
        }

        void set(const String& name, const String& value)
        {
            for (size_t i = 0; i < m_size; i++) {
                if (m_names[i] == name) {
                    m_items[i].set(value);
                    break;
                }
            }
        }

    private:
        String m_names[MAX_ITEMS];
        TuneItem m_items[MAX_ITEMS];
        size_t m_size;
    };
} // namespace detail

#endif

namespace detail
{
}


template <size_t MAX_ITEMS = SERIAL_TUNING_DEFAULT_MAX_ITEMS>
class TuneSet
{
    detail::container<MAX_ITEMS> m_container;

public:
    template <typename T>
    void add(String name, T& data)
    {
        m_container.insert(name, TuneItem(data));
    }

    void readSerial()
    {
        while (Serial.available()) {
            String name = Serial.readStringUntil('=');
            String value = Serial.readStringUntil('\n');
            if (name == "" || value == "")
                continue;

            m_container.set(name, value);
        }
    }
};

#endif
