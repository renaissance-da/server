/*
 * defines.h
 *
 *  Created on: 2012-04-10
 *      Author: per
 */

#ifndef DEFINES_H_
#define DEFINES_H_

//Server control constants

//Direction constants
#define UP 0
#define DOWN 2
#define RIGHT 1
#define LEFT 3
#define DIR_MAX 3
//Maximum view distance
#define NEARBY 12

//Move flags
#define MOVE_NO_PORTALS 1
#define MOVE_SOLID 2
#define MOVE_NO_DOORS 4

#define CLIENT_VERSION_MIN 736
#define MAX_PACKET_LEN 16384

//Starting map
#define START_MAP 136
#define START_X 6
#define START_Y 8

//Map allowing field warps
#define GATE 1001

//character constants
#define NUM_ITEMS 59
#define NUM_EQUIPS 19
#define NUM_STORAGE 120
#define WEAPON_SLOT 1
#define ARMOR_SLOT 2
#define MAX_NAME_LEN 12
#define ITEM_LIMIT 10 //max number of times a player can use an item in a second
#define SECRET_LIMIT 3 //max number of times a player can use a secret in a second
#define LABOR_RESET_INTERVAL 43200 //43200 = 12 hours
#define MAX_LABOR 24
#define MAX_EXCHANGE 10 // maximum number of items exchanged at once
#define MAX_GOLD 1000000000 // max amount of gold in bank or on character

//ticks per second. Setting too high may result in server lag, while too low may cause rapid hits to occur more
//frequently
// NOTE Do not change this, there are no guarantees things will work with a different setting
#define TICKS 2

enum SessionError
{
	E_LOGGEDIN = 1,
	E_NOEXIST = 2,
	E_WRONGPW = 3,
	E_NOKEYTABLE = 4,
	E_INVALID = 5,
	E_UNSPECIFIED = 6
};

#endif /* DEFINES_H_ */
