/*
 * Stats.h
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#ifndef STATS_H_
#define STATS_H_

#include "element.h"

#define STATS_INVULNERABLE (const Stats *)1
#define CLIP(s,l,h) ((s) < (l) ? (l) : ((s) > (h) ? (h) : (s)))

class Stats
{
public:
    Stats();
    virtual ~Stats();

    Stats operator+(Stats const &rhs);
    Stats &operator+=(Stats const &rhs);
    friend Stats operator-(Stats const &stat);
    Stats &operator-=(Stats const &rhs);
    Stats operator-(Stats const &rhs);

    //Setters
    void setStr(short str)
    {
        strength = str;
    }
    void setDex(short dex)
    {
        dexterity = dex;
    }
    void setInt(short inte)
    {
        intelligence = inte;
    }
    void setCon(short con)
    {
        constitution = con;
    }
    void setWis(short wis)
    {
        wisdom = wis;
    }
    void setMaxHp(int hp)
    {
        maxHp = hp;
    }
    void setMaxMp(int mp)
    {
        maxMp = mp;
    }
    void setHit(short hit)
    {
        this->hit = hit;
    }
    void setDmg(short dmg)
    {
        this->dmg = dmg;
    }
    void setAc(short ac)
    {
        this->ac = ac;
    }
    void setMr(short mr)
    {
        this->mr = mr;
    }
    void setRegen(short regen)
    {
        this->regen = regen;
    }
    void setMinAtk(unsigned int amt)
    {
        atkMin = amt;
    }
    void setMaxAtk(unsigned int amt)
    {
        atkMax = amt;
    }
    void setMinAtkL(unsigned int amt)
    {
        atkMinL = amt;
    }
    void setMaxAtkL(unsigned int amt)
    {
        atkMaxL = amt;
    }

    //Getters
    int getHp() const
    {
        return CLIP(maxHp, 0, 999999);
    }
    int getMp() const
    {
        return CLIP(maxMp, 0, 999999);
    }
    short getStr() const
    {
        return CLIP(strength, 0, 255);
    }
    short getDex() const
    {
        return CLIP(dexterity, 0, 255);
    }
    short getCon() const
    {
        return CLIP(constitution, 0, 255);
    }
    short getInt() const
    {
        return CLIP(intelligence, 0, 255);
    }
    short getWis() const
    {
        return CLIP(wisdom, 0, 255);
    }
    short getHit() const
    {
        return CLIP(hit, 0, 100);
    }
    short getDmg() const
    {
        return CLIP(dmg, 0, 100);
    }
    short getAc() const
    {
        return ac;
    }
    short getMr() const
    {
        return CLIP(mr, 0, 7);
    }
    short getRegen() const
    {
        return regen;
    }
    int getAtkMin() const
    {
        return atkMin;
    }
    int getAtkMax() const
    {
        return atkMax;
    }
    int getAtkRange() const
    {
        return atkMax - atkMin;
    }
    int getAtkMinL() const
    {
        return atkMinL;
    }
    int getAtkMaxL() const
    {
        return atkMaxL;
    }
    int getAtkRangeL() const
    {
        return atkMaxL - atkMinL;
    }
    Element getAtkEle() const
    {
        return atkEle;
    }
    Element getDefEle() const
    {
        return defEle;
    }
    short getDodge() const
    {
        return dodge;
    }

    void incHp(int amt);
    void incMp(int amt);
    void incAc(int amt);
    void incStr(short amt)
    {
        strength += amt;
    }
    void incDex(short amt)
    {
        dexterity += amt;
    }
    void incCon(short amt)
    {
        constitution += amt;
    }
    void incWis(short amt)
    {
        wisdom += amt;
    }
    void incInt(short amt)
    {
        intelligence += amt;
    }
    void incHit(short amt)
    {
        hit += amt;
    }
    void incDmg(short amt)
    {
        dmg += amt;
    }
    void incMr(short amt)
    {
        mr += amt;
    }
    void incRegen(short amt)
    {
        regen += amt;
    }
    void incAtk(int amt)
    {
        atkMin += amt;
        atkMax += amt;
    }
    void setDodge(short amt)
    {
        dodge = amt;
    }

    //Applies the "Magic" modifier
    void magicMod()
    {
        maxMp = maxMp < 0 ? maxMp / 2 : maxMp + 50;
    }

    char getCappedAc();
    float getAcMod() const;

    void writeOut(int fd);
    void readFrom(int fd);

private:
    int maxHp, maxMp;
    short strength, intelligence, wisdom, dexterity, constitution;
    short hit, dmg, ac, mr, regen;
    int atkMin, atkMax, atkMinL, atkMaxL;
    Element atkEle, defEle;
    short dodge;
};

#endif /* STATS_H_ */
