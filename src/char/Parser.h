/*
 * Parser.h
 *
 *  Created on: 2012-12-05
 *      Author: per
 */

#ifndef PARSER_H_
#define PARSER_H_

enum ChatCmdCode
{
    SET_BGM,
    SET_APPEARANCE,
    GOTO_MAP,
    SPAWN_MOB,
    PLAY_EFFECT,
    INVAL,
    ITM,
    PVP,
    RELOAD,
    INVITE,
    CHANGE_PW,
    IP_BAN,
    CHAR_BAN,
    GOTO_PLAYER,
    RECALL_PLAYER,
    NONE
};

struct ChatCommand
{
    ChatCmdCode cmd;
    union
    {
        int p1;
        const char *c1;
    };
    union
    {
        int p2;
        const char *c2;
    };
    int p3, p4;
};

//Checks if the text given is a command string
ChatCommand tryGetCommand(const char *src);

#endif /* PARSER_H_ */
