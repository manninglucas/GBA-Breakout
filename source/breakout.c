//Breakout by Lucas Manning
//
//
#include "breakout.h"

//checks if 2 rectangles are intersecting in the nect frame
int will_intersect(int x1, int xv1, int y1, int yv1, int w1, int h1,
        int x2, int xv2, int y2, int yv2, int w2, int h2)
{
    int dx1 = x1 + xv1, dy1 = y1 + yv1;
    int dx2 = x2 + xv2, dy2 = y2 + yv2;
    
    //intersecting within the rectangle I check where it was before 
    //the collision to determine which side it hit.
    if ((dx1 + w1 > dx2 && dx1 < dx2 + w2) && (dy1 + h1 > dy2 && dy1 < dy2 + h2)) {
        if (y1 - yv1 + h1 < y2 - yv2) 
            return TOP_COLLISION; 
        else if (y1 - yv1 > y2 - yv2 + h2)
            return BOTTOM_COLLISION;
        else if (x1 - xv1 + w1 < x2 - xv2)
            return LEFT_COLLISION;
        else if (x1 - xv1 > x2 + w2 - xv2)
            return RIGHT_COLLISION;
    } 
    return NO_COLLISION;
}

game_obj create_game_obj(int x, int y, int w, int h, 
        int xvel, int yvel, int hid) {
    game_obj tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.w = w;
    tmp.h = h;
    tmp.xvel = xvel;
    tmp.yvel = yvel;
    tmp.hidden = hid;

    return tmp;
}


void resolve_ball_collision(game_obj *ball, game_obj *obj)
{
    switch(will_intersect(ball->x, ball->xvel, ball->y, ball->yvel, ball->w, ball->h, 
                obj->x, obj->xvel, obj->y, obj->yvel, obj->w, obj->h)){
        
        case NO_COLLISION:
            break;
        
        case TOP_COLLISION:
            ball->y = obj->y - ball->h;
            flip(&(ball->yvel));
            break;

        case BOTTOM_COLLISION:
            ball->y = obj->y + obj->h;
            flip(&(ball->yvel));
            break;

        case LEFT_COLLISION:
            ball->x = obj->x - ball->w + obj->xvel;
            //should always reflect velocity a certain way
            ball->xvel = ball->xvel > 0 ? -ball->xvel : ball->xvel;
            break;

        case RIGHT_COLLISION:
            ball->x = obj->x + obj->w + obj->xvel;
            //same as the other
            ball->xvel = ball->xvel < 0 ? -ball->xvel : ball->xvel;
            break;
    }
}

int main()
{
    //This is the first color pallete at index zero. The palette has 16
    //colors total. Each color is 16 bits. we can have 32 palettes total. 
    obj_pal_mem[1] = rgb15(0x1F, 0x1F, 0x1F);   //white
    obj_pal_mem[2] = rgb15(0x1F, 0, 0);         //red
    obj_pal_mem[5] = rgb15(0, 0x1F, 0);         //green
    obj_pal_mem[6] = rgb15(0, 0, 0x1F);         //blue
    obj_pal_mem[4] = rgb15(0x1F, 0x1F, 0);      //yellow
    obj_pal_mem[3] = rgb15(0x1F, 0x10, 0);      //orange 

    //write sprite tiles into the fourth tile block
    volatile u16 *paddle_tile_mem = (u16*)tile_mem[4][1];   
    volatile u16 *ball_tile_mem = (u16*)tile_mem[4][5];
   
    //alright so here is a little trickery to avoid byte writes to vram,
    //which is a big no-no on the GBA. The tile is casted to a short (aka
    //2 byte, 16-bit) pointer. So paddle_tile}_mem is essentially an array of shorts.
    //Becuase its an array of shorts and not of bytes, we have to divide the
    //suze by 2 to get the correct amount of iterations. There are 4 tiles
    //to loop through (32x8p) so the whole thing is multiplied by 4
    //0x2222 converts to: 0b0010001000100010
    for (int i = 0; i < 4 * (sizeof(tile4bpp) / 2); ++i) {
        paddle_tile_mem[i] = 0x1111;
    }
    
    //same thing as above except this one is only one tile (8x8p)
    for (int i = 0; i < sizeof(tile4bpp) / 2; ++i) {
        ball_tile_mem[i] = 0x1111;
    }

    volatile obj_attr *paddle_attr = &oam_mem[0];
    volatile obj_attr *ball_attr = &oam_mem[1];

    paddle_attr->attr0 = ATTR0_SHAPE_WIDE;
    paddle_attr->attr1 = ATTR1_SIZE_MEDIUM;

    //this bitfield sets the id of the object and the palette bank.
    //The pallete bank we use is at index zero so no need to se it.
    //this sets the base tile for pallete
    paddle_attr->attr2 = 1;

    ball_attr->attr0 = ATTR0_SHAPE_SQUARE;
    ball_attr->attr1 = ATTR1_SIZE_SMALL; 
    ball_attr->attr2 = 5;

    //five rows of bricks
    for (int i = 0; i < 5; ++i) {
        //15 bricks in each row
        for (int j = 0; j < 15; ++j) {
            volatile u16* brick_tile_mem = (u16*)tile_mem[4][6+i*15+j*2];

            //2 tiles in each brick
            for (int k = 0; k < 2 * (sizeof(tile4bpp) / 2); ++k) {
                brick_tile_mem[k] = ((i+2) << 12) | ((i+2) << 8) | ((i+2) << 4) | (i+2);  
            }

            volatile obj_attr* brick_attr = &oam_mem[2+i*15+j];
            brick_attr->attr0 = ATTR0_SHAPE_WIDE;
            brick_attr->attr1 = ATTR1_SIZE_SMALL;
            brick_attr->attr2 = 6+i*15;

            obj_set_pos(brick_attr, j*16 ,24+i*8);
        }

    }

    //This sets it in object and 1d mode
    REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D;

    const int paddle_vel = 3;

    game_obj paddle = create_game_obj(20, SCREEN_HEIGHT - 8, 32, 8, 0, 0, 0);
    game_obj ball = create_game_obj(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 8, 8, 1, 1, 0);

    obj_set_pos(paddle_attr, paddle.x, paddle.y);
    obj_set_pos(ball_attr, ball.x, ball.y);

    int paused = FALSE;
    int lives = 5;

    while (1)
    {
        //Note that this wastes a poop ton of battery. To fix this I _should_
        //throttle the CPU down when not in use and use system calls to bring 
        //it to life again when I need it. However, that requires some assembly,
        //which is a beast for another time.
        while(REG_VCOUNT >= 160);   // wait till VDraw
        while(REG_VCOUNT < 160);    // wait till VBlank
        
        key_poll();

        if (key_is_down(KEY_LEFT)) {
            paddle.xvel = paddle.x - paddle.xvel < 0 ?  0 : -paddle_vel;

        } else if (key_is_down(KEY_RIGHT)) {
            paddle.xvel = paddle.x + paddle.w + paddle.xvel > SCREEN_WIDTH ? 
                0 : paddle_vel; 
        } else {
            paddle.xvel = 0;
        }
        
        paddle.x += paddle.xvel;

        ball.x += ball.xvel;
        ball.y += ball.yvel;

        if (ball.x + ball.w > SCREEN_WIDTH) {
            ball.x = SCREEN_WIDTH - ball.w;
            flip(&(ball.xvel));
        } else if (ball.x < 0) {
            ball.x = 0;
            flip(&(ball.xvel));
        }

        if (ball.y < 0) {
            ball.y = 0;
            flip(&(ball.yvel));
        }

        //eventually replace this with death code
        if (ball.y > SCREEN_HEIGHT) {
            ball.y = SCREEN_HEIGHT;
            flip(&(ball.yvel));
        }

        resolve_ball_collision(&ball, &paddle); 

        obj_set_pos(paddle_attr, paddle.x, paddle.y);
        obj_set_pos(ball_attr, ball.x, ball.y);
    }
    
    return 0;
}

