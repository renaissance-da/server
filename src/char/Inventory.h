/*
 * Inventory.h
 *
 *  Created on: 2013-05-16
 *      Author: per
 */

#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "Item.h"
#include "Equipment.h"
#include <assert.h>

template<int N>
class Inventory
{
public:

    Inventory();
    ~Inventory();

    void clear();

    unsigned int getWeight()
    {
        return weight;
    }
    unsigned int getFreeSlots()
    {
        return freeSlots;
    }
    unsigned int getFullSlots()
    {
        return N - freeSlots;
    }
    int repairCost();
    unsigned short countItems(int id, unsigned short mod);

    template<typename Holder>
    bool repairAll(unsigned int &gold, Holder *holder);
    template<typename Holder>
    bool repair(unsigned int slot, unsigned int &gold, Holder *holder);

    template<typename Holder>
    Item *take(int id, int amt, int mod, Holder *holder);
    template<typename Holder>
    Item *remove(int slot, Holder *holder);
    template<typename Holder>
    Item *removeSome(int slot, int amt, Holder *holder);

    template<typename Holder>
    bool addItem(Item *item, Holder *holder);
    template<typename Holder>
    void putItem(Item *item, int slot, Holder *holder);

    template<typename Holder>
    void swap(int s1, int s2, Holder *holder);

    Item **allItems()
    {
        return items;
    }
    Item *operator[](int offset)
    {
        assert(offset >= 0 && offset < N);
        return items[offset];
    }

private:
    template<typename Holder>
    Item *takeStacking(int id, int amt, int slot, Holder *holder);
    template<typename Holder>
    Item *takeSingles(int id, int amt, int mod, int slot, Holder *holder);

    Item *items[N];
    int weight, freeSlots;
};

/**
 * \brief Constructs an empty inventory of size N
 *
 * After construction, all slots will be null.
 */
template<int N>
Inventory<N>::Inventory()
{
    for (int i = 0; i < N; i++) {
        items[i] = 0;
    }

    weight = 0;
    freeSlots = N;
}

/**
 * \brief Destroys an inventory
 *
 * The destructor for Inventory will deallocate all items remaining in its
 * slots. See \ref clear for more information.
 */
template<int N>
Inventory<N>::~Inventory()
{
    clear();
}

/**
 * \brief Clear an inventory of its items
 *
 * Clear will deallocate any items which are currently in this inventory,
 * and set all slots to null. Since it is assumed that this function will
 * be used to clean up an inventory which is going to be destroyed, no
 * notifications are given to the caller.
 */
template<int N>
void Inventory<N>::clear()
{
    for (int i = 0; i < N; i++) {
        if (items[i]) {
            delete items[i];
            items[i] = 0;
        }
    }
    weight = 0;
    freeSlots = N;
}

/**
 * \brief Removes an item from a given slot
 *
 * Removes an item from a given slot, if an item is in said slot.
 * \param[in] slot The slot to remove the item from.
 * \param[out] holder The object to be notified about changes to this inventory.
 * \return The item which was in the given slot, or null if no item was present.
 * \pre 0 <= slot < N
 */
template<int N>
template<typename Holder>
Item *Inventory<N>::remove(int slot, Holder *holder)
{
    assert(slot >= 0 && slot < N);

    Item *r = items[slot];
    items[slot] = 0;
    if (r) {
        weight -= r->getWeight();
        freeSlots++;
        holder->removedItem(slot);
    }

    return r;
}

/**
 * \brief Decrease the quantity of an item in a given slot
 *
 * Decrease the quantity of an item in a given slot. If the quantity is
 * depleted, the item will be destroyed. If amt is greater than the
 * current quantity, the current quantity is removed instead.
 * \param[in] slot The slot to take the item from.
 * \param[in] amt The amount to take from the stack.
 * \param[out] holder The object to be notified about changes to this inventory.
 * \return A stack of the item removed with the appropriate quantity, or null if the
 * slot was empty.
 * \pre 0 <= slot < N
 */
template<int N>
template<typename Holder>
Item *Inventory<N>::removeSome(int slot, int amt, Holder *holder)
{
    assert(0 <= slot && N > slot);

    if (items[slot]) {
        if (items[slot]->getQty() > amt) {
            items[slot]->addQty(-amt);
            holder->addedItem(items[slot], slot);
            return new Item(items[slot]->getId(), amt, items[slot]->getDur());
        }
        else {
            weight -= items[slot]->getWeight();
            freeSlots++;

            //delete items[slot];
            Item *r = items[slot];
            items[slot] = 0;
            holder->removedItem(slot);
            return r;
        }
    }

    return 0;
}

/**
 * \brief Calculates the cost of repairing everything in this inventory
 *
 * The cost to repair everything is just the sum of the cost to repair each
 * item in the inventory.
 * \return The cost to repair all items in this inventory, in gold. -1 if all
 * items are repaired.
 * \sa Item::getRepairCost()
 */
template<int N>
int Inventory<N>::repairCost()
{
    unsigned int cost = 0;
    bool allRepaired = true;
    for (int i = 0; i < N; i++) {
        if (items[i]) {
            allRepaired = !items[i]->canRepair() && allRepaired;
            cost += items[i]->repairCost();
        }
    }
    return allRepaired ? -1 : cost;
}

/**
 * \brief Counts the number of items of a given mod within this inventory
 *
 *  Counts the number of items of a given mod within this inventory. If the
 *  item to be counted can be stacked, then it will be assumed that only one
 *  stack is in this inventory.
 *  \param[in] id The unique identifier of the item to be counted.
 *  \param[in] mod The modifier of the item to be counted. If the item cannot
 *  be modified, this should be set to 0.
 *  \return The number of items found within this inventory.
 */
template<int N>
unsigned short Inventory<N>::countItems(int id, unsigned short mod)
{
    unsigned short qty = 0;
    int i;
    for (i = 0; i < N; i++) {
        if (items[i] && items[i]->getId() == id && items[i]->getMod() == mod) {
            if (items[i]->getMaxQty())
                return items[i]->getQty();
            else {
                qty = 1;
                break;
            }
        }
    }

    for (++i; i < N; i++) {
        if (items[i] && items[i]->getId() == id && items[i]->getMod() == mod)
            ++qty;
    }

    return qty;
}

/**
 * \brief Attempts to repair all items in this inventory
 *
 * Repairs items starting from slot 0 until an item is found which is
 * too expensive to repair, or all items are repaired.
 * \param[in,out] gold The gold variable which is being used to pay for the repairs.
 * \param[out] holder The object which will be notified as a result of any of the
 * 				items in this inventory changing state.
 * \return True if all items were repaired, False if some items were not repaired
 * 				(possibly because gold was initially less than \ref repairCost)
 */
template<int N>
template<typename Holder>
bool Inventory<N>::repairAll(unsigned int &gold, Holder *holder)
{
    for (int i = 0; i < N; i++) {
        if (items[i]) {
            if (items[i]->repairCost() <= gold) {
                gold -= items[i]->repairCost();
                items[i]->repair();
                holder->addedItem(items[i], i);
            }
            else
                return false;
        }
    }
    return true;
}

/**
 * \brief Attempts to repair a specific item in this inventory
 *
 * Repairs a single item in this inventory, as specified by slot.
 * \param[in] slot The slot of the item to be repaired.
 * \param[in,out] gold The gold variable which is being used to pay for the repair.
 * \param[in] holder The object which will be notified as a result of any of the
 * 			items in this inventory changing state.
 * \return True if the item was repaired, false otherwise.
 */
template<int N>
template<typename Holder>
bool Inventory<N>::repair(unsigned int slot, unsigned int &gold, Holder *holder)
{
    assert(slot < N);
    if (!items[slot] || items[slot]->repairCost() > gold)
        return false;
    gold -= items[slot]->repairCost();
    items[slot]->repair();
    holder->addedItem(items[slot], slot);
    return true;
}

/**
 * \brief Take some items of a particular ID and modifier from this inventory
 *
 * Take some items from this inventory. Either all of the items requested are taken,
 * or none. The items taken must be fully repaired, if applicable.
 * \param[in] id The ID of the items to be taken.
 * \param[in] amt The quantity of the items to be taken.
 * \param[in] mod The modifier of the items to be taken. If the items to be taken cannot
 * 					be modified, this should be set to 0.
 * \param[out] holder The object to be notified as a result of any of the slots of this
 * 				inventory changing state.
 * \return On success, returns a pointer to one or more of the items removed. If the items
 * 			removed were stack-able, the returned item will have a quantity equal to amt.
 * 			Otherwise, the returned item will only be the first of the items removed.
 * 			On failure, returns null.
 */
template<int N>
template<typename Holder>
Item *Inventory<N>::take(int id, int amt, int mod, Holder *holder)
{
    for (int i = 0; i < N; i++) {
        if (items[i] && items[i]->getId() == id && items[i]->getMod() == mod) {
            if (items[i]->getMaxQty())
                return takeStacking(id, amt, i, holder);
            else
                return takeSingles(id, amt, mod, i, holder);
        }
    }

    return 0;
}

template<int N>
template<typename Holder>
Item *Inventory<N>::takeStacking(int id, int amt, int slot, Holder *holder)
{
    if (items[slot]->getQty() > amt) {
        Item *ret = new Item(id, amt, 0);
        items[slot]->addQty(-amt);
        holder->addedItem(items[slot], slot);
        return ret;
    }
    else if (items[slot]->getQty() == amt) {
        Item *ret = items[slot];
        items[slot] = 0;
        weight -= ret->getWeight();
        holder->removedItem(slot);
        ++freeSlots;
        return ret;
    }
    else
        return 0;
}

template<int N>
template<typename Holder>
Item *Inventory<N>::takeSingles(int id, int amt, int mod, int slot,
    Holder *holder)
{
    if (amt > N)
        return 0;

    int slots[N];
    int count = 1;
    slots[0] = slot;
    for (int i = slot + 1; i < N && count < amt; i++) {
        if (items[i] && items[i]->getId() == id && items[i]->getMod() == mod) {
            if (items[i]->getType() == BaseItem::EQUIP
                && ((Equipment *) items[i])->getDur()
                    != ((Equipment *) items[i])->getMaxDur())
                continue;
            slots[count++] = i;
        }
    }

    if (count == amt) {
        //can take this many
        //can only return the first one
        for (int i = 1; i < count; i++) {
            weight -= items[slots[i]]->getWeight();
            delete items[slots[i]];
            holder->removedItem(slots[i]);
            items[slots[i]] = 0;
        }
        weight -= items[slots[0]]->getWeight();
        Item *ret = items[slots[0]];
        items[slots[0]] = 0;
        holder->removedItem(slots[0]);
        freeSlots += amt;
        return ret;
    }

    return 0;
}

/**
 * \brief Add an item to this inventory
 *
 * Add an item to this inventory. If the item is stack-able and another item of
 * the same kind is present in this inventory, the quantities will be combined.
 * \param[in] item The item to be added to the inventory. If this method is
 * 				successful, then this pointer should be considered invalid after
 * 				the call. On failure, the caller is responsible for cleaning up item.
 * \param[out] holder The object to be notified if this inventory changes state.
 * \return True if the item was successfully added, and false otherwise. May fail
 * 			either because no slots were free or because the maximum quantity for
 * 			this item was exceeded for this inventory.
 */
template<int N>
template<typename Holder>
bool Inventory<N>::addItem(Item *item, Holder *holder)
{
    if (item->getMaxQty()) {
        for (int i = 0; i < N; i++) {
            if (items[i] && items[i]->getId() == item->getId()) {
                //try to combine
                if (item->getMaxQty() < item->getQty() + items[i]->getQty()) {
                    //Take the maximum possible from the item given
                    item->addQty(items[i]->getQty() - item->getMaxQty());
                    items[i]->addQty(item->getMaxQty() - items[i]->getQty());
                    holder->addedItem(items[i], i);
                    return false;
                }
                items[i]->addQty(item->getQty());
                holder->addedItem(items[i], i);
                return true;
            }
        }
    }
    for (int i = 0; i < N; i++) {
        if (!items[i]) {
            items[i] = item;
            weight += item->getWeight();
            holder->addedItem(items[i], i);
            --freeSlots;
            return true;
        }
    }
    return false;
}

/**
 * \brief Puts an item in a slot
 *
 * Puts an item in the specified slot. This should only be used if the caller
 * can guarantee that the slot was previously empty (such as with a new inventory,
 * or right after clearing the inventory).
 * \param[in] item The item to put in the given slot.
 * \param[in] slot The slot to put the item in.
 * \param[out] holder The object to notify about changes to this inventory.
 * \pre slot must be at least 0 and less than N. The slot specified must be null.
 */
template<int N>
template<typename Holder>
void Inventory<N>::putItem(Item *item, int slot, Holder *holder)
{
    assert(slot < N && !items[slot]);

    freeSlots--;
    items[slot] = item;
    weight += item->getWeight();
    holder->addedItem(item, slot);
}

/**
 * \brief Swap two items' positions
 *
 * Swap two items' positions. This can be used if one or both slots are empty.
 * \param[in] s1 The slot of the first item to be swapped.
 * \param[in] s2 The slot of the second item to be swapped.
 * \param[out] holder The object to be notified of any changes to this inventory.
 * \pre s1 and s2 must be valid slots (between 0 and N-1 inclusive).
 */
template<int N>
template<typename Holder>
void Inventory<N>::swap(int s1, int s2, Holder *holder)
{
    assert(s1 < N && s2 < N);

    Item *i1 = items[s1];
    Item *i2 = items[s2];

    holder->removedItem(s1);
    holder->removedItem(s2);

    items[s2] = i1;
    items[s1] = i2;
    if (i1)
        holder->addedItem(i1, s2);
    if (i2)
        holder->addedItem(i2, s1);
}

#endif /* INVENTORY_H_ */
