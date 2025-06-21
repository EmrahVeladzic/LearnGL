#ifndef FIXED_POINT_MATH
#define FIXED_POINT_MATH
#include<stdint.h>
typedef int32_t fix;
#define CONVERT_A_TO_B(ina,isa,isb)((ina*(1<<isb))/(1<<isa))
#define F_TO_FIX(in,is)((fix)(in*(float)(1<<is)))
#define FIX_TO_F(in,is)(((float)in/(float)(1<<is)))
#define ADD_A_B(ina,inb,isa,isb)(CONVERT_A_TO_B(ina,isa,isb)+inb)
#define SUB_A_B(ina,inb,isa,isb)(CONVERT_A_TO_B(ina,isa,isb)-inb)
#define MULT_A_B(ina,inb,isa,isb)((ina*inb)/(1<<isa))
#define DIV_A_B(ina,inb,isa,isb)((ina*(1<<isb))/inb)
#endif 

