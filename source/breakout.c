//Breakout by Lucas Manning
//
//
#include "breakout.h"

int main()
{
    int i;

    //write sprite tiles into the fourth tile block
    volatile u16* paddle_tile_mem = (u16*)tile_mem[4][1];   
    volatile u16* ball_tile_mem = (u16*)tile_mem[4][5];
   
    //alright so here is a little trickery to avoid byte writes to vram,
    //which is a big no-no on the GBA. The tile is casted to a short (aka
    //2 byte) pointer. So paddle_tile}_mem is essentially an array of shorts
    //Becuase its an array of shorts and not of bytes, we have to divide the
    //suze by 2 to get the correct amount of iterations. There are 4 tiles
    //to loop through (32x8p) so the whole thing is multiplied by 4
    for (i = 0; i < 4 * (sizeof(tile4bpp) / 2); ++i) {
        paddle_tile_mem[i] = 0x2222;
    }
    
    //same thing as above except this one is only one tile (8x8p)
    for (i = 0; i < sizeof(tile4bpp) / 2; ++i) {
        ball_tile_mem[i] = 0x2222;
    }

    //This is the first color pallete at index zero. The palette has 16
    //colors total. Each color is 16 bits. we can have 32 palettes total. 
    obj_pal_mem[1] = rgb15(0x1F, 0, 0);
    obj_pal_mem[2] = rgb15(0x1F, 0x1F, 0x1F);

    //This sets it in object and 1d mode
    REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D;

    volatile obj_attr* paddle_attr = &oam_mem[0];
    volatile obj_attr* ball_attr = &oam_mem[1];

    paddle_attr->attr0 = ATTR0_SHAPE_WIDE;
    paddle_attr->attr1 = ATTR1_SIZE_MEDIUM;

    //this bitfield sets the id of the object and the palette bank.
    //The pallete bank we use is at index zero so no need to se it.
    paddle_attr->attr2 = 1;

    ball_attr->attr0 = ATTR0_SHAPE_SQUARE;
    ball_attr->attr1 = ATTR1_SIZE_SMALL; 
    ball_attr->attr2 = 2;

    const int ball_w = 8, ball_h = 8, paddle_w = 32, paddle_h = 8;
    const int paddle_vel = 3, paddle_y = SCREEN_HEIGHT - paddle_h;

    int ball_x = SCREEN_WIDTH /2, ball_y = SCREEN_HEIGHT /2, ball_x_vel = 20, ball_y_vel = 20;
    int paddle_x = 20;

    obj_set_pos(paddle_attr, paddle_x, paddle_y);
    obj_set_pos(ball_attr, ball_x, ball_y);

    while (1)
    {
        //Note that this wastes a poop ton of battery. To fix this I _should_ throttle
        //the CPU down when not in use and use system calls to bring it to life again
        //when I need it. However, that requires some assembly, which is a beast for another
        //time.
        while(REG_VCOUNT >= 160);   // wait till VDraw
        while(REG_VCOUNT < 160);    // wait till VBlank
        
        key_poll();

        if (key_is_down(KEY_LEFT))
            paddle_x -= paddle_vel;
        else if (key_is_down(KEY_RIGHT))
            paddle_x += paddle_vel;

        obj_set_pos(paddle_attr, paddle_x, paddle_y);

    }
    
    return 0;
}

