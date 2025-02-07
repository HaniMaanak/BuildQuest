#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

class item {
public:
    string name;
    int weight;
    int value;

    virtual void use() = 0;
    virtual ~item() = default;
};

class weapon : public item {
public:
    weapon(string n, int w, int v) {
        name = n;
        weight = w;
        value = v;
    }

    void use() override {
        cout << "Equipped: " << name << " (Strength: " << value << ", Weight: " << weight << ")\n";
    }
};

class armor : public item {
protected:
    int protection; // Armor protection value

public:
    armor(string n, int w, int p) : protection(p) {
        name = n;
        weight = w;
    }

    virtual int get_protection() const {
        return protection;
    }

    void use() override {
        cout << "Worn: " << name << " (Protection: " << protection << ", Weight: " << weight << ")\n";
    }

    void reduce_protection(int amount) {
        protection -= amount;
        if (protection < 0) protection = 0; // Protection cannot be negative
    }
};

class helmet : public armor {
public:
    helmet() : armor("Helmet", 5, 20) {}
};

class chest : public armor {
public:
    chest() : armor("Chest Piece", 15, 50) {}
};

class boots : public armor {
public:
    boots() : armor("Boots", 3, 10) {}
};

class potion : public item {
public:
    potion(string n, int w, int v) {
        name = n;
        weight = w;
        value = v;
    }

    void use() override {
        cout << "Used: " << name << " (Value: " << value << ", Weight: " << weight << ")\n";
    }
};

class staminaPotion : public potion {
    int staminaBoost;

public:
    staminaPotion(string n, int w, int v, int boost)
        : potion(n, w, v), staminaBoost(boost) {}

    void use() override {
        potion::use();
        cout << "Stamina increased by " << staminaBoost << ".\n";
    }

    int get_stamina_boost() const { // Getter for stamina boost
        return staminaBoost;
    }
};

class healthPotion : public potion {
    int healthRecovery;

public:
    healthPotion(string n, int w, int v, int recovery)
        : potion(n, w, v), healthRecovery(recovery) {}

    void use() override {
        potion::use();
        cout << "Health recovered by " << healthRecovery << ".\n";
    }

    int get_health_recovery() const { // Getter for health recovery
        return healthRecovery;
    }
};

class inventory {
    vector<item*> items;

public:
    void add_item(item* newItem) {
        items.push_back(newItem);
    }

    void remove_item(int index) {
        if (index >= 0 && index < items.size()) {
            delete items[index]; // Clean up memory for the item
            items.erase(items.begin() + index); // Remove item from vector
        }
    }

    vector<item*>& get_items() {
        return items;
    }

    ~inventory() {
        for (auto it : items) {
            delete it; // Clean up memory for items
        }
    }
};

class player {
private:
    int health;
    int totalArmor; // Total armor protection
    int stamina;
    int strength;
    inventory* inv; // Inventory pointer accessible in player class
    unordered_set<string> equippedArmor; // Track equipped armor types

public:
    player() : health(100), stamina(50), inv(new inventory()), totalArmor(0), strength(0) {}

    void add_item_to_inv(item* newItem) {
        if (dynamic_cast<armor*>(newItem)) {
            // Check if armor type is already equipped
            if (equippedArmor.find(newItem->name) != equippedArmor.end()) {
                cout << "Cannot equip " << newItem->name << ": already equipped.\n";
                delete newItem; // Clean up memory for the item
                return;
            }

            // Check total armor does not exceed 100
            if (totalArmor + dynamic_cast<armor*>(newItem)->get_protection() <= 100) {
                inv->add_item(newItem);
                totalArmor += dynamic_cast<armor*>(newItem)->get_protection();
                equippedArmor.insert(newItem->name); // Add armor type to equipped set
                cout << newItem->name << " added to inventory. Total Armor: " << totalArmor << endl;
            }
            else {
                cout << "Cannot add " << newItem->name << ": exceeds total armor capacity.\n";
                delete newItem; // Clean up memory for the item
            }
        }
        else {
            inv->add_item(newItem);
            cout << newItem->name << " added to inventory.\n";
        }
    }

    void remove_item_from_inv(int index) {
        if (index >= 0 && index < inv->get_items().size()) {
            if (dynamic_cast<armor*>(inv->get_items()[index])) {
                totalArmor -= dynamic_cast<armor*>(inv->get_items()[index])->get_protection(); // Reduce total armor
                equippedArmor.erase(inv->get_items()[index]->name); // Remove armor from equipped set
            }
            inv->remove_item(index); // Call to remove item from inventory
            cout << "Item removed from inventory.\n";
        }
        else {
            cout << "Invalid item index!" << endl;
        }
    }

    void use_item(int index) {
        if (index >= 0 && index < inv->get_items().size()) {
            inv->get_items()[index]->use();
            if (dynamic_cast<healthPotion*>(inv->get_items()[index])) {
                health += dynamic_cast<healthPotion*>(inv->get_items()[index])->get_health_recovery(); // Use health potion
                if (health > 100) health = 100; // Ensure health does not exceed 100
                cout << "Health is now: " << health << endl;
            }
            else if (dynamic_cast<staminaPotion*>(inv->get_items()[index])) {
                stamina += dynamic_cast<staminaPotion*>(inv->get_items()[index])->get_stamina_boost(); // Use stamina potion
                if (stamina > 100) stamina = 100; // Ensure stamina does not exceed 100
                cout << "Stamina is now: " << stamina << endl;
            }
        }
        else {
            cout << "Invalid item selection!" << endl;
        }
    }

    void display_stats() {
        cout << "Health: " << health << ", Total Armor: " << totalArmor << ", Stamina: " << stamina << ", Strength: " << strength << endl;

        // Health and stamina checks
        if (totalArmor + health > 100) {
            stamina -= 5; // Gradually decrease stamina if weight exceeds limit
            if (stamina < 0) {
                health -= 5; // Decrease health if stamina is exhausted
            }
        }
        if (health <= 0) {
            cout << "You have died! Restarting the game...\n";
            reset(); // Restart the game when health drops to 0
        }
    }

    void take_damage(int damage) {
        if (totalArmor > 0) {
            int damageToArmor = min(damage, totalArmor); // Damage to armor first
            totalArmor -= damageToArmor; // Reduce armor
            cout << "Armor took " << damageToArmor << " damage. Remaining armor: " << totalArmor << endl;

            if (totalArmor < 0) totalArmor = 0; // Ensure armor does not go negative

            // If armor is depleted, damage health
            damage -= damageToArmor;
        }

        // Now apply damage to health if any remains
        if (damage > 0) {
            health -= damage;
            cout << "Health took " << damage << " damage. Remaining health: " << health << endl;
        }

        // Check for death
        if (health <= 0) {
            cout << "You have died! Restarting the game...\n";
            reset();
        }
    }

    void reset() {
        health = 100;
        totalArmor = 0;
        stamina = 50;
        strength = 0;
        inv = new inventory(); // Reset inventory
        equippedArmor.clear(); // Clear equipped armor set
    }

    inventory* get_inventory() {
        return inv; // Method to access inventory from outside
    }

    ~player() {
        delete inv; // Clean up memory for inventory
    }
};

int main() {
    player p; // Create a player object
    bool running = true; // Flag to keep the game running
    int choice;

    cout << "Welcome to the Adventure Game!\n";
    cout << "You will explore, fight, and collect items!\n";
    cout << "Your goal is to survive and become the strongest warrior!\n\n";

    while (running) {
        cout << "\n--- Player Menu ---" << endl;
        cout << "1. Add Item to Inventory" << endl;
        cout << "2. Use Item" << endl;
        cout << "3. Display Stats" << endl;
        cout << "4. Take Damage" << endl; // New option to take damage
        cout << "5. Exit Game" << endl;
        cout << "Choose an action (1-5): ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "\nSelect an item to add:\n";
            cout << "1. Sword (Weight: 5, Value: 15)\n";
            cout << "2. Bow (Weight: 3, Value: 20)\n";
            cout << "3. Helmet (Weight: 5, Protection: 20)\n";
            cout << "4. Chest Piece (Weight: 15, Protection: 50)\n";
            cout << "5. Boots (Weight: 3, Protection: 10)\n";
            cout << "6. Health Potion (Weight: 1, Recovery: 20)\n";
            cout << "7. Stamina Potion (Weight: 1, Boost: 10)\n";

            int itemChoice;
            cout << "Enter item choice (1-7): ";
            cin >> itemChoice;

            switch (itemChoice) {
            case 1:
                p.add_item_to_inv(new weapon("Sword", 5, 15));
                break;
            case 2:
                p.add_item_to_inv(new weapon("Bow", 3, 20));
                break;
            case 3:
                p.add_item_to_inv(new helmet());
                break;
            case 4:
                p.add_item_to_inv(new chest());
                break;
            case 5:
                p.add_item_to_inv(new boots());
                break;
            case 6:
                p.add_item_to_inv(new healthPotion("Health Potion", 1, 20, 20));
                break;
            case 7:
                p.add_item_to_inv(new staminaPotion("Stamina Potion", 1, 10, 10));
                break;
            default:
                cout << "Invalid choice! Please choose again." << endl;
            }
            break;
        }
        case 2: {
            cout << "Select an item to use:\n";
            int index = 0;
            for (item* it : p.get_inventory()->get_items()) {
                cout << index++ << ". " << it->name << endl;
            }
            int itemIndex;
            cout << "Enter item index to use: ";
            cin >> itemIndex;

            p.use_item(itemIndex);
            break;
        }
        case 3:
            p.display_stats();
            break;
        case 4: {
            int damage;
            cout << "Enter damage amount (e.g., 10, 20): ";
            cin >> damage;
            p.take_damage(damage); // Take damage from the player
            break;
        }
        case 5:
            running = false; // Exit the game
            cout << "Thank you for playing! Goodbye!\n";
            break;
        default:
            cout << "Invalid choice! Please select a valid option." << endl;
        }

        // Additional gameplay mechanics
        if (running) {
            // Random events, challenges, or encounters could be added here
            int randomEvent = rand() % 5; // Random number between 0 and 4
            if (randomEvent == 0) {
                cout << "You encountered a wild beast! Prepare for battle!\n";
                // Implement battle logic here
                int damage = (rand() % 20) + 5; // Random damage between 5 and 25
                p.take_damage(damage);
            }
        }
    }

    return 0;
}