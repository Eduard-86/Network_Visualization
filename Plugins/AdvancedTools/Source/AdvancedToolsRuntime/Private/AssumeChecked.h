#include "HAL/Platform.h"

#if UE_BUILD_DEBUG == 0
	#define assumeChecked(expr) UE_ASSUME(expr)
#elif USING_CODE_ANALYSIS == 1
	#define assumeChecked(expr) check(expr); CA_ASSUME(expr)
#else
	#define assumeChecked(expr) check(expr)
#endif