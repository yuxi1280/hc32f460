#include "hot.h"
#include "io.h"
#include "system_hc32f460.h"
void temperature_into(void)
{
    CM_PWC->FCG3 &= ~(1 << 5); 
}


