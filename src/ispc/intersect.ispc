//
// Example ispc file to test compiler
//

#include "src/intersect.h"
extern "C" {
unmasked void fooCpp(float *uniform v  /* the data */,
                     int *uniform active /* explicit active mask */,
                     int uniform N);
}

void fooIspc(varying float f)
{
    /* the same stuff we did above: */
    uniform float fArray[programCount];
//    fArray[programCount] = f;

    /* now "construct" an active mask - mind the 'unmasked'!!!! */
    uniform int activeMask[programCount];
    unmasked { activeMask[programIndex] = 0; }
    activeMask[programIndex] = 1;

//    fooCpp(.....);
}