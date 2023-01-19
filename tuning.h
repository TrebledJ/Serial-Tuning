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


// Use an x-macro to avoid repetition/typos.
#ifndef SERIAL_TUNING_TYPE_LIST
#define SERIAL_TUNING_TYPE_LIST(X) \
    X(int8_t)                      \
    X(int16_t)                     \
    X(int32_t)                     \
    X(int64_t)                     \
    X(uint8_t)                     \
    X(uint16_t)                    \
    X(uint32_t)                    \
    X(uint64_t)                    \
    X(float)                       \
    X(double)                      \
    X(String)
#endif


template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

#define ENUMIFY(T) TUNING_TYPE_##T

#define ENABLE_IF(COND) enable_if_t<COND, int> = 0


/**
 * Converts strings to various tuning types. You may inherit this class and
 * implement your own read functions, then pass it to TuneSet.
 */
class DefaultReader
{
public:
    template <typename T, ENABLE_IF(std::is_signed<T>::value&& std::is_integral<T>::value)>
    static T read(const String& value)
    {
        char* str_end;
        return strtoll(value.c_str(), &str_end, 0);
    }

    template <typename T, ENABLE_IF(std::is_unsigned<T>::value&& std::is_integral<T>::value)>
    static T read(const String& value)
    {
        char* str_end;
        return strtoull(value.c_str(), &str_end, 0);
    }

    template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
    static T read(const String& value)
    {
        char* str_end;
        return strtod(value.c_str(), &str_end);
    }

    template <typename T, ENABLE_IF((std::is_same<T, String>::value))>
    static String read(const String& value)
    {
        return value;
    }
};


/**
 * Converts data to Strings. You may inherit this class and implement your own
 * write functions, then pass it to TuneSet.
 */
class DefaultWriter
{
public:
    // Some versions of WString.h don't provide conversions for long long types.
    template <typename T, ENABLE_IF((std::is_same<T, uint64_t>::value))>
    static String write(T value)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%llu", value);
        return String(buffer);
    }

    template <typename T, ENABLE_IF((std::is_same<T, int64_t>::value))>
    static String write(T value)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%lld", value);
        return String(buffer);
    }

    template <typename T, ENABLE_IF(std::is_integral<T>::value && sizeof(T) != sizeof(uint64_t))>
    static String write(T value)
    {
        return String(value, 10);
    }

    template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
    static String write(T value)
    {
        return String(value, 6);
    }

    template <typename T, ENABLE_IF((std::is_same<T, String>::value))>
    static String write(const T& value)
    {
        return value;
    }
};


enum Type
{
#define X_ENUM(T) ENUMIFY(T),
    SERIAL_TUNING_TYPE_LIST(X_ENUM)
#undef X_ENUM
};


/**
 * Tagged union-like object containing a pointer storing a value to tune.
 */
class TuneItem
{
public:
    Type type;
    void* data = nullptr;

    TuneItem() = default;

#define X_CONSTRUCTOR(T) \
    TuneItem(T& data) : type{ENUMIFY(T)}, data{reinterpret_cast<void*>(&data)} {}

    SERIAL_TUNING_TYPE_LIST(X_CONSTRUCTOR)

#undef X_CONSTRUCTOR
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

        TuneItem& get(const String& name)
        {
            auto it = m_items.find(name);
            if (it != m_items.end())
                return &it->second;

            return nullptr;
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

        TuneItem* get(const String& name)
        {
            for (size_t i = 0; i < m_size; i++) {
                if (m_names[i] == name) {
                    return &m_items[i];
                }
            }
            return nullptr;
        }

    private:
        String m_names[MAX_ITEMS];
        TuneItem m_items[MAX_ITEMS];
        size_t m_size;
    };
} // namespace detail

#endif


template <size_t MAX_ITEMS = SERIAL_TUNING_DEFAULT_MAX_ITEMS, typename Reader = DefaultReader,
          typename Writer = DefaultWriter>
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
            name.trim();
            String value = Serial.readStringUntil('\n');
            if (!name.isEmpty()) {
                TuneItem* item = m_container.get(name);
                if (!item) {
#ifdef SERIAL_TUNING_WARN_NOT_FOUND
                    Serial.println("error: could not find variable '" + name + "'");
#endif
                } else {
                    if (!value.isEmpty()) {
                        set(*item, value);
                    } else {
                        Serial.printf("%s=%s\n", name, to_string(*item));
                    }
                }
            }
        }
    }

private:
    void set(TuneItem& item, const String& value)
    {
        switch (item.type) {
#define X_CASE(T) \
    case ENUMIFY(T): *reinterpret_cast<T*>(item.data) = Reader::template read<T>(value); break;

            SERIAL_TUNING_TYPE_LIST(X_CASE)

#undef X_CASE
        }
    }

    String to_string(TuneItem& item)
    {
        switch (item.type) {
#define X_CASE(T) \
    case ENUMIFY(T): return Writer::template write<T>(*reinterpret_cast<T*>(item.data));

            SERIAL_TUNING_TYPE_LIST(X_CASE)

#undef X_CASE
        }
    }
};


#undef ENABLE_IF
#undef ENUMIFY


// Helper macro for adding a tuning variable with the same label as the variable name.
#define TUNE(VAR) add(#VAR, VAR)


#endif
