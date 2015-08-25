/*
 * Exchange.h
 *
 *  Created on: 2013-04-09
 *      Author: per
 */

#ifndef EXCHANGE_H_
#define EXCHANGE_H_

#include <vector>
#include "Item.h"

class Character;

class Exchange
{
public:
    Exchange(Character *c1, Character *c2);
    ~Exchange();

    bool addItem(Item *itm, int perspective);
    bool addGold(int amt, int perspective);
    void cancel(int perspective);
    bool confirm(int perspective);

    void addedItem(Item *itm, int slot);

    int gold[2];
    bool confirms[2];
    Character *chars[2];
    int currentView;
};

class TradeView
{
public:
    TradeView(Exchange *ex, int perspective);

    void cancel()
    {
        ex->cancel(perspective);
    }
    bool addItem(Item *itm)
    {
        return ex->addItem(itm, perspective);
    }
    bool addItem(Item *itm, char amt)
    {
        return ex->addItem(itm, perspective);
    }
    bool addGold(int amt)
    {
        return ex->addGold(amt, perspective);
    }
    bool confirm()
    {
        return ex->confirm(perspective);
    }
    Character *getOther()
    {
        return ex->chars[1 - perspective];
    }
    void deleteExchange()
    {
        delete ex;
    }
    unsigned int getOfferedGold()
    {
        return ex->gold[1 - perspective];
    }

private:
    Exchange *ex;
    int perspective;
};

#endif /* EXCHANGE_H_ */
