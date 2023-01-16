// This is a sample Serial Tuning Profile header. Rename this file to
// "tuning_profile.h", place it among your source files, and play around with
// the options below.

#ifndef SERIAL_TUNING_PROFILE_H
#define SERIAL_TUNING_PROFILE_H


// ----- Underlying Data Structure -----
// There are two underlying data structures to choose from:
//  * an etl::unordered_map, which provides efficient lookup, especially if you have a bazillion tuning values.
//      This option requires ETL (Embedded Template Library) to be installed.
//  * plain C arrays, which are more space efficient and don't rely on etlcpp.

// Uncomment the following line to use etl unordered map for tuning.
// #define SERIAL_TUNING_USE_ETL_UNORDERED_MAP


// ----- Default Max Items -----
// The maximum number of items to use by default. You may wish to set a smaller
// or larger value, depending on your needs.
#define SERIAL_TUNING_DEFAULT_MAX_ITEMS 32


// ----- Tuning Types -----
// You may define your own x-macro list of types to tune.
// You may need to define your own parser/converter if your type is not specialised in the DefaultParser.

// #define SERIAL_TUNING_TYPE_LIST(X) \
//     X(int32_t)       \
//     X(uint32_t)      \
//     X(float)         \
//     X(String)        \


// ----- Serial Output -----

// Uncomment the following line to print warnings to Serial when a variable name is not found.
// #define SERIAL_TUNING_WARN_NOT_FOUND


#endif
