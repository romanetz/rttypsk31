#include "fractionaltypes.h"

#include "float.h"
#include "math.h"

//X coeffs: [n - 0] (0.000000, 0x0), [n - 1] (0.007846, 0x101), [n - 2] (-0.007253, 0xFF12), [n - 3] (0.007165, 0xEB), [n - 4] (0.000000, 0x0), 
//Y coeffs: [n - 0] (-0.691730, 0x588B), [n - 1] (1.000000, 0x8000), [n - 2] (-0.631693, 0x50DB), [n - 3] (0.313409, 0xD7E2), 

//X coeffs: [n - 0] (0.000000, 0x0), [n - 1] (0.009815, 0x142), [n - 2] (0.000000, 0x0), 
//Y coeffs: [n - 0] (1.000000, 0x8000), [n - 1] (-0.874318, 0x9016), [n - 2] (0.807649, 0x6761), 

F15 xd1, xd2, xd3,
	yd1, yd2, yd3, yd4;
	
F15 xc1, xc2, xc3,
	yc1, yc2, yc3, yc4;

void frontend_init() {
	xd1 = xd2 = xd3 = yd1 = yd2 = yd3 = yd4 = 0;
	
	xc1 = floatToF15(0.009815f);
	xc2 = floatToF15(0.0f);
	xc3 = floatToF15(0.0f);
	
	//yc1 = floatToF15(0.874318f);
	yc1 = floatToF15(0.915618f);
	//yc2 = floatToF15(-0.807649f);
	yc2 = floatToF15(-0.898693f);
	yc3 = floatToF15(0.0f);
	yc4 = floatToF15(0.0f);
}

F15 frontend_filter(F15 x) {
	F15 y = F15mul(xd1, xc1) + F15mul(xd2, xc2) + F15mul(xd3, xc3) +
			F15mul(yd1, yc1) + F15mul(yd2, yc2) + F15mul(yd3, yc3) + F15mul(yd4, yc4);
		
	xd3 = xd2;
	xd2 = xd1;
	xd1 = x;
	
	yd4 = yd3;
	yd3 = yd2;
	yd2 = yd1;
	yd1 = y;
}
