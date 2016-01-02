//Breakout by Lucas Manning
//
//
#include "breakout.h"

//Checks if 2 rectangles are intersecting in the next frame
//Returns the side from which the intersection happens
int will_intersect(int x1, int xv1, int y1, int yv1, int w1, int h1,
        int x2, int xv2, int y2, int yv2, int w2, int h2)
{
    int dx1 = x1 + xv1, dy1 = y1 + yv1;
    int dx2 = x2 + xv2, dy2 = y2 + yv2;
    
    //First I check if the rectangles are intersecting, then extrapolate with
    //the given parameters to estimate where the ball was in the previous frame
    //to determine which side of the paddle collided with the ball.
    if ((dx1 + w1 > dx2 && dx1 < dx2 + w2) 
            && (dy1 + h1 > dy2 && dy1 < dy2 + h2)) {
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
        int xvel, int yvel) {
    game_obj tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.w = w;
    tmp.h = h;
    tmp.xvel = xvel;
    tmp.yvel = yvel;

    return tmp;
}

//This resolves collisions with a moving paddle, including moving
//x direction collisions.
void resolve_paddle_collision(game_obj *ball, game_obj *paddle)
{
    switch(will_intersect(ball->x, ball->xvel, ball->y, ball->yvel, 
                ball->w, ball->h, paddle->x, paddle->xvel, paddle->y,
                paddle->yvel, paddle->w, paddle->h)){
        
        case NO_COLLISION:
            break;
        
        case TOP_COLLISION:
            ball->y = paddle->y - ball->h;
            FLIP(ball->yvel);
            break;

        case BOTTOM_COLLISION:
            ball->y = paddle->y + paddle->h;
            FLIP(ball->yvel);
            break;

        case LEFT_COLLISION:
            ball->x = paddle->x - ball->w + paddle->xvel;
            ball->xvel = -ABS(ball->xvel);
            break;

        case RIGHT_COLLISION:
            ball->x = paddle->x + paddle->w + paddle->xvel;
            ball->xvel = ABS(ball->xvel);
            break;
    }
}

//Checks for collisions with bricks, resolves them and removes the brick.
void resolve_brick_collision(game_obj *ball, int *hits)
{
    for (int i = 0; i < BRICK_ROWS*BRICK_COLUMNS; ++i)
    {
        volatile obj_attr *brick_attr = &oam_mem[i];
        
        int brick_y = brick_attr->attr0 & ATTR0_Y_MASK;
        int brick_x = brick_attr->attr1 & ATTR1_X_MASK;
 
        //to remove bricks I just put them off screen instead of messing
        //with the hidden attribute.
        obj_set_pos(brick_attr, SCREEN_WIDTH, 0);

        switch(will_intersect(ball->x, ball->xvel, ball->y, ball->yvel,
                    ball->w, ball->h, brick_x, 0, brick_y, 
                    0, BRICK_WIDTH, BRICK_HEIGHT)) {

            case NO_COLLISION:
                obj_set_pos(brick_attr, brick_x, brick_y);
                break;
            
            case TOP_COLLISION:
                ball->y = brick_y - ball->h;
                FLIP(ball->yvel);
                break;

            case BOTTOM_COLLISION:
                ball->y = brick_y + BRICK_HEIGHT;
                FLIP(ball->yvel);
                break;

            case LEFT_COLLISION:
                ball->x = brick_x - ball->w;
                ball->xvel = -ABS(ball->xvel);
                break;

            case RIGHT_COLLISION:
                ball->x = brick_x + BRICK_WIDTH;
                ball->xvel = ABS(ball->xvel);
                break;
       }
    } 
}

void resolve_wall_collision(game_obj *ball)
{
    if (ball->x + ball->w + ball->xvel > SCREEN_WIDTH) {
        ball->x = SCREEN_WIDTH - ball->w;
        FLIP(ball->xvel);
    } else if (ball->x + ball->xvel < 0) {
        ball->x = 0;
        FLIP(ball->xvel);
    }

    if (ball->y + ball->yvel < 0) {
        ball->y = 0;
        FLIP(ball->yvel);
    } else if (ball->y + ball->yvel > SCREEN_HEIGHT) {
        //eventually replace this with death code
        ball->y = SCREEN_HEIGHT;
        FLIP(ball->yvel);
    }
}

int main()
{
    //This is the first color pallete at index zero. The palette has 16
    //colors total. Each color is 16 bits. we can have 32 palettes total. 
    obj_pal_mem[1] = rgb15(0x1F, 0, 0);         //red
    obj_pal_mem[2] = rgb15(0x1F, 0x10, 0);      //orange 
    obj_pal_mem[3] = rgb15(0x1F, 0x1F, 0);      //yellow
    obj_pal_mem[4] = rgb15(0, 0x1F, 0);         //green
    obj_pal_mem[5] = rgb15(0, 0, 0x1F);         //blue
    obj_pal_mem[6] = rgb15(0x1F, 0x1F, 0x1F);   //white

    //This setup code is explained when I set up the other sprites later
    for (int i = 0; i < BRICK_ROWS; ++i) {
        
        //we need to space these apart by 2 because there are 2 tiles in each
        //brick and if I just set them sequentially the object will read into
        //the wrong color tile.
        volatile u16* brick_tile_mem = (u16*)tile_mem[4][1+i*BRICK_TILES];

        //2 tiles in each brick. There's a bit of trickery to get the colors
        //formatted for 4bpp. These operators for index 1 will create 0x1111
        //The indicies correspond to the indicies of the colors in the 
        //palette memory.
        for (int k = 0; k < BRICK_TILES*(sizeof(tile4bpp) / sizeof(u16)); ++k) {
            brick_tile_mem[k] = ((i + 1) << 12) | ((i + 1) << 8) 
                    | ((i + 1) << 4) | i + 1;
        }

       
        for (int j = 0; j < BRICK_COLUMNS; ++j) {

            volatile obj_attr* brick_attr = &oam_mem[i*BRICK_COLUMNS + j];
            brick_attr->attr0 = ATTR0_SHAPE_WIDE;
            brick_attr->attr1 = ATTR1_SIZE_SMALL;
            //here since we have limited space for the tile indicies I just base
            //the tile color off the first one in the row becuase the mas is 64 
            //I think
            brick_attr->attr2 = i*BRICK_TILES + 1;

            //offset the bricks by 3 rows from the cieling
            obj_set_pos(brick_attr, j*BRICK_WIDTH , 
                    BRICK_HEIGHT*3 + i*BRICK_HEIGHT);
        }

    }

    //write sprite tiles into the fourth tile block. 
    //Blocks 0-3 are for background. We only need to make one tile here
    //because the paddle and the ball use the same exact tile. No need to create
    //extras. We just point to the tile in memory in attribute 2 where the base
    //is specified. It's offset by 1 because the zero tile is still background.
    volatile u16 *game_obj_tile_mem = 
        (u16*)tile_mem[4][BRICK_ROWS*BRICK_TILES + 1];
   
    //alright so here is a little trickery to avoid byte writes to vram,
    //which is a big no-no on the GBA. The tile is casted to a short (aka
    //2 byte, 16-bit) pointer. So paddle_tile}_mem is essentially an array of
    //shorts. Becuase its an array of shorts and not of bytes, we have to divide
    //the suze by 2 to get the correct amount of iterations. There are 4 tiles
    //to loop through (32x8p) so the whole thing is multiplied by 4
    //0x2222 converts to: 0b0010001000100010
    for (int i = 0; i < 4 * (sizeof(tile4bpp) / sizeof(u16)); ++i) {
        game_obj_tile_mem[i] = 0x6666; //bad luck...?
    }
   
    volatile obj_attr *paddle_attr = &oam_mem[BRICK_ROWS*BRICK_COLUMNS];

    paddle_attr->attr0 = ATTR0_SHAPE_WIDE;
    paddle_attr->attr1 = ATTR1_SIZE_MEDIUM;

    //this bitfield sets the id of the object and the palette bank.
    //The pallete bank we use is at index zero so no need to set it.
    //this sets the base tile for pallete to build the sprite from
    paddle_attr->attr2 = BRICK_ROWS*BRICK_TILES + 1;

    volatile obj_attr *ball_attr = &oam_mem[BRICK_ROWS*BRICK_COLUMNS+1];

    ball_attr->attr0 = ATTR0_SHAPE_SQUARE;
    ball_attr->attr1 = ATTR1_SIZE_SMALL; 
    ball_attr->attr2 = BRICK_ROWS*BRICK_TILES + 1;

    //This sets it in object and 1d mode for tiles
    REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D;

    const int paddle_vel = 3;

    game_obj paddle = create_game_obj(20, SCREEN_HEIGHT - PADDLE_HEIGHT, 
            PADDLE_WIDTH, PADDLE_HEIGHT, 0, 0);
    game_obj ball = create_game_obj(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
            BALL_WIDTH, BALL_HEIGHT, 1, 1);

    obj_set_pos(paddle_attr, paddle.x, paddle.y);
    obj_set_pos(ball_attr, ball.x, ball.y);

    int paused = FALSE;
    int lives = 5;
    int hits = 0;

    while (1)
    {
        //Note that this wastes a poop ton of battery. To fix this I _should_
        //throttle the CPU down when not in use and use system calls to bring 
        //it to life again when I need it. However, that requires some assembly,
        //which is a beast for another time.
        while(REG_VCOUNT >= 160);   // wait till VDraw
        while(REG_VCOUNT < 160);    // wait till VBlank
        
        key_poll();
        paddle.x += paddle.xvel;

        if (key_is_down(KEY_LEFT)) {
            paddle.xvel = paddle.x + paddle.xvel < 0 ?  0 : -paddle_vel;

        } else if (key_is_down(KEY_RIGHT)) {
            paddle.xvel = paddle.x + paddle.w + paddle.xvel > SCREEN_WIDTH ? 
                0 : paddle_vel; 
        } else {
            paddle.xvel = 0;
        }

        ball.x += ball.xvel;
        ball.y += ball.yvel;

        resolve_wall_collision(&ball);
        resolve_paddle_collision(&ball, &paddle); 
        resolve_brick_collision(&ball, &hits); 

        obj_set_pos(paddle_attr, paddle.x, paddle.y);
        obj_set_pos(ball_attr, ball.x, ball.y);
    }
    
    return 0;
}

