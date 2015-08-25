/*
 * Secret.cpp
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#include "Secret.h"
#include "defines.h"
#include "SecretInfo.h"
#include "skills.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

Secret::Secret(unsigned int skillId, Path p, char level, int uses) :
    Skill(skillId, p, level, uses)
{
}

void Secret::getSecretInfo(IDataStream *dp, int weapon)
{
    const bool canTarget = ((SecretInfo *) getBase())->targets;

    dp->appendByte(getBase()->icon);
    dp->appendByte(canTarget ? 2 : 5);

    if (getMaxlevel() > 0) {
        char buffer[200];
        int len = snprintf(buffer, 200, "%s (Lev:%hu/%hu)", getBase()->name,
            getLevel(), getMaxlevel());
        dp->appendString(len, buffer);
    }
    else {
        dp->appendString(getBase()->nameLen, getBase()->name);
    }

    const char *desc = ((SecretInfo *) getBase())->targetDesc;
    dp->appendString(strlen(desc), desc);
    dp->appendByte(getCastTime((Staff) weapon));
}

bool Secret::isTargeted()
{
    return ((SecretInfo *) getBase())->targets;
}

unsigned char Secret::getCastTime(Staff staff)
{
    unsigned char retLines = ((SecretInfo *) getBase())->nlines;

    if (staff == ST_NONE) {
        return retLines; //no staff, just return base
    }

    SecretType secretType = STYPE_OTHER; //not an ioc or cradh spell

    //determine what type of spell it is - ioc, cradh, etc for staffs that are spelltype-specific
    switch (getId())
    {
    //case SK_BEAG_IOC_FEIN:
    case SK_BEAG_IOC:
    case SK_BEAG_IOC_COMLHA:
    case SK_IOC:
    case SK_IOC_COMLHA:
    case SK_MOR_IOC:
    case SK_MOR_IOC_COMLHA:
    case SK_ARD_IOC:
    case SK_NUADHAICH:

        secretType = STYPE_IOC;
        break;

    case SK_BEAG_CRADH:
    case SK_CRADH:
    case SK_MOR_CRADH:
    case SK_ARD_CRADH:

        secretType = STYPE_CRADH;
        break;

    default:

        secretType = STYPE_OTHER;
        break;
    }

    switch (staff) {
    case ST_HOLY_HERMES:

        if (secretType == STYPE_IOC) {
            return 0;
        }

        return retLines;

    case ST_HOLY_KRONOS: //2 line -> 1 line
    case ST_MAGUS_KRONOS:

        if (retLines == 2) {
            return 1;
        }

        return retLines;

    case ST_HOLY_ZEUS: //4 line -> 2 line
    case ST_MAGUS_ZEUS:

        if (retLines == 4) {
            return 2;
        }

        return retLines;

    case ST_HOLY_GAEA: //4 line -> 2 line
    case ST_MAGUS_GAEA:

        if (retLines > 4) {
            return 4;
        }

        return retLines;

    case ST_HOLY_DIANA: //-2 lines
    case ST_ASSASSINS_CROSS:
    case ST_VELTAIN_STAFF:
    case ST_ANDOR_STAFF:

        retLines = retLines > 2 ? retLines - 2 : 0;

        return retLines;

    case ST_HOLY_APOLLO:

        return 2;

    case ST_MAGUS_APOLLO:

        return 3;

    case ST_HOLY_CAIL:			//-1 line
    case ST_HOLY_CEANNLAIDIR:
    case ST_HOLY_DEOCH:
    case ST_HOLY_FIOSACHD:
    case ST_HOLY_GLIOCA:
    case ST_HOLY_GRAMAIL:
    case ST_HOLY_LUATHAS:
    case ST_HOLY_SGRIOS:
    case ST_MAGUS_DIANA:
    case ST_MAGUS_CAIL:
    case ST_MAGUS_CEANNLAIDIR:
    case ST_MAGUS_DEOCH:
    case ST_MAGUS_FIOSACHD:
    case ST_MAGUS_GLIOCA:
    case ST_MAGUS_GRAMAIL:
    case ST_MAGUS_LUATHAS:
    case ST_MAGUS_SGRIOS:

        retLines = retLines > 1 ? retLines - 1 : 0;

        return retLines;

    case ST_MAGUS_HY_BRASYL_GNARL: // >2 line -> 2 line
    case ST_MAGUS_HY_BRASYL_ORB:

        if (retLines > 2) {
            return 2;
        }

        return retLines;

    case ST_EMPOWERED_HY_BRASYL_GNARL: //ard cradh 0 line

        if (getId() == SK_ARD_CRADH) {
            return 0;
        }

        return retLines;

    case ST_ENCHANTED_HY_BRASYL_GNARL: //ard cradh 1 line

        if (getId() == SK_ARD_CRADH) {
            return 1;
        }

        return retLines;

    case ST_MASTER_DIVINE_STAFF:

        if (secretType == STYPE_CRADH) {
            return 1;
        }

        retLines = retLines > 3 ? retLines - 3 : 0;

        return retLines;

        //force all spells to 1 line. This is different than >1 line -> 1 line.
    case ST_STAFF_OF_BRILLIANCE:
    case ST_STAFF_OF_AGES:

        return 1;

    case ST_MASTER_CELESTIAL_STAFF:
    case ST_WOODEN_HARP:	//>1 line -> 1 line.
    case ST_GOLDBERRY_HARP:
    case ST_ROSEWOOD_HARP:
    case ST_IRONWOOD_HARP:
    case ST_HWARONE_LUTE:
    case ST_EMPOWERED_HWARONE_LUTE:
    case ST_HOLY_HY_BRASYL_BATON:
    case ST_BRUTES_QUILL:
    case ST_EMPOWERED_MAGUS_ORB:
    case ST_ENCHANTED_MAGUS_ORB:
    case ST_SPHERE:
    case ST_SHAINE_SPHERE:
    case ST_MARON_SPHERE:
    case ST_CHERNOL_SPHERE:
    case ST_SERPANT_SPHERE:
    case ST_EMPOWERED_SERPANT_SPHERE:
    case ST_GLIMMERING_WAND:
    case ST_YOWIEN_TREE_STAFF:

        if (retLines > 1) {
            return 1;
        }

        return retLines;

    case ST_MAGUS_ARES:

        if (secretType == STYPE_CRADH) {
            return 1;
        }

        return retLines;
        break;

    default:

        return retLines;
        break;
    }

    return retLines;
}
