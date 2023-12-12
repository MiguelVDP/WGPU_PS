#ifndef WGPU_PS_ENUMS_H
#define WGPU_PS_ENUMS_H


enum Integration
{
    Explicit = 0,
    Symplectic = 1,
    Implicit = 2,
};

enum SpringType{
    Stretch = 0,
    Bend = 1
};

#endif //WGPU_PS_ENUMS_H
