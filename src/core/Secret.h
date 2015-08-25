/*
 * Secret.h
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#ifndef SECRET_H_
#define SECRET_H_

#include "Skill.h"
#include "IDataStream.h"
#include "SecretInfo.h"

enum Staff {
	ST_NONE =							0, 		//All Classes
	ST_STAFF_OF_BRILLIANCE =			0xbadde,
	ST_STAFF_OF_AGES =					0xbaddf,
	//Priest staffs
	ST_HOLY_HERMES =					4600,
	ST_HOLY_KRONOS =					4601,
	ST_HOLY_ZEUS =						4602,
	ST_HOLY_GAEA =						4603,
	ST_HOLY_DIANA =						4604,
	ST_HOLY_APOLLO =					4605,
	//Priest religious staffs
	ST_HOLY_CAIL =						4611,
	ST_HOLY_CEANNLAIDIR =				4608,
	ST_HOLY_DEOCH =						4606,
	ST_HOLY_FIOSACHD =					4612,
	ST_HOLY_GLIOCA =					4609,
	ST_HOLY_GRAMAIL =					4607,
	ST_HOLY_LUATHAS =					4610,
	ST_HOLY_SGRIOS =					4613,
	//Priest master staffs
	ST_MAGUS_HY_BRASYL_GNARL =			4104,
	ST_EMPOWERED_HY_BRASYL_GNARL =		4105,
	ST_ENCHANTED_HY_BRASYL_GNARL =		4106,
	ST_MASTER_DIVINE_STAFF =			0xbadd3,
	ST_MASTER_CELESTIAL_STAFF =			0xbadd4,
	//Bard weapons
	ST_WOODEN_HARP =					0xbadd5,
	ST_GOLDBERRY_HARP =					0xbadd6,
	ST_ROSEWOOD_HARP =					0xbadd7,
	ST_IRONWOOD_HARP =					0xbadd8,
	ST_HWARONE_LUTE =					0xbadd9,
	ST_EMPOWERED_HWARONE_LUTE =			0xbadda,
	ST_HOLY_HY_BRASYL_BATON =			0xbaddb,
	ST_BRUTES_QUILL =					0xbaddc,
	ST_ASSASSINS_CROSS =				0xbaddd,
	//Wizard staffs
	ST_MAGUS_ARES =						5600,
	ST_MAGUS_KRONOS =					5601,
	ST_MAGUS_ZEUS =						5602,
	ST_MAGUS_GAEA =						5603,
	ST_MAGUS_DIANA =					5604,
	ST_MAGUS_APOLLO =					5605,
	//Wizard religious staffs
	ST_MAGUS_CAIL =						5611,
	ST_MAGUS_CEANNLAIDIR =				5608,
	ST_MAGUS_DEOCH =					5606,
	ST_MAGUS_FIOSACHD =					5612,
	ST_MAGUS_GLIOCA =					5609,
	ST_MAGUS_GRAMAIL =					5607,
	ST_MAGUS_LUATHAS =					5610,
	ST_MAGUS_SGRIOS =					5613,
	//Wizard master staffs
	ST_MAGUS_HY_BRASYL_ORB =			5104,
	ST_EMPOWERED_MAGUS_ORB =			5105,
	ST_ENCHANTED_MAGUS_ORB =			5106,
	//Summoner weapons
	ST_SPHERE =							0xbade0,
	ST_SHAINE_SPHERE =					0xbade1,
	ST_MARON_SPHERE =					0xbade2,
	ST_CHERNOL_SPHERE =					0xbade3,
	ST_SERPANT_SPHERE =					0xbade4,
	ST_EMPOWERED_SERPANT_SPHERE =		0xbade5,
	ST_GLIMMERING_WAND =				0xbade6,
	ST_YOWIEN_TREE_STAFF =				0xbade7,
	ST_VELTAIN_STAFF =					0xbade8,
	ST_ANDOR_STAFF =					0xbade9

};

enum SecretType {
	STYPE_OTHER,
	STYPE_IOC,
	STYPE_CRADH
};


class Secret: public Skill {
public:
	Secret(unsigned int skillId, Path p = Peasant, char level = 0, int uses = 0);

	virtual ~Secret() {}

	void getSecretInfo(IDataStream *dp, int weapon);
	unsigned char getCastTime(Staff staff);
	bool isTargeted();
	bool isSecret() { return true; }

private:

};

#endif /* SECRET_H_ */
