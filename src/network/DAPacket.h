/*
 * DAPacket.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef DAPACKET_H_
#define DAPACKET_H_

#include "IDataStream.h"
#include "defines.h"

namespace Client
{
enum PacketCode
{
    START_ENCRYPT = 0x00,
    CREATE_CHARACTER = 0x02,
    LOGIN = 0x03,
    SET_ATTRIBUTES = 0x04,
    GET_MAP = 0x05,
    MOVE = 0x06,
    PICKUP = 0x07,
    DROP_ITEM = 0x08,
    LOGOUT = 0x0b,
//		THANKS_FOR_FISH = 	0x0c,		sent by client after it has had its position refreshed
    TALK = 0x0e,
    USE_SECRET = 0x0f,		//byte(slot) 0
    ATTACH_CHARACTER = 0x10,
    TURN = 0x11,
    ATTACK = 0x13,
    COUNTRY_LIST = 0x18,//reply is short(total) short(country) 8/1(non/master)x(1=war,5=monk,3=wiz,4=priest,5=rog)
                        //color(0xff white, 0x97 orange)
    WHISPER = 0x19,		//sends recip_name, message
    SETTINGS = 0x1B,
    USE_ITEM = 0x1c,
    EMOTE = 0x1d,
    DROP_GOLD = 0x24,
    CHANGE_PASSWORD = 0x26,
    GIVE_ITEM = 0x29,
    GIVE_GOLD = 0x2A,		//int(amt) int(target oid)
    GET_LEGEND_INFO = 0x2d,	//sent when client connects to character server, data ordinal 0
    GROUP_INVITE = 0x2E,	//02 string(name) invites, 03 string(name) accepts
    TOGGLE_GROUP = 0x2F,
    MOVE_PANE = 0x30,		//sent when the client moves something in a pane
    REFRESH = 0x38,		//f5
    RESUME_CONV = 0x39,
    RESUME_CONV_L = 0x3A,
    READ_BOARD = 0x3b,		//byte board#(i guess)
    USE_SKILL = 0x3e,		//only data is skill slot
    FIELD_WARP = 0x3f,
    CLICKED = 0x43,	//Server responds with 32 01(num door thingies) char x char y (char 0=open 1=closed) (char 0=right 1=left), fmt(03 short x short y  1 0)
    UNEQUIP_SLOT = 0x44,
    STAY_ALIVE = 0x45,
    INC_STAT = 0x47,		//sends stat to inc
    TRADE = 0x4a,
    QUERY_MOTD = 0x4B,
    START_CASTING = 0x4D,		//byte(lines)
    CHANT = 0x4E,		//string(chant)
    GET_EXTRA_INFO = 0x4f,//got 0 4 0 0 0 0 0 ?, think this might get info for an entity
    CONFIRM_ENCRYPT = 0x57,
    IGNORE_0 = 0x62,		//sent when the client connects to the server
    QUERY_HOMEPAGE = 0x68,
    UNK_0 = 0x75,
    PING = 0x7b	//data ordinal 1 0
};
}
namespace Server
{
enum PacketCode
{
    ENCRYPT_DATA = 0x00,
    SERVER_MESSAGE = 0x02,
    REDIRECT = 0x03,
    SET_COORD_INFO = 0x04,
    SET_OID = 0x05, //Think this sets the ID
    SHOW_MOBS = 0x07,
    CHAR_DATA = 0x08,
    SYSTEM_MESSAGE = 0x0A, //code 03 - message bar, code 05 - clear bar, code 08 - text box
    MOVED_SELF = 0x0B,
    ENT_MOVE = 0x0C,
    TALK = 0x0D,
    ENT_GONE = 0x0E,
    GET_ITEM = 0x0F,
    REMOVE_ITEM = 0x10,
    TURNED = 0x11,
    STRUCK = 0x13,
    AREA_DATA = 0x15,
    GET_SECRET = 0x17,
    ERASE_SECRET = 0x18,
    PLAY_SOUND = 0x19,
    ANIMATE = 0x1a,
    UNK_1 = 0x1E,
    UNK_11 = 0x1F,
    SET_LIGHTING = 0x20, //set lighting, values are between 0 and 11 with 0=midnight and 4-8=day
    PLAY_EFFECT = 0x29,
    SKILL = 0x2C,
    ERASE_SKILL = 0x2D,
    FIELD_WARP = 0x2E,
    NPC_SERVICE = 0x2F,
    NPC_DIALOG = 0x30,
    SHOW_BOARD = 0x31,
    DOOR_STATUS = 0x32,
    SHOW_CHAR = 0x33,
    PROFILE_INFO = 0x34,
    COUNTRY_LIST = 0x36,
    EQUIPPED_ITEM = 0x37,
    EQUIPMENT_GONE = 0x38,
    LEGEND_DATA = 0x39,
    STATUS_EFFECT = 0x3A,
    STAY_ALIVE = 0x3b,
    SEND_FILE = 0x3C,
    SET_CD = 0x3f,
    UPDATE_TRADE = 0x42,
    UNK_9 = 0x49,
    INIT_TRADE = 0x4b,
    END_SIGNAL = 0x4c,
    UPDATE_SERVERLIST = 0x56,
    UNK_7 = 0x58,
    GET_MOTD = 0x60,
    GROUP_INVITE = 0x63,
    SEND_HOMEPAGE = 0x66,
    LEFT_MAP = 0x67,
    UNK_12 = 0x68,
    METADATA = 0x6f,
    WELCOME = 0x7e
};
}

class DAPacket: public IDataStream
{
public:
    enum PacketError
    {
        UNKNOWN_SIZE, CONNECTION_CLOSED, SHORT_HEADER, BAD_HEADER, PAST_END
    };

    DAPacket(int fd);
    DAPacket(DAPacket *p);
    DAPacket(int code, char const *data, unsigned short dataLen);

    virtual ~DAPacket();

    unsigned short getCode();
    void writeData(int fd);
    char *getDataPtr();
    unsigned short getDataLen();
    void setKRoot(char hi, char med, char lo);
    char getOrd();

    void appendString(unsigned char len, char const *newData);
    void appendBytes(unsigned int len, char const *newData);
    void appendByte(char b);
    void appendInt(int x);
    void appendShort(short x);

    char *getCurPtr();
    std::string extractString();
    char extractByte();
    int extractInt();
    short extractShort();

    void skip(int x);
    void cyphered();

    char *getKeyRoot();

private:
    //its probably not a bad idea to have the key separate
    //so the next revision of dapacket should have data, pdata, dataLen, ord, code, keyroot
    //and data/pdata should? be a LL of char*, have to think about it
    //gives easy append, complicates sending, doesn't really matter for reading

    char raw[MAX_PACKET_LEN];

    char *data, *pData;
    unsigned short dataLength;
    unsigned short code;
    char *kRoot;
    char ordinal;
    bool wasCyphered;
};

#endif /* DAPACKET_H_ */
