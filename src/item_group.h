#ifndef _ITEM_GROUP_H_
#define _ITEM_GROUP_H_

#include "item.h"
#include <vector>
#include <set>
#include <string>
#include <memory>

typedef std::string Item_tag;
typedef std::string Group_tag;

/**
 * Base interface for item spawn.
 * Used to generate a list of items.
 */
class Item_spawn_data {
public:
    typedef std::vector<item> ItemList;
    typedef std::vector<Item_tag> RecursionList;

    Item_spawn_data(int _probability) : probability(_probability) { }
    virtual ~Item_spawn_data() { }
    /**
     * Create a list of items. The create list might be empty.
     * No item of it will be the null item.
     * @param birthday All items have that value as birthday.
     */
    virtual ItemList create(int birthday, RecursionList &rec) const = 0;
    ItemList create(int birthday) const {
        RecursionList rec;
        return create(birthday, rec);
    }
    /**
     * The same as create, but create a single item only.
     * The returned item might be a null item!
     */
    virtual item create_single(int birthday, RecursionList &rec) const = 0;
    item create_single(int birthday) const {
        RecursionList rec;
        return create_single(birthday, rec);
    }
    /**
     * Check item / spawn settings for consistency. Includes
     * checking for valid item types and valid settings.
     */
    virtual void check_consistency() const = 0;
    /**
     * For item blacklisted, remove the given item from this and
     * all linked groups.
     */
    virtual bool remove_item(const Item_tag &itemid) = 0;
    virtual bool has_item(const Item_tag &itemid) const = 0;
    // TODO: remove this legacy function
    virtual bool guns_have_ammo() const { return false; }
    /** probability, used by the parent object. */
    int probability;
private:
    // not implemented
    Item_spawn_data(const Item_spawn_data&);
    Item_spawn_data&operator=(const Item_spawn_data&);
};
/**
 * Creates a single item, but can change various aspects
 * of the created item.
 * This is basically a wrapper around @ref Single_item_creator
 * that adds item properties.
 */
class Item_modifier {
public:
    std::pair<int, int> damage;
    std::pair<int, int> count;
    /**
     * Charges to spawn the item with, if this turns out to
     * be negative, the default charges are used.
     */
    std::pair<long, long> charges;
    /**
     * Ammo for guns. If NULL the gun spawns
     * without ammo.
     * This does not take @ref charges into count. Instead it
     * assumes that the item returned by that Single_item_creator
     * contains the charges.
     */
    std::unique_ptr<Item_spawn_data> ammo;
    /**
     * Item should spawn inside this container, can be NULL,
     * if item should not spawn in a container.
     * If the created item is a liquid and it uses the default
     * charges, it will expand/shrink to fill the container completely.
     * If it is created with to much charges, they are reduced.
     * If it is created with the non-default charges, but it still fits
     * it is not changed.
     */
    std::unique_ptr<Item_spawn_data> container;
    /**
     * This is used to create the contents of an item.
     */
    std::unique_ptr<Item_spawn_data> contents;

    Item_modifier();
    ~Item_modifier();

    void modify(item &it) const;
    void check_consistency() const;
    bool remove_item(const Item_tag &itemid);
};
/**
 * Basic item creator. It contains either the item id directly,
 * or a name of a group that it queries for it.
 * It can spawn a single item and a item list.
 * Creates a single item, with its standard properties
 * (charges, no damage, ...).
 * As it inherits from @ref Item_spawn_data it can also wrap that
 * created item into a list. That list will always contain a
 * single item (or no item at all).
 * Note that the return single item my be a null item.
 * The returned item list will (according to the definition in
 * @ref Item_spawn_data) never contain null items, that's why it
 * might be empty.
 */
class Single_item_creator : public Item_spawn_data {
public:
    typedef enum {
        S_ITEM_GROUP,
        S_ITEM,
        S_NONE,
    } Type;

    Single_item_creator(const std::string &id, Type type, int probability);
    virtual ~Single_item_creator();

    /**
     * Id of the item group or id of the item.
     */
    std::string id;
    Type type;
    std::unique_ptr<Item_modifier> modifier;

    virtual ItemList create(int birthday, RecursionList &rec) const;
    virtual item create_single(int birthday, RecursionList &rec) const;
    virtual void check_consistency() const;
    virtual bool remove_item(const Item_tag &itemid);
    virtual bool has_item(const Item_tag &itemid) const;
};

/**
 * This is a list of item spawns. It can act as distribution
 * (one entry from the list) or as collection (all entries get a chance).
 */
class Item_group : public Item_spawn_data {
public:
    typedef std::vector<item> ItemList;
    typedef enum {
        G_COLLECTION,
        G_DISTRIBUTION
    } Type;

    Item_group(Type type, int probability);
    virtual ~Item_group();

    const Type type;
    /**
     * Item data to create item from and probability
     * that items should be created.
     * If type is G_COLLECTION, probability is in percent.
     * A value of 100 means always create items, a value of 0 never.
     * If type is G_DISTRIBUTION, probability is relative,
     * the sum probability is sum_prob.
     */
    typedef std::vector<Item_spawn_data*> prop_list;

    void add_item_entry(const Item_tag &itemid, int probability);
    void add_group_entry(const Group_tag &groupid, int probability);
    void add_entry(std::unique_ptr<Item_spawn_data> &ptr);

    virtual ItemList create(int birthday, RecursionList &rec) const;
    virtual item create_single(int birthday, RecursionList &rec) const;
    virtual void check_consistency() const;
    virtual bool remove_item(const Item_tag &itemid);
    virtual bool has_item(const Item_tag &itemid) const;

protected:
    /**
     * Contains the sum of the probability of all entries
     * that this group contains.
     */
    int sum_prob;
    /**
     * Links to the entries in this group.
     */
    prop_list items;

public:
    // TODO: remove this legacy function
    virtual bool guns_have_ammo() const { return with_ammo; }
    // TODO: remove this legacy member
    bool with_ammo;
};

#endif
