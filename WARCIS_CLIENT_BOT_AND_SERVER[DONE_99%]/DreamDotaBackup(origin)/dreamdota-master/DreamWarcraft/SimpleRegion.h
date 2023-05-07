#include "stdafx.h"
#ifndef SIMPLEREGION_H_INCLUDED
#define SIMPLEREGION_H_INCLUDED

#include "UIStructs.h"
#include "FunctionTemplate.h"
#include "Offsets.h"

namespace SimpleRegion {
	template <typename ThisSimpleRegionType>
	inline uint32_t setColor(ThisSimpleRegionType* t, uint32_t argb) {
		return aero::generic_this_call<uint32_t>(
			Offset(SIMPLEREGION_SETCOLOR), 
			t,
			&argb
		);
	}
	/*
	template <typename ThisSimpleRegionType>
	inline void set80(ThisSimpleRegionType* t, float v80) {
		aero::generic_this_call<void>(
			aero::offset_element_get<void*>(VTBL(t), 0x28), 
			t,
			v80
		);
	}
	*/
}

#endif