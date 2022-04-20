#ifndef CONTROLLERS_COLLISION_FILTERING_H_
#define CONTROLLERS_COLLISION_FILTERING_H_

const short CATEGORY_WALL = 0x0001;
const short CATEGORY_PLAYER = 0x0002;
const short CATEGORY_ENEMY = 0x0004;
const short CATEGORY_ENEMY_HITBOX = 0x0008;
const short CATEGORY_ENEMY_DAMAGE = 0x0010;
const short CATEGORY_PROJECTILE = 0x0020;
const short CATEGORY_SWORD = 0x0040;

const short MASK_WALL = 0xFFFF;  // (i.e. everthing)

const short MASK_PLAYER = CATEGORY_WALL | CATEGORY_ENEMY |
                          CATEGORY_ENEMY_DAMAGE | CATEGORY_SWORD |
                          CATEGORY_PROJECTILE;

const short MASK_PLAYER_DASHING = CATEGORY_WALL | CATEGORY_ENEMY_HITBOX;
const short MASK_PLAYER_ATTACKING = CATEGORY_WALL;

const short MASK_SWORD =
    CATEGORY_WALL | CATEGORY_ENEMY_HITBOX | CATEGORY_PLAYER;

const short MASK_PROJECTILE =
    CATEGORY_WALL | CATEGORY_ENEMY_HITBOX | CATEGORY_PLAYER;

const short MASK_ENEMY = CATEGORY_WALL | CATEGORY_PLAYER | CATEGORY_ENEMY;

const short MASK_ENEMY_HITBOX =
    CATEGORY_PLAYER | CATEGORY_SWORD | CATEGORY_PROJECTILE;

const short MASK_ENEMY_DAMAGE = CATEGORY_PLAYER;

#endif  // CONTROLLERS_COLLISION_FILTERING_H_
