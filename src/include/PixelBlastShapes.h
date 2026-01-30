#pragma once

/* THE BIT (1 << 31) has Rotate state */

enum StaticShapes : int
{
    // --- SQUARE ---
    /*
        # # . . .
        # # . . .
        . . . . .
    */
    Shape0 = 0x0303 | (1 << 31),

    // --- LINE I ---
    /*
        # # # # .
        . . . . .
        . . . . .
    */
    Shape1 = 0x0F | (1 << 31),
    /*
        # . . . .
        # . . . .
        # . . . .
        # . . . .
    */
    Shape2 = 0x01010101 | (1 << 31),

    // --- ZETA (Z) ---
    /*
        . # # . .
        # # . . .
        . . . . .
    */
    Shape3 = 0x0603 | (1 << 31),
    /*
        # . . . .
        # # . . .
        . # . . .
    */
    Shape4 = 0x020301 | (1 << 31),

    // --- S-SHAPE ---
    /*
        # # . . .
        . # # . .
        . . . . .
    */
    Shape5 = 0x0306 | (1 << 31),
    /*
        . # . . .
        # # . . .
        # . . . .
    */
    Shape6 = 0x010302 | (1 << 31),

    // --- T-SHAPE ---
    /*
        . # . . .
        # # # . .
        . . . . .
    */
    Shape7 = 0x0207 | (1 << 31),
    /*
        # . . . .
        # # . . .
        # . . . .
    */
    Shape8 = 0x010301 | (1 << 31),
    /*
        # # # . .
        . # . . .
        . . . . .
    */
    Shape9 = 0x0702 | (1 << 31),
    /*
        . # . . .
        # # . . .
        . # . . .
    */
    Shape10 = 0x020302 | (1 << 31),

    // --- L-SHAPE ---
    /*
        # . . . .
        # # # . .
        . . . . .
    */
    Shape11 = 0x0107 | (1 << 31),
    /*
        # # . . .
        # . . . .
        # . . . .
    */
    Shape12 = 0x030101 | (1 << 31),
    /*
        # # # . .
        . . # . .
        . . . . .
    */
    Shape13 = 0x0704 | (1 << 31),
    /*
        . # . . .
        . # . . .
        # # . . .
    */
    Shape14 = 0x020203 | (1 << 31),

    // --- J-SHAPE ---
    /*
        . . # . .
        # # # . .
        . . . . .
    */
    Shape15 = 0x0407 | (1 << 31),
    /*
        # . . . .
        # . . . .
        # # . . .
    */
    Shape16 = 0x010103 | (1 << 31),
    /*
        # # # . .
        # . . . .
        . . . . .
    */
    Shape17 = 0x0701 | (1 << 31),
    /*
        # # . . .
        . # . . .
        . # . . .
    */
    Shape18 = 0x030202 | (1 << 31),
    /*
        # # # . .
        # # # . .
        # # # . .
    */
    Shape19 = 0x70707,
    /*
        # . . . .
        . . . . .
        . . . . .
    */
    Shape20 = 0x1,
    /*
        # # . . .
        . . . . .
        . . . . .
    */
    Shape21 = 0x3 | (1 << 31),
    /*
        # . . . .
        # . . . .
        . . . . .
    */
    Shape22 = 0x101 | (1 << 31),
    /*
        # # . . .
        # . . . .
        . . . . .
    */
    Shape23 = 0x103 | (1 << 31),
    /*
        # . . . .
        # # . . .
        . . . . .
    */
    Shape24 = 0x301 | (1 << 31)
};

constexpr int MaxShapes = 25;

int getShape(int idx)
{
    idx = qBound<int>(0, idx, MaxShapes - 1);
    switch(idx)
    {
        case 0:
            return StaticShapes::Shape0;
        case 1:
            return StaticShapes::Shape1;
        case 2:
            return StaticShapes::Shape2;
        case 3:
            return StaticShapes::Shape3;
        case 4:
            return StaticShapes::Shape4;
        case 5:
            return StaticShapes::Shape5;
        case 6:
            return StaticShapes::Shape6;
        case 7:
            return StaticShapes::Shape7;
        case 8:
            return StaticShapes::Shape8;
        case 9:
            return StaticShapes::Shape9;
        case 10:
            return StaticShapes::Shape10;
        case 11:
            return StaticShapes::Shape11;
        case 12:
            return StaticShapes::Shape12;
        case 13:
            return StaticShapes::Shape13;
        case 14:
            return StaticShapes::Shape14;
        case 15:
            return StaticShapes::Shape15;
        case 16:
            return StaticShapes::Shape16;
        case 17:
            return StaticShapes::Shape17;
        case 18:
            return StaticShapes::Shape18;
        case 19:
            return StaticShapes::Shape19;
        case 20:
            return StaticShapes::Shape20;
        case 21:
            return StaticShapes::Shape21;
        case 22:
            return StaticShapes::Shape22;
        case 23:
            return StaticShapes::Shape23;
        case 24:
            return StaticShapes::Shape24;
        default:
            return StaticShapes::Shape0;
    }
}

inline int randomShapes()
{
    return getShape(QRandomGenerator::global()->bounded(0, MaxShapes));
}
