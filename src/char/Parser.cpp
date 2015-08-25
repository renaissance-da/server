/*
 * Parser.cpp
 *
 *  Created on: 2012-12-05
 *      Author: per
 */

#include "Parser.h"
#include <string.h>
#include <ctype.h>

ChatCommand tryGetCommand(const char *src)
{
    ChatCommand c;
    c.cmd = NONE;

    //Otherwise,
    //TODO Improve if there is ever a good reason to, this is just an experiment at the moment
    if (src[0] && src[1] && src[2] && src[3]) {
        if (!strncmp("\\mus", src, 4)) {
            //mus command, 1 arg
            unsigned int r = 0;
            for (int i = 5; isdigit(src[i]); i++) {
                r = r * 10 + src[i] - '0';
            }
            if (!r || r > 64) {
                c.cmd = INVAL;
            }
            else {
                c.cmd = SET_BGM;
                c.p1 = r;
            }
        }

        else if (!strncmp("\\map", src, 4)) {
            unsigned int r = 0;
            int i;
            for (i = 5; isdigit(src[i]); i++) {
                r = (r * 10) + src[i] - '0';
            }
            c.cmd = GOTO_MAP;
            c.p1 = r;
            r = 0;
            for (i = i + 1; isdigit(src[i]); i++) {
                r = (r * 10) + src[i] - '0';
            }
            c.p2 = r;
            r = 0;
            for (i = i + 1; isdigit(src[i]); i++) {
                r = (r * 10) + src[i] - '0';
            }
            c.p3 = r;
        }

        else if (!strncmp("\\mob", src, 4)) {
            unsigned int r = 0;
            int i;
            for (i = 5; isdigit(src[i]); i++) {
                r = (r * 10) + src[i] - '0';
            }
            c.cmd = SPAWN_MOB;
            c.p1 = r;
        }

        else if (!strncmp("\\eff", src, 4)) {
            unsigned int r = 0;
            int i;
            for (i = 5; isxdigit(src[i]); i++) {
                r = (r << 4)
                    + (isdigit(src[i]) ?
                        src[i] - '0' : tolower(src[i]) - 'a' + 10);
            }
            c.cmd = PLAY_EFFECT;
            c.p1 = r;
        }

        else if (!strncmp("\\itm", src, 4)) {
            unsigned int r = 0;
            int i;
            for (i = 5; isdigit(src[i]); i++) {
                r = (r * 10) + src[i] - '0';
            }
            c.cmd = ITM;
            c.p1 = r;
        }

        else if (!strncmp("\\pvp", src, 4)) {
            c.cmd = PVP;
        }

        else if (!strncmp("\\rld", src, 4)) {
            c.cmd = RELOAD;
        }

        else if (!strncmp("\\inv", src, 4)) {
            c.cmd = INVITE;
            c.c1 = src + 4;
            while (*c.c1 == ' ')
                c.c1++;
            if (*c.c1 == '\0')
                c.c1 = 0;
        }

        else if (!strncmp("\\chp", src, 4)) {
            c.cmd = CHANGE_PW;
            c.c1 = src + 4;
            c.p3 = 0;
            while (*c.c1 == ' ') {
                c.c1++;
            }
            if (*c.c1 == '\0') {
                c.c1 = 0;
                c.c2 = 0;
            }
            else {
                ++c.p3;
                c.c2 = c.c1;
                while (*(++c.c2) != ' ')
                    c.p3++;
                while (*(++c.c2) == ' ')
                    ;
                if (*c.c2 == '\0')
                    c.c2 = 0;
            }
        }

        else if (!strncmp("\\ipb", src, 4)) {
            c.cmd = IP_BAN;
            c.c1 = src + 4;
            while (*c.c1 == ' ')
                c.c1++;
        }

        else if (!strncmp("\\chb", src, 4)) {
            c.cmd = CHAR_BAN;
            c.c1 = src + 4;
            while (*c.c1 == ' ')
                c.c1++;
        }
        else if (!strncmp("\\g2p", src, 4)) {
            c.cmd = GOTO_PLAYER;
            c.c1 = src + 4;
            while (*c.c1 == ' ')
                c.c1++;
        }
        else if (!strncmp("\\rec", src, 4)) {
            c.cmd = RECALL_PLAYER;
            c.c1 = src + 4;
            while (*c.c1 == ' ')
                c.c1++;
        }
    }

    return c;

}
