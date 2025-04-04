#include "pch.h"

PROTOCOL::PFVector MakePFVector(float f1, float f2, float f3)
{
    PROTOCOL::PFVector pfV;
    pfV.set_x(f1);
    pfV.set_y(f2);
    pfV.set_z(f3);

    return pfV;
}
