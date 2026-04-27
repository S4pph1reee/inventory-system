#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <limits>
#include <iomanip>

// ============================================================================
// КОНСТАНТЫ И СТРУКТУРЫ
// ============================================================================

#pragma pack(push, 1)  // Упаковка структуры без выравнивания (для бинарного файла)
const int MAX_LEN = 100;  // Максимальная длина строковых полей

// ОГРАНИЧЕНИЯ НА ЧИСЛОВЫЕ ЗНАЧЕНИЯ
const double MAX_WEIGHT = 999.99;      // 6 символов
const int MAX_QUANTITY = 9999;         // 4 символа
const int MAX_COST_PER_UNIT = 99999;   // 5 символов
const int MAX_TOTAL_COST = 999999;     // 6 символов

// Структура предмета инвентаря
#pragma pack(push, 1)
struct Inventory {
    char item_name[MAX_LEN];    // Название предмета
    bool quest;                 // Флаг квестового предмета
    int cost_per_unit;          // Стоимость за единицу
    char category[MAX_LEN];     // Категория (может быть пустой)
    double weight;              // Вес за единицу
    int quantity;               // Количество
    int Full_cost() const { return cost_per_unit * quantity; }  // Общая стоимость
};
#pragma pack(pop)

const char* FILENAME = "inventory.bin";       // Основной файл данных
const char* TMP_FILE = "inventory_tmp.bin";   // Временный файл для операций
const size_t REC_SIZE = sizeof(Inventory);    // Размер одной записи в байтах

// ============================================================================
// ПРОТОТИПЫ ФУНКЦИЙ
// ============================================================================

// Устанавливает цвет текста консоли
static void setColor(int color);

// Читает число double с проверкой диапазона
double ReadNumberInRange(const char* prompt, double minVal, double maxVal, const char* errorMsg);

// Читает целое число int с проверкой диапазона
int ReadIntInRange(const char* prompt, int minVal, int maxVal, const char* errorMsg);

// Читает строку, обязательно требуя непустое значение
bool ReadRequiredString(const char* prompt, char* buffer, int size);

// Читает строку, разрешая пустое значение
void ReadOptionalString(const char* prompt, char* buffer, int size);

// Читает строку с консоли
void ReadLine(char* buffer, int size);

// Проверяет, является ли строка пустой или содержащей только пробелы
bool IsStringEmpty(const char* str);

// Возвращает количество записей в файле инвентаря
int GetRecordCount();

// Читает запись по индексу из бинарного файла
bool ReadRecordAt(int index, Inventory& out);

// Записывает запись по индексу в бинарный файл
bool WriteRecordAt(int index, const Inventory& in);

// Меняет местами две записи в файле по их индексам
void SwapRecords(int idx1, int idx2);

// Выводит свойства предмета в виде форматированной таблицы
void PrintItem(const Inventory& item);

// Выводит все предметы инвентаря в виде таблицы
void PrintItemTable();

// Выводит таблицу предметов с выделением выбранного элемента цветом
void PrintItemTableWithSelection(int selectedIndex);

// Линейный поиск предмета по имени (регистронезависимый)
int LinearSearchInFile(const char* name);

// Линейный поиск по имени, исключая запись с указанным индексом
// Используется при редактировании имени (чтобы не находить саму себя)
int LinearSearchExcludingIndex(const char* name, int excludeIndex);

// Бинарный поиск по весу
int BinarySearchInFile_ByWeight(double target);

// Обновляет количество предмета по имени
void UpdateQuantityByName(const char* name, int addQty);

// Добавляет новый предмет в файл
void AddItemToFile();

// Редактирует существующий предмет через таблицу выбора
void EditItem();

// Удаляет предмет после подтверждения через таблицу выбора
void DeleteItem();

// Выводит весь инвентарь в виде таблицы
// Обёртка над PrintItemTable() с заголовком
void PrintAllFromFile();

// Пузырьковая сортировка файла по весу
void SortFileByWeight_Bubble();

// Сортировка файла выборкой по количеству
void SortFileByQuantity_Selection();

// Сортировка файла вставками по имени
void SortFileByName_Insertion();

// Потоковая сортировка файла по заданному компаратору
void SortFile_Stream(bool (*comesBefore)(const Inventory&, const Inventory&));

// Записывает предмет во временный файл
// Используется для поиска и фильтрации
void WriteToTmpFile(const Inventory& item);

// Очищает временный файл
void ClearTmpFile();

// Выводит все предметы из временного файла
void PrintTmpFile();

// Выводит все предметы из временного файла в виде таблицы
void PrintTmpFileTable();

// Возвращает количество записей во временном файле
int GetTmpRecordCount();

// Сортирует временный файл по стоимости за единицу (по убыванию)
void SortTmpFileByCost_Desc();

// Меню выбора фильтра категории для поиска
int SelectCategoryForSearch(char* category, int size);

// Поиск по диапазону веса и категории с фильтрацией
// Результаты сохраняются во временный файл и сортируются
void SearchByWeightRangeAndCategory();

// Просмотр инвентаря с группировкой по категориям
// Включает анализ: самый дорогой и самый тяжёлый набор
void ViewInventoryByCategory();

// Генерирует текстовый отчёт в файл inventory_report.txt
void GenerateReport();

// Находит наименее выгодный предмет по соотношению цена/вес.(Исключает квестовые предметы)
void HelpWithOverload();

// Поиск по имени
void SearchByName();

// Поиск по весу с предварительной сортировкой и подтверждением
void SearchByWeight();

// Главное меню программы с навигацией стрелками
int MainMenu();

// Универсальное подменю (используется для Edit, Delete, Category)
// title - заголовок, options - массив пунктов, optionCount - количество
int SubMenu(const char* title, const char** options, int optionCount);

// Выбор предмета из таблицы с навигацией стрелками
int SelectItemFromTable(const char* title);

// ============================================================================
// MAIN
// ============================================================================

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

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ
// ============================================================================

static void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

double ReadNumberInRange(const char* prompt, double minVal, double maxVal, const char* errorMsg) {
    double temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < minVal || temp > maxVal) {
                std::cout << errorMsg;
                std::cout << "Allowed range: " << minVal << " to " << maxVal << "\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return temp;
        }
        else {
            std::cout << "Invalid value. Try again\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return temp;
        }
        else {
            std::cout << "Invalid value. Try again\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    if (str == nullptr) return true;
    for (int i = 0; str[i] != '\0'; ++i)
        if (str[i] != ' ' && str[i] != '\t') return false;
    return true;
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
    Inventory item{};
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
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
        }
    }
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
    Inventory item{};
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, item)) {
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
        }
    }
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "Total items: " << n << " | Use UP/DOWN arrows to select, ENTER to confirm\n\n";
}

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
            if (key == 72) // Up arrow
                selectedIndex = (selectedIndex - 1 + n) % n;
            else if (key == 80) // Down arrow
                selectedIndex = (selectedIndex + 1) % n;
        }
        else if (key == 13) {  // Enter
            system("cls");
            return selectedIndex;
        }
        else if (key == 27) {  // Escape
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
            if (key == 72) choice = (choice - 1 + optionCount) % optionCount;
            else if (key == 80) choice = (choice + 1) % optionCount;
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
            if (key == 72) choice = (choice - 1 + MENU_SIZE) % MENU_SIZE;
            else if (key == 80) choice = (choice + 1) % MENU_SIZE;
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
            if (key == 72) choice = (choice - 1 + CAT_MENU_SIZE) % CAT_MENU_SIZE;
            else if (key == 80) choice = (choice + 1) % CAT_MENU_SIZE;
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

int LinearSearchInFile(const char* name) {
    int n = GetRecordCount();
    Inventory temp{};
    for (int i = 0; i < n; ++i)
        if (ReadRecordAt(i, temp) && _stricmp(temp.item_name, name) == 0)
            return i;
    return -1;
}

int LinearSearchExcludingIndex(const char* name, int excludeIndex) {
    int n = GetRecordCount();
    Inventory temp{};
    for (int i = 0; i < n; ++i) {
        if (i == excludeIndex) continue;
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
        if (_stricmp(buff, "quest") == 0) item.quest = true; break;
        if (_stricmp(buff, "not") == 0) item.quest = false; break;
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
        if (maxQtyByCost < maxQty) maxQty = maxQtyByCost;
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
                if (maxQtyByCost < maxQty) maxQty = maxQtyByCost;
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
    SortFile_Stream([](const Inventory& a, const Inventory& b) {
        return a.weight < b.weight;
    });
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
    SortFile_Stream([](const Inventory& a, const Inventory& b) {
        return a.quantity < b.quantity;
    });
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
    SortFile_Stream([](const Inventory& a, const Inventory& b) {
        return _stricmp(a.item_name, b.item_name) < 0;
    });
    std::cout << "Sorted by name (insertion)!\n";
    PrintItemTable();
    system("pause");
}

void SortFile_Stream(bool (*comesBefore)(const Inventory&, const Inventory&)) {
    const char* SORTED_FILE = TMP_FILE;
    const char* WORK_FILE = "inventory_work.bin";

    std::ifstream src(FILENAME, std::ios::binary);
    if (!src) return;

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
    if (!tmp) return;
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
    if (!tmp) return 0;
    return static_cast<int>(tmp.tellg() / REC_SIZE);
}

void SortTmpFileByCost_Desc() {
    int n = GetTmpRecordCount();
    if (n <= 1) return;
    for (int i = 0; i < n - 1; ++i)
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

void SearchByWeightRangeAndCategory() {
    ClearTmpFile();
    double low = ReadNumberInRange("Lower weight bound: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
    double high = ReadNumberInRange("Upper weight bound: ", 0.01, MAX_WEIGHT, "Invalid weight.\n");
    if (low > high) std::swap(low, high);
    char category[MAX_LEN];
    int filterType = SelectCategoryForSearch(category, MAX_LEN);
    int n = GetRecordCount();
    Inventory item{};
    int matches = 0;
    for (int i = 0; i < n; ++i)
        if (ReadRecordAt(i, item)) {
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
        std::cout << " - " << viewItem.item_name << " (x" << viewItem.quantity << ", " << std::fixed << std::setprecision(2) << viewItem.weight << "kg)\n";
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
    for (int i = 0; i < n; ++i)
        if (ReadRecordAt(i, item)) {
            txt << "Item: " << item.item_name << "\n";
            txt << " Category: " << (IsStringEmpty(item.category) ? "(none)" : item.category) << "\n";
            txt << " Quest: " << (item.quest ? "Yes" : "No") << "\n";
            txt << " Cost/unit: " << item.cost_per_unit << " | Qty: " << item.quantity << " | Total: " << item.Full_cost() << "\n";
            txt << " Weight/unit: " << std::fixed << std::setprecision(2) << item.weight << " | Total weight: " << (item.weight * item.quantity) << "\n\n";
            totalItems += item.quantity;
            totalWeight += static_cast<int>(item.weight * item.quantity);
            totalCost += item.Full_cost();
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
    for (int i = 0; i < n; ++i)
        if (ReadRecordAt(i, item) && !item.quest && item.weight > 0.001) {
            double ratio = static_cast<double>(item.cost_per_unit) / item.weight;
            if (ratio < worstRatio) {
                worstRatio = ratio;
                worstIdx = i;
            }
        }
    if (worstIdx == -1)
        std::cout << "No suitable non-quest items found.\n";
    else {
        ReadRecordAt(worstIdx, item);
        std::cout << "\n=== LEAST COST-EFFECTIVE ITEM (NON-QUEST) ===\n";
        PrintItem(item);
        std::cout << "Cost/Weight ratio: " << worstRatio << " gold/kg\n";
        std::cout << "Consider dropping this item first!\n";
    }
    system("pause");
}