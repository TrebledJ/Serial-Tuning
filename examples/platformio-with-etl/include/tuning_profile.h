#ifndef SERIAL_TUNING_PROFILE_H
#define SERIAL_TUNING_PROFILE_H


// ----- Underlying Data Structure -----
// There are two underlying data structures to choose from:
//  * an etl::unordered_map, which provides efficient lookup, especially if you have a bazillion tuning values.
//      This option requires ETL (Embedded Template Library) to be installed.
//  * plain C arrays, which are more space efficient and don't rely on etlcpp.

// Uncomment the following line to use etl unordered map for tuning.
#define SERIAL_TUNING_USE_ETL_UNORDERED_MAP


// ----- Default Max Items -----
// The maximum number of items to use by default. You may wish to set a smaller
// or larger value, depending on your needs.
#define SERIAL_TUNING_DEFAULT_MAX_ITEMS 32


#endif
