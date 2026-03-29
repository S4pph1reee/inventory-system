#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstring>
#include <cmath>
#include <cstdio>

#pragma pack(push, 1)
const int MAX_LEN = 100;

struct Inventory {
    char item_name[MAX_LEN];
    bool quest;
    int cost_per_unit;
    char category[MAX_LEN];
    double weight;
    int quantity;
    
    int Full_cost() const { return cost_per_unit * quantity; }
};
#pragma pack(pop)

const char* FILENAME = "inventory.bin";
const char* TMP_FILE = "inventory_tmp.bin";
const size_t REC_SIZE = sizeof(Inventory);

auto ReadNumber(const char* prompt) {
    double temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < 0.0) {
                std::cout << "Value must be non-negative.\n";
                std::cin.ignore(999999, '\n');
                continue;
            }
            std::cin.ignore(999999, '\n');
            return temp;
        }
        else {
            std::cout << "Invalid value. Try again\n";
            std::cin.clear();
            std::cin.ignore(999999, '\n');
        }
    }
}

static void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int GetRecordCount() {
    std::ifstream file(FILENAME, std::ios::binary | std::ios::ate);
    if (!file) return 0;
    return static_cast<int>(file.tellg() / REC_SIZE);
}

bool ReadRecordAt(int index, Inventory& out) {
    std::ifstream file(FILENAME, std::ios::binary);
    if (!file) return false;
    file.seekg(index * REC_SIZE);
    return file.read(reinterpret_cast<char*>(&out), REC_SIZE).good();
}

bool WriteRecordAt(int index, const Inventory& in) {
    std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return false;
    file.seekp(index * REC_SIZE);
    return file.write(reinterpret_cast<const char*>(&in), REC_SIZE).good();
}

void SwapRecords(int idx1, int idx2) {
    if (idx1 == idx2) return;
    Inventory a{}, b{};
    ReadRecordAt(idx1, a);
    ReadRecordAt(idx2, b);
    WriteRecordAt(idx1, b);
    WriteRecordAt(idx2, a);
}

void PrintItem(const Inventory& item) {
    std::cout << "=================================\n";
    std::cout << "Name:     " << item.item_name << '\n';
    std::cout << "Quest:    " << (item.quest ? "Yes" : "No") << '\n';
    std::cout << "Cost:     " << item.cost_per_unit << " gold\n";
    std::cout << "Category: " << item.category << '\n';
    std::cout << "Weight:   " << item.weight << " kg\n";
    std::cout << "Quantity: " << item.quantity << '\n';
    std::cout << "Total:    " << item.Full_cost() << " gold\n";
    std::cout << "=================================\n";
}

int LinearSearchInFile(const char* name) {
    int n = GetRecordCount();
    Inventory temp{};
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, temp) && _stricmp(temp.item_name, name) == 0)
            return i;
    }
    return -1;
}

int BinarySearchInFile_ByWeight(double target) {
    int n = GetRecordCount();
    if (n == 0) return -1;
    
    int left = 0, right = n - 1;
    Inventory temp{};
    
    while (left <= right) {
        int mid = (left + right) / 2;
        if (!ReadRecordAt(mid, temp)) break;
        
        if (std::abs(temp.weight - target) < 0.001)
            return mid;
        else if (temp.weight < target)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

void UpdateQuantityByName(const char* name, int addQty) {
    int pos = LinearSearchInFile(name);
    if (pos == -1) return;
    
    Inventory item{};
    if (ReadRecordAt(pos, item)) {
        item.quantity += addQty;
        WriteRecordAt(pos, item);
        std::cout << "Updated quantity: +" << addQty << "\n";
    }
}

void AddItemToFile() {
    if (std::cin.peek() == '\n') std::cin.ignore(1);
    
    Inventory item{};
    
    // 1. Ввод имени (НЕТ system("cls") здесь!)
    std::cout << "\n\n=== Add New Item ===\n\n";
    std::cout << "Print Item Name: ";
    std::cin.getline(item.item_name, MAX_LEN);
    
    // Проверка на дубликат
    if (LinearSearchInFile(item.item_name) != -1) {
        std::cout << "Item '" << item.item_name << "' already exists!\n";
        std::cout << "Add quantity to existing? (y/n): ";
        if (_getch() == 'y') {
            int addQty = static_cast<int>(ReadNumber("Quantity to add: "));
            UpdateQuantityByName(item.item_name, addQty);
        }
        system("pause");
        system("cls");
        return;
    }
    
    // 2. Quest item (здесь КЛЕАРИМ)
    system("cls");
    std::cout << "\n--- Add New Item ---\n";
    std::cout << "Name: " << item.item_name << "\n\n";
    char buff[MAX_LEN];
    while (true) {
        std::cout << "Quest item? (quest/not): ";
        std::cin.getline(buff, MAX_LEN);
        if (_stricmp(buff, "quest") == 0) { item.quest = true; break; }
        if (_stricmp(buff, "not") == 0) { item.quest = false; break; }
        std::cout << "Please enter 'quest' or 'not'\n";
    }
    
    // 3. Cost
    system("cls");
    std::cout << "\n--- Add New Item ---\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n\n";
    item.cost_per_unit = static_cast<int>(ReadNumber("Cost per unit: "));
    
    // 4. Category
    system("cls");
    std::cout << "\n--- Add New Item ---\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n\n";
    std::cout << "Category: ";
    std::cin.getline(item.category, MAX_LEN);
    
    // 5. Weight
    system("cls");
    std::cout << "\n--- Add New Item ---\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n";
    std::cout << "Category: " << item.category << "\n\n";
    item.weight = ReadNumber("Weight per unit: ");
    
    // 6. Quantity
    system("cls");
    std::cout << "\n--- Add New Item ---\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n";
    std::cout << "Category: " << item.category << "\n";
    std::cout << "Weight: " << item.weight << "\n\n";
    item.quantity = static_cast<int>(ReadNumber("Quantity: "));
    
    // Сохранение
    system("cls");
    std::ofstream file(FILENAME, std::ios::binary | std::ios::app);
    if (file) {
        file.write(reinterpret_cast<char*>(&item), REC_SIZE);
        std::cout << "\nItem added successfully!\n\n";
        std::cout << "  Name:       " << item.item_name << "\n";
        std::cout << "  Quest:      " << (item.quest ? "Yes" : "No") << "\n";
        std::cout << "  Cost:       " << item.cost_per_unit << " gold\n";
        std::cout << "  Category:   " << item.category << "\n";
        std::cout << "  Weight:     " << item.weight << " kg\n";
        std::cout << "  Quantity:   " << item.quantity << "\n";
        std::cout << "  Total Cost: " << item.Full_cost() << " gold\n";
    }
    else {
        std::cout << "Error writing to file!\n";
    }
    system("pause");
    system("cls");
}

void EditItem() {
    if (std::cin.peek() == '\n') std::cin.ignore(1);
    
    char name[MAX_LEN];
    std::cout << "Enter item name to edit: ";
    std::cin.getline(name, MAX_LEN);
    
    int pos = LinearSearchInFile(name);
    if (pos == -1) {
        std::cout << "Item not found.\n";
        system("pause");
        return;
    }
    
    Inventory item{};
    ReadRecordAt(pos, item);
    
    std::cout << "\nEditing: " << item.item_name << "\n";
    std::cout << "1. Change cost: " << item.cost_per_unit << "\n";
    std::cout << "2. Change weight: " << item.weight << "\n";
    std::cout << "3. Change quantity: " << item.quantity << "\n";
    std::cout << "4. Change category: " << item.category << "\n";
    std::cout << "5. Toggle quest flag: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "0. Cancel\nSelect: ";
    
    char choice = _getch();
    std::cout << choice << "\n";
    
    bool modified = false;
    switch (choice) {
        case '1': item.cost_per_unit = static_cast<int>(ReadNumber("New cost: ")); modified = true; break;
        case '2': item.weight = ReadNumber("New weight: "); modified = true; break;
        case '3': item.quantity = static_cast<int>(ReadNumber("New quantity: ")); modified = true; break;
        case '4': std::cout << "New category: "; std::cin.getline(item.category, MAX_LEN); modified = true; break;
        case '5': item.quest = !item.quest; modified = true; break;
    }
    
    if (modified) WriteRecordAt(pos, item);
    std::cout << "Saved!\n";
    system("pause");
}

void DeleteItem() {
    if (std::cin.peek() == '\n') std::cin.ignore(1);
    
    char name[MAX_LEN];
    std::cout << "Enter item name to DELETE: ";
    std::cin.getline(name, MAX_LEN);
    
    int pos = LinearSearchInFile(name);
    if (pos == -1) {
        std::cout << "Item not found.\n";
        system("pause");
        return;
    }
    
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    
    Inventory temp{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (index != pos)
            tmp.write(reinterpret_cast<char*>(&temp), REC_SIZE);
        ++index;
    }
    src.close(); tmp.close();
    
    remove(FILENAME);
    rename(TMP_FILE, FILENAME);
    
    std::cout << "Item deleted!\n";
    system("pause");
}

void PrintAllFromFile() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        return;
    }
    
    Inventory item{};
    std::cout << "\n=== INVENTORY (" << n << " items) ===\n\n";
    
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
            std::cout << "[" << i << "] ";
            PrintItem(item);
        }
    }
    system("pause");
}

void SortFileByWeight_Bubble() {
    int n = GetRecordCount();
    if (n <= 1) { std::cout << "Nothing to sort.\n"; system("pause"); return; }
    
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            Inventory a{}, b{};
            ReadRecordAt(j, a);
            ReadRecordAt(j + 1, b);
            if (a.weight > b.weight) {
                SwapRecords(j, j + 1);
            }
        }
    }
    std::cout << "Sorted by weight (bubble)!\n";
    system("pause");
}

void SortFileByQuantity_Selection() {
    int n = GetRecordCount();
    if (n <= 1) { std::cout << "Nothing to sort.\n"; system("pause"); return; }
    
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        Inventory min_item{};
        ReadRecordAt(i, min_item);
        
        for (int j = i + 1; j < n; ++j) {
            Inventory curr{};
            ReadRecordAt(j, curr);
            if (curr.quantity < min_item.quantity) {
                min_item = curr;
                min_idx = j;
            }
        }
        if (min_idx != i) SwapRecords(i, min_idx);
    }
    std::cout << "Sorted by quantity (selection)!\n";
    system("pause");
}

void SortFileByName_Insertion() {
    int n = GetRecordCount();
    if (n <= 1) { std::cout << "Nothing to sort.\n"; system("pause"); return; }
    
    for (int i = 1; i < n; ++i) {
        Inventory key{};
        ReadRecordAt(i, key);
        int j = i - 1;
        
        while (j >= 0) {
            Inventory curr{};
            ReadRecordAt(j, curr);
            if (_stricmp(curr.item_name, key.item_name) > 0) {
                WriteRecordAt(j + 1, curr);
                --j;
            }
            else break;
        }
        WriteRecordAt(j + 1, key);
    }
    std::cout << "Sorted by name (insertion)!\n";
    system("pause");
}

void WriteToTmpFile(const Inventory& item) {
    std::ofstream tmp(TMP_FILE, std::ios::binary | std::ios::app);
    if (tmp) tmp.write(reinterpret_cast<const char*>(&item), REC_SIZE);
}

void ClearTmpFile() {
    std::ofstream tmp(TMP_FILE, std::ios::binary | std::ios::trunc);
}

void PrintTmpFile() {
    std::ifstream tmp(TMP_FILE, std::ios::binary);
    if (!tmp) return;
    Inventory item{};
    while (tmp.read(reinterpret_cast<char*>(&item), REC_SIZE))
        PrintItem(item);
    tmp.close();
}

int GetTmpRecordCount() {
    std::ifstream tmp(TMP_FILE, std::ios::binary | std::ios::ate);
    if (!tmp) return 0;
    return static_cast<int>(tmp.tellg() / REC_SIZE);
}

void SortTmpFileByCost_Desc() {
    int n = GetTmpRecordCount();
    if (n <= 1) return;
    
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            Inventory a{}, b{};
            std::ifstream tmp(TMP_FILE, std::ios::binary);
            tmp.seekg(j * REC_SIZE);
            tmp.read(reinterpret_cast<char*>(&a), REC_SIZE);
            tmp.read(reinterpret_cast<char*>(&b), REC_SIZE);
            tmp.close();
            
            if (a.cost_per_unit < b.cost_per_unit) {
                std::fstream tmp(TMP_FILE, std::ios::binary | std::ios::in | std::ios::out);
                tmp.seekp(j * REC_SIZE);
                tmp.write(reinterpret_cast<char*>(&b), REC_SIZE);
                tmp.write(reinterpret_cast<char*>(&a), REC_SIZE);
                tmp.close();
            }
        }
    }
}

void SearchByWeightRangeAndCategory() {
    ClearTmpFile();
    
    double low = ReadNumber("Lower weight bound: ");
    double high = ReadNumber("Upper weight bound: ");
    if (low > high) std::swap(low, high);
    
    char category[MAX_LEN];
    std::cout << "Category: ";
    std::cin >> category;
    std::cin.ignore(999999, '\n');
    
    int n = GetRecordCount();
    Inventory item{};
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
            if (_stricmp(item.category, category) == 0 && 
                item.weight >= low && item.weight <= high) {
                WriteToTmpFile(item);
            }
        }
    }
    
    int matches = GetTmpRecordCount();
    if (matches == 0) {
        std::cout << "No items found.\n";
        system("pause");
        return;
    }
    
    SortTmpFileByCost_Desc();
    
    std::cout << "\n=== Results (sorted by cost/unit DESC) ===\n";
    PrintTmpFile();
    
    remove(TMP_FILE);
    system("pause");
}

void ViewInventoryByCategory() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        return;
    }
    
    ClearTmpFile();
    
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    Inventory item{};
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE))
        tmp.write(reinterpret_cast<char*>(&item), REC_SIZE);
    src.close(); tmp.close();
    
    int tmpN = GetTmpRecordCount();
    for (int i = 1; i < tmpN; ++i) {
        Inventory key{};
        std::ifstream tIn(TMP_FILE, std::ios::binary);
        tIn.seekg(i * REC_SIZE);
        tIn.read(reinterpret_cast<char*>(&key), REC_SIZE);
        tIn.close();
        
        int j = i - 1;
        while (j >= 0) {
            Inventory curr{};
            std::ifstream tIn2(TMP_FILE, std::ios::binary);
            tIn2.seekg(j * REC_SIZE);
            tIn2.read(reinterpret_cast<char*>(&curr), REC_SIZE);
            tIn2.close();
            
            int catCmp = _stricmp(curr.category, key.category);
            int nameCmp = (catCmp == 0) ? _stricmp(curr.item_name, key.item_name) : catCmp;
            
            if (nameCmp > 0) {
                std::fstream tOut(TMP_FILE, std::ios::binary | std::ios::in | std::ios::out);
                tOut.seekp((j + 1) * REC_SIZE);
                tOut.write(reinterpret_cast<char*>(&curr), REC_SIZE);
                tOut.close();
                --j;
            }
            else break;
        }
        std::fstream tOut(TMP_FILE, std::ios::binary | std::ios::in | std::ios::out);
        tOut.seekp((j + 1) * REC_SIZE);
        tOut.write(reinterpret_cast<char*>(&key), REC_SIZE);
        tOut.close();
    }
    
    std::cout << "\n=== INVENTORY BY CATEGORY ===\n";
    char currentCat[MAX_LEN] = "";
    
    std::ifstream tView(TMP_FILE, std::ios::binary);
    Inventory viewItem{};
    while (tView.read(reinterpret_cast<char*>(&viewItem), REC_SIZE)) {
        if (_stricmp(currentCat, viewItem.category) != 0) {
            strcpy_s(currentCat, viewItem.category);
            std::cout << "\n>>> Category: " << currentCat << " <<<\n";
        }
        std::cout << "  - " << viewItem.item_name 
                  << " (x" << viewItem.quantity 
                  << ", " << viewItem.weight << "kg)\n";
    }
    tView.close();
    
    int maxCostIdx = -1, maxWeightIdx = -1;
    double maxTotalCost = -1, maxTotalWeight = -1;
    
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
            double totalCost = static_cast<double>(item.cost_per_unit) * item.quantity;
            double totalWeight = item.weight * item.quantity;
            
            if (totalCost > maxTotalCost) {
                maxTotalCost = totalCost;
                maxCostIdx = i;
            }
            if (totalWeight > maxTotalWeight) {
                maxTotalWeight = totalWeight;
                maxWeightIdx = i;
            }
        }
    }
    
    std::cout << "\n=== ANALYSIS ===\n";
    if (maxCostIdx != -1 && ReadRecordAt(maxCostIdx, item)) {
        std::cout << "Most expensive set: " << item.item_name 
                  << " | Total: " << maxTotalCost << " gold\n";
    }
    if (maxWeightIdx != -1 && ReadRecordAt(maxWeightIdx, item)) {
        std::cout << "Heaviest set: " << item.item_name 
                  << " | Total: " << maxTotalWeight << " kg\n";
    }
    
    remove(TMP_FILE);
    system("pause");
}

void GenerateReport() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        return;
    }
    
    std::ofstream txt("inventory_report.txt");
    if (!txt) {
        std::cout << "Error creating report!\n";
        system("pause");
        return;
    }
    
    txt << "===== INVENTORY REPORT =====\n";
    txt << "Generated: " << __DATE__ << " " << __TIME__ << "\n\n";
    
    Inventory item{};
    int totalItems = 0, totalWeight = 0, totalCost = 0;
    
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
            txt << "Item: " << item.item_name << "\n";
            txt << "  Category: " << item.category << "\n";
            txt << "  Quest: " << (item.quest ? "Yes" : "No") << "\n";
            txt << "  Cost/unit: " << item.cost_per_unit << " | Qty: " << item.quantity 
                << " | Total: " << item.Full_cost() << "\n";
            txt << "  Weight/unit: " << item.weight << " | Total weight: " 
                << (item.weight * item.quantity) << "\n\n";
            
            totalItems += item.quantity;
            totalWeight += static_cast<int>(item.weight * item.quantity);
            totalCost += item.Full_cost();
        }
    }
    
    txt << "===== SUMMARY =====\n";
    txt << "Total unique items: " << n << "\n";
    txt << "Total quantity: " << totalItems << "\n";
    txt << "Total weight: " << totalWeight << " kg\n";
    txt << "Total value: " << totalCost << " gold\n";
    txt.close();
    
    std::cout << "Report saved to 'inventory_report.txt'\n";
    system("pause");
}

void HelpWithOverload() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        return;
    }
    
    int worstIdx = -1;
    double worstRatio = 1e9;
    Inventory item{};
    
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item) && !item.quest && item.weight > 0.001) {
            double ratio = static_cast<double>(item.cost_per_unit) / item.weight;
            if (ratio < worstRatio) {
                worstRatio = ratio;
                worstIdx = i;
            }
        }
    }
    
    if (worstIdx == -1) {
        std::cout << "No suitable non-quest items found.\n";
    }
    else {
        ReadRecordAt(worstIdx, item);
        std::cout << "\nLeast cost-effective item (non-quest):\n";
        PrintItem(item);
        std::cout << "Cost/Weight ratio: " << worstRatio << " gold/kg\n";
        std::cout << "Consider dropping this item first!\n";
    }
    system("pause");
}

static int Menu() {
    const int MENU_SIZE = 13;
    const char* menu[MENU_SIZE] = {
        "Add Item",
        "View All Items",
        "Edit Item",
        "Delete Item",
        "Search by Name (linear)",
        "Search by Weight (binary)",
        "Sort by Weight (bubble)",
        "Sort by Quantity (selection)",
        "Sort by Name (insertion)",
        "Search: Weight Range + Category",
        "View by Category + Analysis",
        "Generate Text Report",
        "Exit"
    };
    
    int choice = 0;
    while (true) {
        system("cls");
        std::cout << "=== INVENTORY MANAGER ===\n";
        std::cout << "Records in file: " << GetRecordCount() << "\n\n";
        
        for (int i = 0; i < MENU_SIZE; i++) {
            if (i == choice) {
                setColor(14);
                std::cout << " > " << menu[i] << "\n";
                setColor(7);
            }
            else {
                std::cout << "   " << menu[i] << "\n";
            }
        }
        
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72) choice = (choice - 1 + MENU_SIZE) % MENU_SIZE;
            else if (key == 80) choice = (choice + 1) % MENU_SIZE;
        }
        else if (key == 13) {
            system("cls");
            return choice;
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    while (true) {
        int choice = Menu();
        switch (choice) {
            case 0: AddItemToFile(); break;
            case 1: PrintAllFromFile(); break;
            case 2: EditItem(); break;
            case 3: DeleteItem(); break;
            case 4: {
                if (std::cin.peek() == '\n') std::cin.ignore(1);
                char name[MAX_LEN];
                std::cout << "Search by name: ";
                std::cin.getline(name, MAX_LEN);
                int pos = LinearSearchInFile(name);
                if (pos == -1) std::cout << "Not found.\n";
                else {
                    Inventory item{};
                    ReadRecordAt(pos, item);
                    PrintItem(item);
                }
                system("pause");
                break;
            }
            case 5: {
                double target = ReadNumber("Search weight: ");
                int pos = BinarySearchInFile_ByWeight(target);
                if (pos == -1) std::cout << "Not found.\n";
                else {
                    Inventory item{};
                    ReadRecordAt(pos, item);
                    PrintItem(item);
                }
                system("pause");
                break;
            }
            case 6: SortFileByWeight_Bubble(); break;
            case 7: SortFileByQuantity_Selection(); break;
            case 8: SortFileByName_Insertion(); break;
            case 9: SearchByWeightRangeAndCategory(); break;
            case 10: ViewInventoryByCategory(); break;
            case 11: GenerateReport(); break;
            case 12: return 0;
        }
    }
}