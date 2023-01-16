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
#include <cstdlib>
#include <type_traits>

#ifndef SERIAL_TUNING_DEFAULT_MAX_ITEMS
#define SERIAL_TUNING_DEFAULT_MAX_ITEMS 32
#endif

// TODO: 64-bit ints don't (always) work with Arduino's String::toInt.

// Use an x-macro to avoid repetition/typos.
#define TYPE_MAP(X) \
    X(int8_t)       \
    X(int16_t)      \
    X(int32_t)      \
    X(int64_t)      \
    X(uint8_t)      \
    X(uint16_t)     \
    X(uint32_t)     \
    X(uint64_t)     \
    X(float)        \
    X(double)       \
    X(String)


template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

#define ENUMIFY(T) TYPE_##T

#define ENABLE_IF(COND) enable_if_t<COND, int> = 0


class converter
{
public:
    template <typename T, ENABLE_IF(std::is_signed<T>::value && std::is_integral<T>::value)>
    static T convert(const String& value)
    {
        char* str_end;
        return strtoll(value.c_str(), &str_end, 0);
    }

    template <typename T, ENABLE_IF(std::is_unsigned<T>::value && std::is_integral<T>::value)>
    static T convert(const String& value)
    {
        char* str_end;
        return strtoull(value.c_str(), &str_end, 0);
    }

    template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
    static T convert(const String& value)
    {
        char* str_end;
        return strtod(value.c_str(), &str_end);
    }

    template <typename T, ENABLE_IF((std::is_same<T, String>::value))>
    static String convert(const String& value)
    {
        return value;
    }

    // template <typename T, ENABLE_IF(!(std::is_arithmetic<T>::value || (std::is_same<T, String>::value)))>
    // static T convert(const String& value)
    // {
    //     static_assert("Unimplemented");
        // char* str_end;
        // return strtoull(value.c_str(), &str_end, 10);
    // }
};


/**
 * Tagged union-like object containing a pointer storing a value to tune.
 */
class TuneItem
{
public:
    enum Type
    {
#define X_ENUM(T) ENUMIFY(T),
        TYPE_MAP(X_ENUM)
#undef X_ENUM
    };

    TuneItem() = default;

#define X_CONSTRUCTOR(T) \
    TuneItem(T& data) : type{ENUMIFY(T)}, data{reinterpret_cast<void*>(&data)} {}

    TYPE_MAP(X_CONSTRUCTOR)

#undef X_CONSTRUCTOR

    void set(const String& value)
    {
        switch (type) {
#define X_CAST(T) \
    case ENUMIFY(T): *reinterpret_cast<T*>(data) = converter::convert<T>(value); break;

            TYPE_MAP(X_CAST)

#undef X_CAST
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


#undef ENABLE_IF

#endif
