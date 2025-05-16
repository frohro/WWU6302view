#include <Six302.h>

// This will produce compile-time diagnostics showing macro values
#if defined(S302_SERIAL)
#pragma message "S302_SERIAL is DEFINED"
#else
#pragma message "S302_SERIAL is NOT defined"
#endif

#if defined(S302_WEBSOCKETS)
#pragma message "S302_WEBSOCKETS is DEFINED"
#else
#pragma message "S302_WEBSOCKETS is NOT defined"
#endif

// Force an error to stop compilation after diagnostics
#error "Diagnostic build completed"