//Breakout.h by Lucas Manning
//
//A lot of general GBA programming stuff is gonna go in here
//as well as some "breakout" specific defines and function
//declarations
//
//

#ifndef BREAKOUT_H
#define BREAKOUT_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef u16 color;//15 bit color

typedef u32 tile4bpp[8]; //each tile 
typedef tile4bpp tile_block[512];

#define MEM_IO          0x04000000 //IO registers are here
#define MEM_PAL         0x05000000 //Color palette memory
#define MEM_VRAM        0x06000000 //screen VRAM starts here
#define MEM_OAM         0x07000000 //object/srpite mem here

#define DCNT_OBJ        0x1000
#define DCNT_OBJ_1D     0x0040

#define REG_DISPCNT     (*((volatile u32 *)(MEM_IO)))
#define REG_DISPSTAT    (*((volatile u32 *)(MEM_IO + 0x0004)))
#define REG_VCOUNT      (*((volatile u32 *)(MEM_IO + 0x0006)))
#define REG_KEYINPUT    (*((volatile u32 *)(MEM_IO + 0x0130)))

#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   160

#define ATTR0_MODE_MASK         0x0300
#define ATTR0_MODE_SHIFT        0x0008

#define ATTR0_MODE_HIDE         0x0200
#define ATTR0_MODE_REG          0x0000

#define ATTR0_SHAPE_MASK        0xC000
#define ATTR0_SHAPE_SHIFT           14

#define ATTR0_SHAPE_SQUARE      0x0000
#define ATTR0_SHAPE_WIDE        0x4000
#define ATTR1_SIZE_MEDIUM       0x4000
#define ATTR1_SIZE_SMALL        0x0000

#define ATTR0_Y_MASK            0x00FF 
#define ATTR1_X_MASK            0x01FF

typedef struct obj_attr
{
    u16 attr0, attr1, attr2;
    short fill;
} __attribute__((aligned(4))) obj_attr;

//This aligns the struct correctly for
//the GBA memory structure

#define oam_mem ((volatile obj_attr *)MEM_OAM)
#define tile_mem ((volatile tile_block *)MEM_VRAM)
#define obj_pal_mem ((volatile color *)(MEM_PAL + 0x200))

//globals for key states
u16 __key_curr, __key_prev;

#define KEY_RIGHT       0x0010
#define KEY_LEFT        0x0020
#define KEY_START       0x0008
#define KEY_MASK        0x03FF

static inline void key_poll()
{
    __key_prev = __key_curr;
    __key_curr = ~REG_KEYINPUT & KEY_MASK;
}

static inline u16 key_prev_state() { return __key_prev; }
static inline u16 key_curr_state() { return __key_curr; }
static inline u16 key_is_down(u32 key) { return __key_curr & key; } 
static inline u16 key_was_down(u32 key) { return __key_prev & key; } 
static inline u16 key_is_up(u32 key) { return ~__key_curr & key; } 
static inline u16 key_was_up(u32 key) { return ~__key_prev & key; } 
static inline u16 key_transit(u32 key) { return (__key_curr ^ __key_prev) & key; }

static inline void obj_hide(volatile obj_attr *obj)
{
    //first part resets bitfield and then ors it with appropriate bits
    obj->attr0 = (obj->attr0 & ~ATTR0_MODE_MASK) | ATTR0_MODE_HIDE;  
}

static inline void obj_show(volatile obj_attr *obj)
{
    obj->attr0 = (obj->attr0 & ~ATTR0_MODE_MASK) | ATTR0_MODE_REG;  
}

static inline void obj_set_pos(volatile obj_attr *obj, int x, int y)
{
    obj->attr0 = (obj->attr0 & ~ATTR0_Y_MASK) | (y & ATTR0_Y_MASK);
    obj->attr1 = (obj->attr1 & ~ATTR1_X_MASK) | (x & ATTR1_X_MASK);
}

static inline color rgb15(u32 red, u32 green, u32 blue)
{
    return red | (green<<5) | (blue<<10);
}

#endif //breakout_h
