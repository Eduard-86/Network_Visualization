#pragma once

#include "HAL/Platform.h"

#if false
	#if USING_CODE_ANALYSIS == 1
		#define assumeChecked(expr) check(expr); CA_ASSUME(expr)
	#else
		#define assumeChecked(expr) UE_ASSUME(expr)
	#endif
#else
	#define assumeChecked(expr) check(expr)
#endif