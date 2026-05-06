#define NOMINMAX
#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <limits>
#include <iomanip>

#pragma pack(push, 1)
const int MAX_LEN = 100;

const double MAX_WEIGHT = 999.99;
const int MAX_QUANTITY = 9999;
const int MAX_COST_PER_UNIT = 99999;
const int MAX_TOTAL_COST = 999999;

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
const char* WORK_FILE = "inventory_work.bin";
const size_t REC_SIZE = sizeof(Inventory);

// ==================== ПРОТОТИПЫ ФУНКЦИЙ ====================
// Изменяет цвет текста в консоли Windows.
static void setColor(int color);
// Очищает буфер стандартного ввода до символа новой строки.
inline void FlushInput();
// Полностью сбрасывает состояние std::cin после ошибки ввода.
inline void ResetCin();
// Читает число double с проверкой диапазона и повторным запросом при ошибке.
double ReadNumberInRange(const char* prompt, double minVal, double maxVal, const char* errorMsg);
// Читает целое число int с проверкой диапазона и повторным запросом при ошибке.
int ReadIntInRange(const char* prompt, int minVal, int maxVal, const char* errorMsg);
// Читает строку, требуя непустого ввода без пробелов.
bool ReadRequiredString(const char* prompt, char* buffer, int size);
// Читает строку, разрешая пустой ввод.
void ReadOptionalString(const char* prompt, char* buffer, int size);
// Безопасно читает строку из стандартного ввода с ограничением размера.
void ReadLine(char* buffer, int size);
// Проверяет, содержит ли строка только пробелы или табуляции.
bool IsStringEmpty(const char* str);
// Возвращает количество записей в бинарном файле инвентаря.
int GetRecordCount();
// Последовательно читает файл до указанной позиции и возвращает запись.
bool ReadRecordAt(int index, Inventory& out);
// Заменяет запись по индексу через полную перезапись файла.
bool WriteRecordAt(int index, const Inventory& in);
// Меняет местами две записи в файле путём последовательного копирования.
void SwapRecords(int idx1, int idx2);
// Выводит детали одной записи в виде форматированной рамки.
void PrintItem(const Inventory& item);
// Выводит все записи файла в виде таблицы с заголовками.
void PrintItemTable();
// Выводит таблицу с подсветкой выбранной строки цветом.
void PrintItemTableWithSelection(int selectedIndex);
// Ищет запись по имени последовательным проходом по файлу.
int LinearSearchInFile(const char* name);
// Ищет запись по имени, игнорируя указанный индекс.
int LinearSearchExcludingIndex(const char* name, int excludeIndex);
// Ищет запись по весу бинарным поиском.
int BinarySearchInFile_ByWeight(double target);
// Увеличивает количество найденного предмета с проверкой лимита.
void UpdateQuantityByName(const char* name, int addQty);
// Добавляет новый предмет в конец файла после валидации ввода.
void AddItemToFile();
// Позволяет изменить поля выбранного предмета через интерактивное меню.
void EditItem();
// Удаляет выбранный предмет путём перезаписи файла без удалённой записи.
void DeleteItem();
// Выводит всё содержимое файла с заголовком и ожиданием нажатия.
void PrintAllFromFile();
// Сортирует файл по весу с помощью пузырьковой сортировки.
void SortFileByWeight_Bubble();
// Сортирует файл по количеству с помощью сортировки выбором.
void SortFileByQuantity_Selection();
// Сортирует файл по имени с помощью сортировки вставками.
void SortFileByName_Insertion();
// Универсальная сортировка файла с использованием компаратора и временных файлов.
void SortFile_Stream(bool (*comesBefore)(const Inventory&, const Inventory&));
// Добавляет одну запись в конец временного файла.
void WriteToTmpFile(const Inventory& item);
// Очищает временный файл, оставляя его пустым.
void ClearTmpFile();
// Выводит все записи из временного файла в детальном формате.
void PrintTmpFile();
// Выводит содержимое временного файла в табличном виде.
void PrintTmpFileTable();
// Возвращает количество записей во временном файле.
int GetTmpRecordCount();
// Сортирует временный файл по стоимости за единицу по убыванию.
void SortTmpFileByCost_Desc();
// Предоставляет меню выбора фильтра по категории.
int SelectCategoryForSearch(char* category, int size);
// Фильтрует предметы по диапазону веса и категории, сортирует результаты.
void SearchByWeightRangeAndCategory();
// Группирует инвентарь по категориям и анализирует самые дорогие/тяжёлые наборы.
void ViewInventoryByCategory();
// Генерирует текстовый отчёт о состоянии инвентаря в файл.
void GenerateReport();
// Находит наименее выгодный предмет по соотношению цены к весу.
void HelpWithOverload();
// Запрашивает имя и выводит найденный предмет или сообщение об отсутствии.
void SearchByName();
// Сортирует файл по весу и выполняет поиск по заданному значению.
void SearchByWeight();
// Отображает главное меню программы с навигацией стрелками.
int MainMenu();
// Отображает универсальное подменю с выбором стрелками и подтверждением Enter.
int SubMenu(const char* title, const char** options, int optionCount);
// Позволяет выбрать запись из таблицы навигацией стрелками.
int SelectItemFromTable(const char* title);

// ==================== MAIN ====================
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    while (true) {
        int choice = MainMenu();
        switch (choice) {
            case 0: AddItemToFile(); break;
            case 1: PrintAllFromFile(); break;
            case 2: EditItem(); break;
            case 3: DeleteItem(); break;
            case 4: SearchByName(); break;
            case 5: SearchByWeight(); break;
            case 6: SortFileByWeight_Bubble(); break;
            case 7: SortFileByQuantity_Selection(); break;
            case 8: SortFileByName_Insertion(); break;
            case 9: SearchByWeightRangeAndCategory(); break;
            case 10: ViewInventoryByCategory(); break;
            case 11: GenerateReport(); break;
            case 12: HelpWithOverload(); break;
            case 13: return 0;
        }
    }
}

// ==================== УТИЛИТЫ ВВОДА ====================
inline void FlushInput() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

inline void ResetCin() {
    std::cin.clear();
    FlushInput();
}

double ReadNumberInRange(const char* prompt, double minVal, double maxVal, const char* errorMsg) {
    double temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < minVal || temp > maxVal) {
                std::cout << errorMsg;
                std::cout << "Allowed range: " << minVal << " to " << maxVal << "\n";
                FlushInput();
                continue;
            }
            FlushInput();
            return temp;
        }
        else {
            std::cout << "Invalid value. Try again\n";
            ResetCin();
        }
    }
}

int ReadIntInRange(const char* prompt, int minVal, int maxVal, const char* errorMsg) {
    int temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < minVal || temp > maxVal) {
                std::cout << errorMsg;
                std::cout << "Allowed range: " << minVal << " to " << maxVal << "\n";
                FlushInput();
                continue;
            }
            FlushInput();
            return temp;
        }
        else {
            std::cout << "Invalid value. Try again\n";
            ResetCin();
        }
    }
}

bool ReadRequiredString(const char* prompt, char* buffer, int size) {
    while (true) {
        std::cout << prompt;
        ReadLine(buffer, size);
        if (!IsStringEmpty(buffer))
            return true;
        std::cout << "This field cannot be empty. Please try again.\n";
    }
}

void ReadOptionalString(const char* prompt, char* buffer, int size) {
    std::cout << prompt;
    ReadLine(buffer, size);
}

void ReadLine(char* buffer, int size) {
    std::cin.getline(buffer, size);
}

bool IsStringEmpty(const char* str) {
    if (str == nullptr) 
        return true;
    for (int i = 0; str[i] != '\0'; ++i)
        if (str[i] != ' ' && str[i] != '\t') 
            return false;
    return true;
}

static void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// ==================== ФАЙЛОВЫЕ ОПЕРАЦИИ ====================
int GetRecordCount() {
    std::ifstream file(FILENAME, std::ios::binary | std::ios::ate);
    if (!file) 
        return 0;
    return static_cast<int>(file.tellg() / REC_SIZE);
}

bool ReadRecordAt(int index, Inventory& out) {
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) 
        return false;
    Inventory temp{};
    int current = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (current == index) {
            out = temp;
            return true;
        }
        ++current;
    }
    return false;
}

bool WriteRecordAt(int index, const Inventory& in) {
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    if (!src || !tmp) 
        return false;
    Inventory temp{};
    int current = 0;
    bool found = false;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (current == index)
            found = true;
        else
            tmp.write(reinterpret_cast<const char*>(&temp), REC_SIZE);
        ++current;
    }
    if (found)
        tmp.write(reinterpret_cast<const char*>(&in), REC_SIZE);
    src.close(); tmp.close();
    remove(FILENAME);
    rename(TMP_FILE, FILENAME);
    return found;
}

void SwapRecords(int idx1, int idx2) {
    if (idx1 == idx2) 
        return;
    Inventory a{}, b{};
    if (!ReadRecordAt(idx1, a) || !ReadRecordAt(idx2, b)) 
        return;
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    if (!src || !tmp) 
        return;
    Inventory temp{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (index == idx1)
            tmp.write(reinterpret_cast<const char*>(&b), REC_SIZE);
        else if (index == idx2)
            tmp.write(reinterpret_cast<const char*>(&a), REC_SIZE);
        else
            tmp.write(reinterpret_cast<const char*>(&temp), REC_SIZE);
        ++index;
    }
    src.close(); tmp.close();
    remove(FILENAME);
    rename(TMP_FILE, FILENAME);
}

// ==================== ВЫВОД ====================
void PrintItem(const Inventory& item) {
    std::cout << "\n";
    std::cout << "+------------------------------------------+\n";
    std::cout << "|  INVENTORY ITEM DETAILS                  |\n";
    std::cout << "+------------------------------------------+\n";
    char nameDisplay[25];
    strncpy_s(nameDisplay, item.item_name, 24);
    nameDisplay[24] = '\0';
    std::cout << "|  Name:     " << std::left << std::setw(29) << nameDisplay << "|\n";
    std::cout << "|  Quest:    " << std::left << std::setw(29) << (item.quest ? "Yes" : "No") << "|\n";
    char costStr[12];
    snprintf(costStr, sizeof(costStr), "%d gold", item.cost_per_unit);
    std::cout << "|  Cost:     " << std::left << std::setw(29) << costStr << "|\n";
    char catDisplay[11];
    if (IsStringEmpty(item.category))
        strcpy_s(catDisplay, "(none)");
    else {
        strncpy_s(catDisplay, item.category, 10);
        catDisplay[10] = '\0';
    }
    std::cout << "|  Category: " << std::left << std::setw(29) << catDisplay << "|\n";
    char weightStr[10];
    snprintf(weightStr, sizeof(weightStr), "%.2f kg", item.weight);
    std::cout << "|  Weight:   " << std::left << std::setw(29) << weightStr << "|\n";
    std::cout << "|  Quantity: " << std::left << std::setw(29) << item.quantity << "|\n";
    char totalStr[12];
    snprintf(totalStr, sizeof(totalStr), "%d gold", item.Full_cost());
    std::cout << "|  Total:    " << std::left << std::setw(29) << totalStr << "|\n";
    std::cout << "+------------------------------------------+\n\n";
}

void PrintItemTable() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        return;
    }
    std::cout << "\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "| #  | Name                       | Category   | Quest    | Weight | Qty  | Total  |\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int i = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        char nameDisplay[25];
        char catDisplay[11];
        strncpy_s(nameDisplay, item.item_name, 24);
        nameDisplay[24] = '\0';
        if (IsStringEmpty(item.category))
            strcpy_s(catDisplay, "(none)");
        else {
            strncpy_s(catDisplay, item.category, 10);
            catDisplay[10] = '\0';
        }
        char weightStr[8];
        snprintf(weightStr, sizeof(weightStr), "%.2f", item.weight);
        std::cout << "| " << std::setw(2) << i << " | " 
                  << std::left << std::setw(26) << nameDisplay << " | "
                  << std::left << std::setw(10) << catDisplay << " | "
                  << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                  << std::setw(6) << weightStr << " | "
                  << std::setw(4) << item.quantity << " | "
                  << std::setw(6) << item.Full_cost() << " |\n";
        i++;
    }
    src.close();
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "Total items: " << n << "\n\n";
}

void PrintItemTableWithSelection(int selectedIndex) {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        return;
    }
    std::cout << "\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "| #  | Name                       | Category   | Quest    | Weight | Qty  | Total  |\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int i = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        char nameDisplay[25];
        char catDisplay[11];
        strncpy_s(nameDisplay, item.item_name, 24);
        nameDisplay[24] = '\0';
        if (IsStringEmpty(item.category))
            strcpy_s(catDisplay, "(none)");
        else {
            strncpy_s(catDisplay, item.category, 10);
            catDisplay[10] = '\0';
        }
        char weightStr[8];
        snprintf(weightStr, sizeof(weightStr), "%.2f", item.weight);
        if (i == selectedIndex) {
            setColor(14);
            std::cout << "| " << std::setw(2) << i << " | " 
                      << std::left << std::setw(26) << nameDisplay << " | "
                      << std::left << std::setw(10) << catDisplay << " | "
                      << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                      << std::setw(6) << weightStr << " | "
                      << std::setw(4) << item.quantity << " | "
                      << std::setw(6) << item.Full_cost() << " |\n";
            setColor(7);
        }
        else
            std::cout << "| " << std::setw(2) << i << " | " 
                      << std::left << std::setw(26) << nameDisplay << " | "
                      << std::left << std::setw(10) << catDisplay << " | "
                      << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                      << std::setw(6) << weightStr << " | "
                      << std::setw(4) << item.quantity << " | "
                      << std::setw(6) << item.Full_cost() << " |\n";
        i++;
    }
    src.close();
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "Total items: " << n << " | Use UP/DOWN arrows to select, ENTER to confirm\n\n";
}

// ==================== НАВИГАЦИЯ И МЕНЮ ====================
int SelectItemFromTable(const char* title) {
    int n = GetRecordCount();
    if (n == 0) return -1;
    int selectedIndex = 0;
    while (true) {
        system("cls");
        std::cout << "=== " << title << " ===\n";
        std::cout << "Records in file: " << n << "\n";
        PrintItemTableWithSelection(selectedIndex);
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72)
                selectedIndex = (selectedIndex - 1 + n) % n;
            else if (key == 80)
                selectedIndex = (selectedIndex + 1) % n;
        }
        else if (key == 13) {
            system("cls");
            return selectedIndex;
        }
        else if (key == 27) {
            system("cls");
            return -1;
        }
    }
}

int SubMenu(const char* title, const char** options, int optionCount) {
    int choice = 0;
    while (true) {
        system("cls");
        std::cout << "=== " << title << " ===\n";
        std::cout << "Records in file: " << GetRecordCount() << "\n\n";
        for (int i = 0; i < optionCount; i++) {
            if (i == choice) {
                setColor(14);
                std::cout << " > " << options[i] << "\n";
                setColor(7);
            }
            else
                std::cout << "   " << options[i] << "\n";
        }
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72) 
                choice = (choice - 1 + optionCount) % optionCount;
            else if (key == 80) 
                choice = (choice + 1) % optionCount;
        }
        else if (key == 13) {
            system("cls");
            return choice;
        }
    }
}

int MainMenu() {
    const int MENU_SIZE = 14;
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
        "Help With Overload",
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
            else
                std::cout << "   " << menu[i] << "\n";
        }
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72) 
                choice = (choice - 1 + MENU_SIZE) % MENU_SIZE;
            else if (key == 80) 
                choice = (choice + 1) % MENU_SIZE;
        }
        else if (key == 13) {
            system("cls");
            return choice;
        }
    }
}

int SelectCategoryForSearch(char* category, int size) {
    const int CAT_MENU_SIZE = 3;
    const char* catMenu[CAT_MENU_SIZE] = {
        "Search items WITH category",
        "Search items WITHOUT category (empty)",
        "Search ALL items (no filter)"
    };
    int choice = 0;
    while (true) {
        system("cls");
        std::cout << "=== CATEGORY FILTER ===\n\n";
        for (int i = 0; i < CAT_MENU_SIZE; i++) {
            if (i == choice) {
                setColor(14);
                std::cout << " > " << catMenu[i] << "\n";
                setColor(7);
            }
            else 
                std::cout << "   " << catMenu[i] << "\n";
        }
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72) 
                choice = (choice - 1 + CAT_MENU_SIZE) % CAT_MENU_SIZE;
            else if (key == 80) 
                choice = (choice + 1) % CAT_MENU_SIZE;
        }
        else if (key == 13) {
            system("cls");
            if (choice == 0) {
                std::cout << "Enter category name: ";
                ReadLine(category, size);
                if (IsStringEmpty(category)) {
                    memset(category, 0, size);
                    return 0;
                }
                return 1; 
            }
            else if (choice == 1) {
                memset(category, 0, size);
                return 0; 
            }
            else {
                memset(category, 0, size);
                return 2;
            }
        }
    }
}

// ==================== ПОИСК И СОРТИРОВКА ====================
int LinearSearchInFile(const char* name) {
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) 
        return -1;
    Inventory temp{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (_stricmp(temp.item_name, name) == 0)
            return index;
        ++index;
    }
    return -1;
}

int LinearSearchExcludingIndex(const char* name, int excludeIndex) {
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) 
        return -1;
    Inventory temp{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (index == excludeIndex) {
            ++index;
            continue;
        }
        if (_stricmp(temp.item_name, name) == 0)
            return index;
        ++index;
    }
    return -1;
}

int BinarySearchInFile_ByWeight(double target) {
    int n = GetRecordCount();
    if (n == 0) 
        return -1;
    int low = 0, high = n - 1;
    Inventory temp{};
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) 
        return -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        src.seekg(mid * REC_SIZE, std::ios::beg);
        src.read(reinterpret_cast<char*>(&temp), REC_SIZE);
        if (!src) 
            break;
        if (std::abs(temp.weight - target) < 0.001) {
            src.close();
            return mid;
        }
        else if (temp.weight < target)
            low = mid + 1;
        else
            high = mid - 1;
    }
    src.close();
    return -1;
}

void UpdateQuantityByName(const char* name, int addQty) {
    int pos = LinearSearchInFile(name);
    if (pos == -1) 
        return;
    Inventory item{};
    if (ReadRecordAt(pos, item)) {
        if (item.quantity + addQty > MAX_QUANTITY) {
            std::cout << "Cannot add: quantity would exceed maximum (" << MAX_QUANTITY << ")\n";
            return;
        }
        item.quantity += addQty;
        WriteRecordAt(pos, item);
        std::cout << "Updated quantity: +" << addQty << "\n";
    }
}

void AddItemToFile() {
    Inventory item{};
    memset(item.item_name, 0, MAX_LEN);
    memset(item.category, 0, MAX_LEN);
    std::cout << "\n=== ADD NEW ITEM ===\n\n";
    std::cout << "Max values: Weight=" << MAX_WEIGHT << ", Cost=" << MAX_COST_PER_UNIT << ", Qty=" << MAX_QUANTITY << "\n\n";
    while (true) {
        ReadRequiredString("Enter Item Name: ", item.item_name, MAX_LEN);
        if (LinearSearchInFile(item.item_name) != -1) {
            std::cout << "Item '" << item.item_name << "' already exists!\n";
            std::cout << "Add quantity to existing? (y/n): ";
            while (_kbhit()) _getch();
            if (_getch() == 'y') {
                int addQty = ReadIntInRange("Quantity to add: ", 1, MAX_QUANTITY, "Invalid quantity.\n");
                UpdateQuantityByName(item.item_name, addQty);
            }
            system("pause");
            system("cls");
            return;
        }
        break;
    }
    system("cls");
    std::cout << "\n=== ADD NEW ITEM ===\n";
    std::cout << "Name: " << item.item_name << "\n\n";
    char buff[MAX_LEN];
    while (true) {
        std::cout << "Quest item? (quest/not): ";
        ReadLine(buff, MAX_LEN);
        if (_stricmp(buff, "quest") == 0) { item.quest = true; break; }
        if (_stricmp(buff, "not") == 0) { item.quest = false; break; }
        std::cout << "Please enter 'quest' or 'not'\n";
    }
    system("cls");
    std::cout << "\n=== ADD NEW ITEM ===\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n\n";
    item.cost_per_unit = ReadIntInRange("Cost per unit: ", 1, MAX_COST_PER_UNIT, "Invalid cost.\n");
    system("cls");
    std::cout << "\n=== ADD NEW ITEM ===\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n\n";
    std::cout << "Category (press Enter to skip): ";
    ReadOptionalString("", item.category, MAX_LEN);
    system("cls");
    std::cout << "\n=== ADD NEW ITEM ===\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n";
    std::cout << "Category: " << (IsStringEmpty(item.category) ? "(none)" : item.category) << "\n\n";
    item.weight = ReadNumberInRange("Weight per unit: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
    system("cls");
    std::cout << "\n=== ADD NEW ITEM ===\n";
    std::cout << "Name: " << item.item_name << "\n";
    std::cout << "Quest: " << (item.quest ? "Yes" : "No") << "\n";
    std::cout << "Cost: " << item.cost_per_unit << "\n";
    std::cout << "Category: " << (IsStringEmpty(item.category) ? "(none)" : item.category) << "\n";
    std::cout << "Weight: " << std::fixed << std::setprecision(2) << item.weight << "\n\n";
    int maxQty = MAX_QUANTITY;
    if (item.cost_per_unit > 0) {
        int maxQtyByCost = MAX_TOTAL_COST / item.cost_per_unit;
        if (maxQtyByCost < maxQty) 
            maxQty = maxQtyByCost;
    }
    item.quantity = ReadIntInRange("Quantity: ", 1, maxQty, "Invalid quantity.\n");
    system("cls");
    std::ofstream file(FILENAME, std::ios::binary | std::ios::app);
    if (file) {
        file.write(reinterpret_cast<char*>(&item), REC_SIZE);
        std::cout << "\n=== ITEM ADDED SUCCESSFULLY ===\n\n";
        PrintItem(item);
    }
    else
        std::cout << "Error writing to file!\n";
    system("pause");
    system("cls");
}

void EditItem() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty. Nothing to edit.\n";
        system("pause");
        return;
    }
    int pos = SelectItemFromTable("SELECT ITEM TO EDIT");
    if (pos == -1) {
        std::cout << "Edit cancelled.\n";
        system("pause");
        return;
    }
    Inventory item{};
    ReadRecordAt(pos, item);
    const int EDIT_MENU_SIZE = 7;
    const char* editMenu[EDIT_MENU_SIZE] = {
        "Change Name",
        "Change Cost",
        "Change Weight",
        "Change Quantity",
        "Change Category",
        "Toggle Quest Flag",
        "Cancel"
    };
    int choice = SubMenu("EDIT ITEM", editMenu, EDIT_MENU_SIZE);
    bool modified = false;
    bool nameChanged = false;
    char newName[MAX_LEN];
    switch (choice) {
        case 0:
            while (true) {
                ReadRequiredString("New name: ", newName, MAX_LEN);
                if (LinearSearchExcludingIndex(newName, pos) != -1) {
                    std::cout << "Item with name '" << newName << "' already exists!\n";
                    continue;
                }
                strcpy_s(item.item_name, newName);
                nameChanged = true;
                modified = true;
                break;
            }
            break;
        case 1:
            item.cost_per_unit = ReadIntInRange("New cost: ", 1, MAX_COST_PER_UNIT, "Invalid cost.\n");
            modified = true;
            break;
        case 2:
            item.weight = ReadNumberInRange("New weight: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
            modified = true;
            break;
        case 3: {
            int maxQty = MAX_QUANTITY;
            if (item.cost_per_unit > 0) {
                int maxQtyByCost = MAX_TOTAL_COST / item.cost_per_unit;
                if (maxQtyByCost < maxQty) 
                    maxQty = maxQtyByCost;
            }
            item.quantity = ReadIntInRange("New quantity: ", 1, maxQty, "Invalid quantity.\n");
            modified = true;
            break;
        }
        case 4:
            std::cout << "New category (press Enter to clear): ";
            ReadOptionalString("", item.category, MAX_LEN);
            modified = true;
            break;
        case 5:
            item.quest = !item.quest;
            modified = true;
            break;
        case 6:
            std::cout << "Cancelled.\n";
            system("pause");
            return;
    }
    if (modified) {
        WriteRecordAt(pos, item);
        std::cout << "Saved!\n";
        if (nameChanged)
            std::cout << "Note: Item name was changed. Sort order may have changed.\n";
    }
    system("pause");
}

void DeleteItem() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty. Nothing to delete.\n";
        system("pause");
        return;
    }
    int pos = SelectItemFromTable("SELECT ITEM TO DELETE");
    if (pos == -1) {
        std::cout << "Delete cancelled.\n";
        system("pause");
        return;
    }
    Inventory item{};
    ReadRecordAt(pos, item);
    std::cout << "\n=== CONFIRM DELETE ===\n";
    PrintItem(item);
    std::cout << "Are you sure you want to delete this item? (y/n): ";
    while (_kbhit()) _getch();
    char confirm = _getch();
    std::cout << confirm << "\n\n";
    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "Delete cancelled.\n";
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
    std::cout << "\n=== INVENTORY ===\n";
    PrintItemTable();
    system("pause");
}

void SortFileByWeight_Bubble() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        return; 
    }
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            Inventory a{}, b{};
            if (!ReadRecordAt(j, a) || !ReadRecordAt(j + 1, b)) 
                continue;
            if (a.weight > b.weight) {
                SwapRecords(j, j + 1);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
    std::cout << "Sorted by weight!\n";
    system("pause");
}

void SearchByName() {
    char name[MAX_LEN];
    ReadRequiredString("Search by name: ", name, MAX_LEN);
    int pos = LinearSearchInFile(name);
    if (pos == -1) 
        std::cout << "Not found.\n";
    else {
        Inventory item{};
        ReadRecordAt(pos, item);
        PrintItem(item);
    }
    system("pause");
}

void SearchByWeight() {
    std::cout << "Binary search requires the data to be sorted by weight.\n";
    std::cout << "The file will now be sorted by weight.\n";
    std::cout << "The original order of records will be changed.\n\n";
    std::cout << "Do you want to continue? (y/n): ";
    while (_kbhit()) _getch();
    char confirm = _getch();
    std::cout << confirm << "\n\n";
    system("cls");
    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "Operation cancelled by user.\n";
        system("pause");
        return;
    }
    SortFileByWeight_Bubble();
    system("cls");
    double target = ReadNumberInRange("Enter weight to search: ", 0.01, MAX_WEIGHT, "Weight out of range.\n");
    int pos = BinarySearchInFile_ByWeight(target);
    system("cls");
    if (pos == -1)
        std::cout << "\nItem with weight " << std::fixed << std::setprecision(2) << target << " kg not found.\n";
    else {
        Inventory item{};
        ReadRecordAt(pos, item);
        std::cout << "\nFound:\n";
        PrintItem(item);
    }
    system("pause");
}

void SortFileByQuantity_Selection() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        return; 
    }
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        Inventory minItem{};
        if (!ReadRecordAt(i, minItem)) continue;
        
        for (int j = i + 1; j < n; ++j) {
            Inventory curr{};
            if (ReadRecordAt(j, curr)) {
                if (curr.quantity < minItem.quantity) {
                    minItem = curr;
                    minIdx = j;
                }
            }
        }
        if (minIdx != i) {
            SwapRecords(i, minIdx);
        }
    }
    std::cout << "Sorted by quantity (selection)!\n";
    PrintItemTable();
    system("pause");
}

void SortFileByName_Insertion() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        return; 
    }
    for (int i = 1; i < n; ++i) {
        Inventory key{};
        if (!ReadRecordAt(i, key)) continue;
        
        int j = i - 1;
        while (j >= 0) {
            Inventory curr{};
            if (!ReadRecordAt(j, curr)) break;
            
            if (_stricmp(curr.item_name, key.item_name) > 0) {
                WriteRecordAt(j + 1, curr);
                --j;
            }
            else {
                break;
            }
        }
        WriteRecordAt(j + 1, key);
    }
    std::cout << "Sorted by name (insertion)!\n";
    PrintItemTable();
    system("pause");
}

void SortFile_Stream(bool (*comesBefore)(const Inventory&, const Inventory&)) {
    const char* SORTED_FILE = TMP_FILE;
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) 
        return;
    std::ofstream initSorted(SORTED_FILE, std::ios::binary | std::ios::trunc);
    initSorted.close();
    Inventory item{};
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        std::ifstream sortedIn(SORTED_FILE, std::ios::binary);
        std::ofstream workOut(WORK_FILE, std::ios::binary | std::ios::trunc);
        if (!workOut) {
            sortedIn.close();
            break;
        }
        bool inserted = false;
        Inventory current{};
        while (sortedIn.read(reinterpret_cast<char*>(&current), REC_SIZE)) {
            if (!inserted && comesBefore(item, current)) {
                workOut.write(reinterpret_cast<const char*>(&item), REC_SIZE);
                inserted = true;
            }
            workOut.write(reinterpret_cast<const char*>(&current), REC_SIZE);
        }
        if (!inserted)
            workOut.write(reinterpret_cast<const char*>(&item), REC_SIZE);
        sortedIn.close();
        workOut.close();
        remove(SORTED_FILE);
        rename(WORK_FILE, SORTED_FILE);
    }
    src.close();
    remove(FILENAME);
    rename(SORTED_FILE, FILENAME);
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
    if (!tmp) 
        return;
    Inventory item{};
    while (tmp.read(reinterpret_cast<char*>(&item), REC_SIZE))
        PrintItem(item);
    tmp.close();
}

void PrintTmpFileTable() {
    std::ifstream tmp(TMP_FILE, std::ios::binary);
    if (!tmp) return;
    std::cout << "\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "| #  | Name                       | Category   | Quest    | Weight | Qty  | Total  |\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    Inventory item{};
    int i = 0;
    while (tmp.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        char nameDisplay[25];
        char catDisplay[11];
        strncpy_s(nameDisplay, item.item_name, 24);
        nameDisplay[24] = '\0';
        if (IsStringEmpty(item.category))
            strcpy_s(catDisplay, "(none)");
        else {
            strncpy_s(catDisplay, item.category, 10);
            catDisplay[10] = '\0';
        }
        char weightStr[8];
        snprintf(weightStr, sizeof(weightStr), "%.2f", item.weight);
        std::cout << "| " << std::setw(2) << i << " | " 
                  << std::left << std::setw(26) << nameDisplay << " | "
                  << std::left << std::setw(10) << catDisplay << " | "
                  << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                  << std::setw(6) << weightStr << " | "
                  << std::setw(4) << item.quantity << " | "
                  << std::setw(6) << item.Full_cost() << " |\n";
        i++;
    }
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "Total items: " << i << "\n\n";
    tmp.close();
}

int GetTmpRecordCount() {
    std::ifstream tmp(TMP_FILE, std::ios::binary | std::ios::ate);
    if (!tmp) 
        return 0;
    return static_cast<int>(tmp.tellg() / REC_SIZE);
}

void SortTmpFileByCost_Desc() {
    int n = GetTmpRecordCount();
    if (n <= 1) 
        return;
    for (int i = 0; i < n - 1; ++i) {
        Inventory maxRec{};
        int maxIdx = -1;
        std::ifstream tmp(TMP_FILE, std::ios::binary);
        Inventory item{};
        int idx = 0;
        bool found = false;
        while (tmp.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
            bool alreadyPlaced = false;
            std::ifstream placed(TMP_FILE, std::ios::binary);
            Inventory p{};
            int pIdx = 0;
            while (placed.read(reinterpret_cast<char*>(&p), REC_SIZE) && pIdx < i) {
                if (memcmp(&p, &item, REC_SIZE) == 0) {
                    alreadyPlaced = true;
                    break;
                }
                ++pIdx;
            }
            placed.close();
            if (!alreadyPlaced)
                if (!found || item.cost_per_unit > maxRec.cost_per_unit) {
                    maxRec = item;
                    maxIdx = idx;
                    found = true;
                }
            ++idx;
        }
        tmp.close();
        if (found && maxIdx >= i) {
            std::ifstream src(TMP_FILE, std::ios::binary);
            std::ofstream out(WORK_FILE, std::ios::binary);
            Inventory temp{};
            int cur = 0;
            while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
                if (cur < i)
                    out.write(reinterpret_cast<const char*>(&temp), REC_SIZE);
                else if (cur == i)
                    out.write(reinterpret_cast<const char*>(&maxRec), REC_SIZE);
                else if (cur != maxIdx)
                    out.write(reinterpret_cast<const char*>(&temp), REC_SIZE);
                ++cur;
            }
            src.close(); out.close();
            remove(TMP_FILE);
            rename(WORK_FILE, TMP_FILE);
        }
    }
}

void SearchByWeightRangeAndCategory() {
    ClearTmpFile();
    double low = ReadNumberInRange("Lower weight bound: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
    double high = ReadNumberInRange("Upper weight bound: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
    if (low > high) 
        std::swap(low, high);
    char category[MAX_LEN];
    int filterType = SelectCategoryForSearch(category, MAX_LEN);
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int matches = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        bool categoryMatch = false;
        if (filterType == 2)
            categoryMatch = true;
        else if (filterType == 0)
            categoryMatch = IsStringEmpty(item.category);
        else
            categoryMatch = (_stricmp(item.category, category) == 0);
        if (categoryMatch && item.weight >= low && item.weight <= high) {
            WriteToTmpFile(item);
            matches++;
        }
    }
    src.close();
    if (matches == 0) {
        std::cout << "No items found.\n";
        system("pause");
        return;
    }
    SortTmpFileByCost_Desc();
    std::cout << "\n=== RESULTS (sorted by cost/unit DESC) ===\n";
    std::cout << "Found: " << matches << " item(s)\n";
    if (filterType == 0)
        std::cout << "Filter: Items WITHOUT category\n";
    else if (filterType == 1)
        std::cout << "Filter: Category = " << category << "\n";
    else
        std::cout << "Filter: All categories\n";
    std::cout << "Weight range: " << std::fixed << std::setprecision(2) << low << " - " << high << " kg\n\n";
    PrintTmpFileTable();
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
        {
            std::ifstream tIn(TMP_FILE, std::ios::binary);
            tIn.seekg(i * REC_SIZE);
            tIn.read(reinterpret_cast<char*>(&key), REC_SIZE);
        }
        int j = i - 1;
        while (j >= 0) {
            Inventory curr{};
            {
                std::ifstream tIn2(TMP_FILE, std::ios::binary);
                tIn2.seekg(j * REC_SIZE);
                tIn2.read(reinterpret_cast<char*>(&curr), REC_SIZE);
            }
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
    bool firstItem = true;
    std::ifstream tView(TMP_FILE, std::ios::binary);
    Inventory viewItem{};
    while (tView.read(reinterpret_cast<char*>(&viewItem), REC_SIZE)) {
        bool currentEmpty = IsStringEmpty(currentCat);
        bool viewEmpty = IsStringEmpty(viewItem.category);
        bool categoryChanged = (currentEmpty != viewEmpty) || (!currentEmpty && !viewEmpty && _stricmp(currentCat, viewItem.category) != 0);
        if (categoryChanged || firstItem) {
            strcpy_s(currentCat, viewItem.category);
            if (viewEmpty)
                std::cout << "\n>>> Category: (none/empty) <<<\n";
            else
                std::cout << "\n>>> Category: " << currentCat << " <<<\n";
            firstItem = false;
        }
        std::cout << " - " << viewItem.item_name << " (x" << viewItem.quantity 
                  << ", " << std::fixed << std::setprecision(2) << viewItem.weight << "kg)\n";
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
    if (maxCostIdx != -1 && ReadRecordAt(maxCostIdx, item))
        std::cout << "Most expensive set: " << item.item_name << " | Total: " << maxTotalCost << " gold\n";
    if (maxWeightIdx != -1 && ReadRecordAt(maxWeightIdx, item))
        std::cout << "Heaviest set: " << item.item_name << " | Total: " << maxTotalWeight << " kg\n";
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
    std::ifstream src(FILENAME, std::ios::binary);
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        txt << "Item: " << item.item_name << "\n";
        txt << " Category: " << (IsStringEmpty(item.category) ? "(none)" : item.category) << "\n";
        txt << " Quest: " << (item.quest ? "Yes" : "No") << "\n";
        txt << " Cost/unit: " << item.cost_per_unit << " | Qty: " << item.quantity 
            << " | Total: " << item.Full_cost() << "\n";
        txt << " Weight/unit: " << std::fixed << std::setprecision(2) << item.weight 
            << " | Total weight: " << (item.weight * item.quantity) << "\n\n";
        totalItems += item.quantity;
        totalWeight += static_cast<int>(item.weight * item.quantity);
        totalCost += item.Full_cost();
    }
    src.close();
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
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        if (!item.quest && item.weight > 0.001) {
            double ratio = static_cast<double>(item.cost_per_unit) / item.weight;
            if (ratio < worstRatio) {
                worstRatio = ratio;
                worstIdx = index;
            }
        }
        ++index;
    }
    src.close();
    if (worstIdx == -1)
        std::cout << "No suitable non-quest items found.\n";
    else {
        Inventory worstItem{};
        ReadRecordAt(worstIdx, worstItem);
        std::cout << "\n=== LEAST COST-EFFECTIVE ITEM (NON-QUEST) ===\n";
        PrintItem(worstItem);
        std::cout << "Cost/Weight ratio: " << worstRatio << " gold/kg\n";
        std::cout << "Consider dropping this item first!\n";
    }
    system("pause");
}