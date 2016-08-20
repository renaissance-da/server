/*
 * element.h
 *
 *  Created on: 2012-12-29
 *      Author: per
 */

#ifndef ELEMENT_H_
#define ELEMENT_H_

enum Element
{
    None = 0,
    Fire = 1,
    Water = 2,
    Wind = 3,
    Earth = 4,
    Light = 5,
    Dark = 6,
    Wood = 7,
    Metal = 8,
    N_ELEMENTS = 9
};

const char *getEleName(Element e);
const double elemMod[12][4] = { { 0.36, 0.1234286, 0.096, 0.07854546 }, //60^2 		0
    { 0.49, 0.196, 0.1524444, 0.1247273 }, //70^2		1
    { 0.5625, 0.2410714, 0.1875, 0.1534091 }, //75^2	2
    { 0.64, 0.2925714, 0.2275556, 0.1861818 }, //80^2	3
    { 0.7225, 0.3509286, 0.2729444, 0.2233182 }, //85^2	4
    { 0.81, 0.4165714, 0.324, 0.2650909 }, //90^2		5
    { 0.9025, 0.4899286, 0.3810556, 0.3117727 }, //95^2	6
    { 1.0, 1.75, 2.25, 2.75 }, //100^2					7
    { 1.21, 2.32925, 2.99475, 3.66025 }, //110^2		8
    { 1.69, 3.84475, 4.94325, 6.04175 }, //130^2		9
    { 1.8225, 4.305656, 5.535844, 6.766031 }, //135^2	10
    { 2.25, 5.90625, 7.59375, 9.28125 } //150^2			11
};

const int elemModLev[N_ELEMENTS][N_ELEMENTS] = { //      No  F Wa  Wi   E  L  D  Wd  Me Un server won't use undead
    { 2, 0, 0, 0, 0, 0, 0, 0, 0 }, //Attack = none
        { 11, 2, 3, 9, 7, 6, 5, 5, 10 }, //Attack = fire
        { 11, 9, 2, 7, 3, 6, 5, 10, 5 }, //Attack = water
        { 11, 3, 7, 2, 9, 6, 5, 1, 5 }, //Attack = wind
        { 11, 7, 9, 3, 2, 6, 5, 5, 1 }, //Attack = earth
        { 11, 4, 4, 4, 4, 2, 9, 4, 4 }, //Attack = light
        { 11, 8, 8, 8, 8, 9, 2, 8, 8 }, //Attack = dark
        { 11, 1, 5, 5, 10, 6, 5, 2, 9 }, //Attack = wood
        { 11, 5, 1, 10, 5, 6, 5, 9, 2 } //attack = metal
    };

#define GET_ELEMENT_MOD(atk,def,fas) (elemMod[elemModLev[(atk)][(def)]][(fas)])

#endif /* ELEMENT_H_ */
