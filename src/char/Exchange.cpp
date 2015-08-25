/*
 * Exchange.cpp
 *
 *  Created on: 2013-04-09
 *      Author: per
 */

#include "Exchange.h"
#include "defines.h"
#include "srv_proto.h"
#include <assert.h>
#include <algorithm>

TradeView::TradeView(Exchange *ex, int perspective) :
    ex(ex), perspective(perspective)
{
}

Exchange::Exchange(Character *c1, Character *c2)
{
    currentView = 0;

    gold[0] = 0;
    gold[1] = 0;

    confirms[0] = false;
    confirms[1] = false;

    chars[0] = c1;
    chars[1] = c2;

}

Exchange::~Exchange()
{
}

bool Exchange::addItem(Item *itm, int perspective)
{
    int recip = 1 - perspective;
    //Test weights
    if (chars[recip]->getFreeWeight()
        - (int) (itm->getWeight()
            + chars[perspective]->getExchangeItems()->getWeight()) < 0) {
        Server::sendMessage(chars[perspective]->getSession(),
            "They can't carry any more.");
        return false;
    }
    //Test free slots
    if (chars[recip]->countFreeSlots()
        - (int) (chars[perspective]->getExchangeItems()->getFullSlots() + 1)
        < 0) {
        Server::sendMessage(chars[perspective]->getSession(),
            "They can't have any more.");
        return false;
    }
    //Test capacity
    if (itm->getMaxQty()
        && chars[recip]->countItems(itm->getId(), itm->getMod())
            + chars[perspective]->getExchangeItems()->countItems(itm->getId(),
                itm->getMod()) + itm->getQty() > itm->getMaxQty()) {
        Server::sendMessage(chars[perspective]->getSession(),
            "They can't have any more.");
        return false;
    }

    currentView = perspective;
    return chars[perspective]->getExchangeItems()->addItem(itm, this);
}

void Exchange::addedItem(Item *itm, int slot)
{
    Server::exchangeAddItem(chars[currentView]->getSession(), true, itm,
        slot + 1);
    Server::exchangeAddItem(chars[1 - currentView]->getSession(), false, itm,
        slot + 1);
}

bool Exchange::addGold(int amt, int perspective)
{
    int recip = 1 - perspective;
    if (amt + gold[recip] + chars[recip]->getGold() > MAX_GOLD) {
        Server::sendMessage(chars[perspective]->getSession(),
            "They can't have any more.");
        return false;
    }
    gold[recip] += amt;
    Server::exchangeAddGold(chars[perspective]->getSession(), true, amt);
    Server::exchangeAddGold(chars[recip]->getSession(), false, amt);
    return true;
}

/**
 * \brief Returns true if the trade is completed
 *
 * Returns true if the trade is completed
 * \param perspective [in] The perspective of the trader (0=initializer, 1=other)
 */
bool Exchange::confirm(int perspective)
{
    int recip = 1 - perspective;
    if (confirms[recip]) {
        //Trade complete

        chars[perspective]->finishExchange(chars[recip]->getExchangeItems());
        chars[perspective]->addGold(gold[perspective]);
        chars[recip]->finishExchange(chars[perspective]->getExchangeItems());
        chars[recip]->addGold(gold[recip]);
        chars[recip]->clearExchange();
        chars[perspective]->clearExchange();
    }
    Server::confirmExchange(chars[perspective]->getSession(), true);
    Server::confirmExchange(chars[recip]->getSession(), false);
    confirms[perspective] = true;
    return confirms[recip];
}

void Exchange::cancel(int perspective)
{
    int recip = 1 - perspective;

    chars[perspective]->finishExchange(chars[perspective]->getExchangeItems());
    chars[perspective]->addGold(gold[recip]);
    chars[perspective]->clearExchange();
    chars[recip]->finishExchange(chars[recip]->getExchangeItems());
    chars[recip]->addGold(gold[perspective]);
    chars[recip]->clearExchange();

    Server::cancelExchange(chars[perspective]->getSession(), true);
    Server::cancelExchange(chars[recip]->getSession(), false);
}
