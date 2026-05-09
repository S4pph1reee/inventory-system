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

// Контекстные структуры для трансформеров и предикатов
struct NameSkipCtx { 
    const char* name; 
    int skip_index; 
};
struct UpdateCtx { 
    int target_index; 
    Inventory new_data; 
};
struct DeleteCtx { 
    int target_index; 
};
struct Swap_Context { 
    int i1, i2; 
    Inventory r1, r2; 
};

// ==================== ПРОТОТИПЫ ФУНКЦИЙ ====================
// Изменяет цвет текста в консоли Windows.
static void setColor(int color);
// Очищает буфер стандартного ввода до символа новой строки.
inline void FlushInput();
// Полностью сбрасывает состояние std::cin после ошибки ввода.
inline void ResetCin();
// Читает число double с проверкой диапазона и повторным запросом при ошибке.
double ReadNumberInRange(const char* prompt, double min_value, double max_value, const char* error_message);
// Читает целое число int с проверкой диапазона и повторным запросом при ошибке.
int ReadIntInRange(const char* prompt, int min_value, int max_value, const char* error_message);
// Читает строку, требуя непустого ввода без пробелов.
bool ReadRequiredString(const char* prompt, char* buffer, int size);
// Читает строку, разрешая пустой ввод.
void ReadOptionalString(const char* prompt, char* buffer, int size);
// Безопасно читает строку из стандартного ввода с ограничением размера.
void ReadLine(char* buffer, int size);
// Проверяет, содержит ли строка только пробелы или табуляции.
bool IsStringEmpty(const char* str);
// Возвращает количество записей в бинарном файле инвентаря.
int GetRecordCount(const char* filename = FILENAME);
// Выполняет быстрый поиск записи по индексу через смещение указателя файла.
bool ReadRecordAt(int index, Inventory& out);
// Выполняет универсальный линейный поиск по файлу с использованием функции-предиката.
int FindRecord(int (*pred)(const Inventory*, int, const void*), const void* context);
// Выполняет атомарную перезапись файла через временный с применением функции-трансформера.
bool AtomicRewrite(int (*trans)(const Inventory*, Inventory*, int, const void*), const void* context);
// Меняет местами две записи в файле путём перезаписи через трансформер.
void SwapRecords(int index1, int index2);
// Предикат: проверяет совпадение имени записи с заданной строкой.
int PredByName(const Inventory* rec, int index, const void* context);
// Предикат: проверяет совпадение имени, игнорируя запись по заданному индексу.
int PredByNameSkip(const Inventory* rec, int index, const void* context);
// Трансформер: заменяет запись по заданному индексу на новые данные.
int TransUpdate(const Inventory* in, Inventory* out, int index, const void* context);
// Трансформер: пропускает запись по заданному индексу (удаление).
int TransDelete(const Inventory* in, Inventory* out, int index, const void* context);
// Трансформер: меняет местами две записи по указанным индексам.
int TransSwap(const Inventory* in, Inventory* out, int index, const void* context);
// Ищет запись по имени последовательным проходом по файлу.
int LinearSearchInFile(const char* name);
// Ищет запись по имени, игнорируя указанный индекс.
int LinearSearchExcludingIndex(const char* name, int exclude_index);
// Ищет запись по весу бинарным поиском.
int BinarySearchInFile_ByWeight(double target);
// Увеличивает количество найденного предмета с проверкой лимита.
void UpdateQuantityByName(const char* name, int add_quantity);
// Добавляет новый предмет в конец файла после валидации ввода.
void AddItemToFile();
// Позволяет изменить поля выбранного предмета через интерактивное меню.
void EditItem();
// Удаляет выбранный предмет путём перезаписи файла без удалённой записи.
void DeleteItem();
// Выводит детали одной записи в виде форматированной рамки.
void PrintItem(const Inventory& item);
// Выводит все записи файла в виде таблицы с заголовками.
void PrintItemTable(const char* filename = FILENAME, int selected_index = -1);
// Выводит всё содержимое файла с заголовком и ожиданием нажатия.
void PrintAllFromFile();
// Сортирует файл по весу с помощью пузырьковой сортировки.
void SortFileByWeight_Bubble();
// Сортирует файл по количеству с помощью сортировки выбором.
void SortFileByQuantity_Selection();
// Сортирует файл по имени с помощью сортировки вставками.
void SortFileByName_Insertion();
// Заменяет запись по индексу через атомарную перезапись файла.
bool WriteRecordAt(int index, const Inventory& in);
// Добавляет одну запись в конец временного файла.
void WriteToTmpFile(const Inventory& item);
// Очищает временный файл, оставляя его пустым.
void ClearTmpFile();
// Выводит все записи из временного файла в детальном формате.
void PrintTmpFile();
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
int SubMenu(const char* title, const char** options, int option_count);
// Позволяет выбрать запись из таблицы навигацией стрелками.
int SelectItemFromTable(const char* title);
// Универсальная навигация по списку опций с помощью стрелок.
int NavigateList(const char* title, const char** options, int count);

// ==================== MAIN ====================
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    while (true) {
        int choice = MainMenu();
        if (choice == 13) return 0;
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
        }
        system("cls");
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

double ReadNumberInRange(const char* prompt, double min_value, double max_value, const char* error_message) {
    double temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < min_value || temp > max_value) {
                std::cout << error_message;
                std::cout << "Allowed range: " << min_value << " to " << max_value << "\n";
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

int ReadIntInRange(const char* prompt, int min_value, int max_value, const char* error_message) {
    int temp;
    while (true) {
        std::cout << prompt;
        if (std::cin >> temp) {
            if (temp < min_value || temp > max_value) {
                std::cout << error_message;
                std::cout << "Allowed range: " << min_value << " to " << max_value << "\n";
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

// ==================== ОПЕРАЦИИ ДОСТУПА И МОДИФИКАЦИИ ЗАПИСЕЙ ====================

bool ReadRecordAt(int index, Inventory& out) {
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src)
        return false;
    src.seekg(index * REC_SIZE, std::ios::beg);
    return src.read(reinterpret_cast<char*>(&out), REC_SIZE).good();
}

int FindRecord(int (*pred)(const Inventory*, int, const void*), const void* context) {
    std::ifstream src(FILENAME, std::ios::binary);
    if (!src)
        return -1;
    Inventory temp{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (pred(&temp, index, context)) {
            src.close();
            return index;
        }
        ++index;
    }
    return -1;
}

bool AtomicRewrite(int (*trans)(const Inventory*, Inventory*, int, const void*), const void* context) {
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    if (!src || !tmp) {
        if (src) 
            src.close();
        if (tmp) 
            tmp.close();
        return false;
    }
    Inventory in{}, out{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&in), REC_SIZE)) {
        if (trans(&in, &out, index, context))
            tmp.write(reinterpret_cast<char*>(&out), REC_SIZE);
        ++index;
    }
    src.close();
    tmp.close();
    remove(FILENAME);
    return rename(TMP_FILE, FILENAME) == 0;
}

int PredByName(const Inventory* rec, int index, const void* context) {
    return (_stricmp(rec->item_name, (const char*)context) == 0) ? 1 : 0;
}

int PredByNameSkip(const Inventory* rec, int index, const void* context) {
    const NameSkipCtx* p = (const NameSkipCtx*)context;
    if (index == p->skip_index)
        return 0;
    return (_stricmp(rec->item_name, p->name) == 0) ? 1 : 0;
}

int TransUpdate(const Inventory* in, Inventory* out, int index, const void* context) {
    const UpdateCtx* p = (const UpdateCtx*)context;
    if (index == p->target_index) {
        *out = p->new_data;
        return 1;
    }
    *out = *in;
    return 1;
}

int TransDelete(const Inventory* in, Inventory* out, int index, const void* context) {
    if (index == ((const DeleteCtx*)context)->target_index)
        return 0;
    *out = *in;
    return 1;
}

int TransSwap(const Inventory* in, Inventory* out, int index, const void* context) {
    const Swap_Context* p = (const Swap_Context*)context;
    if (index == p->i1) {
        *out = p->r2;
        return 1;
    }
    if (index == p->i2) {
        *out = p->r1;
        return 1;
    }
    *out = *in;
    return 1;
}

// ==================== ФАЙЛОВЫЕ ОПЕРАЦИИ ====================
int GetRecordCount(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) 
        return 0;
    return static_cast<int>(file.tellg() / REC_SIZE);
}

int LinearSearchInFile(const char* name) {
    return FindRecord(PredByName, name);
}

int LinearSearchExcludingIndex(const char* name, int exclude_index) {
    NameSkipCtx context = { name, exclude_index };
    return FindRecord(PredByNameSkip, &context);
}

bool WriteRecordAt(int index, const Inventory& in) {
    UpdateCtx context = { index, in };
    return AtomicRewrite(TransUpdate, &context);
}

void SwapRecords(int index1, int index2) {
    if (index1 == index2)
        return;
    Swap_Context context;
    if (!ReadRecordAt(index1, context.r1) || !ReadRecordAt(index2, context.r2))
        return;
    context.i1 = index1;
    context.i2 = index2;
    AtomicRewrite(TransSwap, &context);
}

// ==================== ВЫВОД ====================
void PrintItem(const Inventory& item) {
    std::cout << "\n";
    std::cout << "+------------------------------------------+\n";
    std::cout << "|  INVENTORY ITEM DETAILS                  |\n";
    std::cout << "+------------------------------------------+\n";
    char name_display[25];
    strncpy_s(name_display, item.item_name, 24);
    name_display[24] = '\0';
    std::cout << "|  Name:     " << std::left << std::setw(29) << name_display << "|\n";
    std::cout << "|  Quest:    " << std::left << std::setw(29) << (item.quest ? "Yes" : "No") << "|\n";
    char cost_str[12];
    snprintf(cost_str, sizeof(cost_str), "%d gold", item.cost_per_unit);
    std::cout << "|  Cost:     " << std::left << std::setw(29) << cost_str << "|\n";
    char cat_display[11];
    if (IsStringEmpty(item.category))
        strcpy_s(cat_display, "(none)");
    else {
        strncpy_s(cat_display, item.category, 10);
        cat_display[10] = '\0';
    }
    std::cout << "|  Category: " << std::left << std::setw(29) << cat_display << "|\n";
    char weight_str[10];
    snprintf(weight_str, sizeof(weight_str), "%.2f kg", item.weight);
    std::cout << "|  Weight:   " << std::left << std::setw(29) << weight_str << "|\n";
    std::cout << "|  Quantity: " << std::left << std::setw(29) << item.quantity << "|\n";
    char total_str[12];
    snprintf(total_str, sizeof(total_str), "%d gold", item.Full_cost());
    std::cout << "|  Total:    " << std::left << std::setw(29) << total_str << "|\n";
    std::cout << "+------------------------------------------+\n\n";
}

void PrintItemTable(const char* filename, int selected_index) {
    int n = GetRecordCount(filename);
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        return;
    }
    std::cout << "\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::cout << "| #  | Name                       | Category   | Quest    | Weight | Qty  | Total  |\n";
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    std::ifstream src(filename, std::ios::binary);
    Inventory item{};
    int i = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        char name_display[25];
        char cat_display[11];
        strncpy_s(name_display, item.item_name, 24);
        name_display[24] = '\0';
        if (IsStringEmpty(item.category))
            strcpy_s(cat_display, "(none)");
        else {
            strncpy_s(cat_display, item.category, 10);
            cat_display[10] = '\0';
        }
        char weight_str[8];
        snprintf(weight_str, sizeof(weight_str), "%.2f", item.weight);
        if (selected_index >= 0 && i == selected_index) {
            setColor(14);
            std::cout << "| " << std::setw(2) << i << " | " 
                      << std::left << std::setw(26) << name_display << " | "
                      << std::left << std::setw(10) << cat_display << " | "
                      << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                      << std::setw(6) << weight_str << " | "
                      << std::setw(4) << item.quantity << " | "
                      << std::setw(6) << item.Full_cost() << " |\n";
            setColor(7);
        }
        else {
            std::cout << "| " << std::setw(2) << i << " | " 
                      << std::left << std::setw(26) << name_display << " | "
                      << std::left << std::setw(10) << cat_display << " | "
                      << std::left << std::setw(8) << (item.quest ? "Yes" : "No") << " | "
                      << std::setw(6) << weight_str << " | "
                      << std::setw(4) << item.quantity << " | "
                      << std::setw(6) << item.Full_cost() << " |\n";
        }
        i++;
    }
    src.close();
    std::cout << "+----+----------------------------+------------+----------+--------+------+--------+\n";
    if (selected_index >= 0)
        std::cout << "Total items: " << n << " | Use UP/DOWN arrows to select, ENTER to confirm\n\n";
    else
        std::cout << "Total items: " << n << "\n\n";
}

// ==================== НАВИГАЦИЯ И МЕНЮ ====================
int NavigateList(const char* title, const char** options, int count) {
    int choice = 0;
    while (true) {
        system("cls");
        if (title) {
            std::cout << "=== " << title << " ===\n";
            std::cout << "Records in file: " << GetRecordCount() << "\n\n";
        }
        for (int i = 0; i < count; i++) {
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
                choice = (choice - 1 + count) % count;
            else if (key == 80) 
                choice = (choice + 1) % count;
        }
        else if (key == 13) {
            system("cls");
            return choice;
        }
    }
}

int SelectItemFromTable(const char* title) {
    int n = GetRecordCount();
    if (n == 0) 
        return -1;
    int selected_index = 0;
    while (true) {
        system("cls");
        std::cout << "=== " << title << " ===\n";
        std::cout << "Records in file: " << n << "\n";
        PrintItemTable(FILENAME, selected_index);
        int key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == 72)
                selected_index = (selected_index - 1 + n) % n;
            else if (key == 80)
                selected_index = (selected_index + 1) % n;
        }
        else if (key == 13) {
            system("cls");
            return selected_index;
        }
        else if (key == 27) {
            system("cls");
            return -1;
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
    return NavigateList("INVENTORY MANAGER", menu, MENU_SIZE);
}

int SubMenu(const char* title, const char** options, int option_count) {
    return NavigateList(title, options, option_count);
}

int SelectCategoryForSearch(char* category, int size) {
    const int CAT_MENU_SIZE = 3;
    const char* cat_menu[CAT_MENU_SIZE] = {
        "Search items WITH category",
        "Search items WITHOUT category (empty)",
        "Search ALL items (no filter)"
    };
    while (true) {
        int choice = NavigateList("CATEGORY FILTER", cat_menu, CAT_MENU_SIZE);
        if (choice == 0) {
            std::cout << "Enter category name: ";
            ReadLine(category, size);
            if (IsStringEmpty(category)) {
                memset(category, 0, size);
                system("cls");
                return 0;
            }
            system("cls");
            return 1; 
        }
        else if (choice == 1) {
            memset(category, 0, size);
            system("cls");
            return 0; 
        }
        else {
            memset(category, 0, size);
            system("cls");
            return 2;
        }
    }
}

// ==================== ПОИСК И СОРТИРОВКА ====================
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

void UpdateQuantityByName(const char* name, int add_quantity) {
    int pos = LinearSearchInFile(name);
    if (pos == -1) 
        return;
    Inventory item{};
    if (ReadRecordAt(pos, item)) {
        if (item.quantity + add_quantity > MAX_QUANTITY) {
            std::cout << "Cannot add: quantity would exceed maximum (" << MAX_QUANTITY << ")\n";
            return;
        }
        item.quantity += add_quantity;
        WriteRecordAt(pos, item);
        std::cout << "Updated quantity: +" << add_quantity << "\n";
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
                int add_quantity = ReadIntInRange("Quantity to add: ", 1, MAX_QUANTITY, "Invalid quantity.\n");
                UpdateQuantityByName(item.item_name, add_quantity);
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
        if (_stricmp(buff, "quest") == 0) { 
            item.quest = true; 
            break; 
        }
        if (_stricmp(buff, "not") == 0) { 
            item.quest = false; 
            break; 
        }
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
    int max_quantity = MAX_QUANTITY;
    if (item.cost_per_unit > 0) {
        int max_quantity_by_cost = MAX_TOTAL_COST / item.cost_per_unit;
        if (max_quantity_by_cost < max_quantity) 
            max_quantity = max_quantity_by_cost;
    }
    item.quantity = ReadIntInRange("Quantity: ", 1, max_quantity, "Invalid quantity.\n");
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
        system("cls");
        return;
    }
    int pos = SelectItemFromTable("SELECT ITEM TO EDIT");
    if (pos == -1) {
        std::cout << "Edit cancelled.\n";
        system("pause");
        system("cls");
        return;
    }
    Inventory item{};
    ReadRecordAt(pos, item);
    const int EDIT_MENU_SIZE = 7;
    const char* edit_menu[EDIT_MENU_SIZE] = {
        "Change Name",
        "Change Cost",
        "Change Weight",
        "Change Quantity",
        "Change Category",
        "Toggle Quest Flag",
        "Cancel"
    };
    int choice = SubMenu("EDIT ITEM", edit_menu, EDIT_MENU_SIZE);
    bool modified = false;
    bool name_changed = false;
    char new_name[MAX_LEN];
    switch (choice) {
        case 0:
            while (true) {
                ReadRequiredString("New name: ", new_name, MAX_LEN);
                if (LinearSearchExcludingIndex(new_name, pos) != -1) {
                    std::cout << "Item with name '" << new_name << "' already exists!\n";
                    continue;
                }
                strcpy_s(item.item_name, new_name);
                name_changed = true;
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
            int max_quantity = MAX_QUANTITY;
            if (item.cost_per_unit > 0) {
                int max_quantity_by_cost = MAX_TOTAL_COST / item.cost_per_unit;
                if (max_quantity_by_cost < max_quantity) 
                    max_quantity = max_quantity_by_cost;
            }
            item.quantity = ReadIntInRange("New quantity: ", 1, max_quantity, "Invalid quantity.\n");
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
            system("cls");
            return;
    }
    if (modified) {
        WriteRecordAt(pos, item);
        std::cout << "Saved!\n";
        if (name_changed)
            std::cout << "Note: Item name was changed. Sort order may have changed.\n";
    }
    system("pause");
    system("cls");
}

void DeleteItem() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty. Nothing to delete.\n";
        system("pause");
        system("cls");
        return;
    }
    int pos = SelectItemFromTable("SELECT ITEM TO DELETE");
    if (pos == -1) {
        std::cout << "Delete cancelled.\n";
        system("pause");
        system("cls");
        return;
    }
    Inventory item{};
    ReadRecordAt(pos, item);
    std::cout << "\n=== CONFIRM DELETE ===\n";
    PrintItem(item);
    std::cout << "Are you sure you want to delete this item? (y/n): ";
    while (_kbhit()) 
        _getch();
    char confirm = _getch();
    std::cout << confirm << "\n\n";
    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "Delete cancelled.\n";
        system("pause");
        system("cls");
        return;
    }
    DeleteCtx context = { pos };
    AtomicRewrite(TransDelete, &context);
    std::cout << "Item deleted!\n";
    system("pause");
    system("cls");
}

void PrintAllFromFile() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        system("cls");
        return;
    }
    std::cout << "\n=== INVENTORY ===\n";
    PrintItemTable();
    system("pause");
    system("cls");
}

void SortFileByWeight_Bubble() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        system("cls");
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
        if (!swapped) 
            break;
    }
    std::cout << "Sorted by weight!\n";
    system("pause");
    system("cls");
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
    system("cls");
}

void SearchByWeight() {
    std::cout << "Binary search requires the data to be sorted by weight.\n";
    std::cout << "The file will now be sorted by weight.\n";
    std::cout << "The original order of records will be changed.\n\n";
    std::cout << "Do you want to continue? (y/n): ";
    while (_kbhit()) 
        _getch();
    char confirm = _getch();
    std::cout << confirm << "\n\n";
    system("cls");
    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "Operation cancelled by user.\n";
        system("pause");
        system("cls");
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
    system("cls");
}

void SortFileByQuantity_Selection() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        system("cls");
        return; 
    }
    for (int i = 0; i < n - 1; ++i) {
        int min_index = i;
        Inventory min_item{};
        if (!ReadRecordAt(i, min_item)) 
            continue;
        for (int j = i + 1; j < n; ++j) {
            Inventory curr{};
            if (ReadRecordAt(j, curr))
                if (curr.quantity < min_item.quantity) {
                    min_item = curr;
                    min_index = j;
                }
        }
        if (min_index != i)
            SwapRecords(i, min_index);
    }
    std::cout << "Sorted by quantity (selection)!\n";
    PrintItemTable();
    system("pause");
    system("cls");
}

void SortFileByName_Insertion() {
    int n = GetRecordCount();
    if (n <= 1) { 
        std::cout << "Nothing to sort.\n"; 
        system("pause"); 
        system("cls");
        return; 
    }
    for (int i = 1; i < n; ++i) {
        Inventory key{};
        if (!ReadRecordAt(i, key)) 
            continue;
        int j = i - 1;
        while (j >= 0) {
            Inventory curr{};
            if (!ReadRecordAt(j, curr)) 
                break;
            if (_stricmp(curr.item_name, key.item_name) > 0) {
                WriteRecordAt(j + 1, curr);
                --j;
            }
            else
                break;
        }
        WriteRecordAt(j + 1, key);
    }
    std::cout << "Sorted by name (insertion)!\n";
    PrintItemTable();
    system("pause");
    system("cls");
}

void WriteToTmpFile(const Inventory& item) {
    std::ofstream tmp(TMP_FILE, std::ios::binary | std::ios::app);
    if (tmp) 
        tmp.write(reinterpret_cast<const char*>(&item), REC_SIZE);
}

void ClearTmpFile() {
    std::ofstream tmp(TMP_FILE, std::ios::binary | std::ios::trunc);
}

void PrintTmpFile() {
    PrintItemTable(TMP_FILE);
}

int GetTmpRecordCount() {
    return GetRecordCount(TMP_FILE);
}

void SortTmpFileByCost_Desc() {
    int n = GetTmpRecordCount();
    if (n <= 1) 
        return;
    for (int i = 0; i < n - 1; ++i) {
        Inventory max_record{};
        int max_index = -1;
        std::ifstream tmp(TMP_FILE, std::ios::binary);
        Inventory item{};
        int index = 0;
        bool found = false;
        while (tmp.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
            bool already_placed = false;
            std::ifstream placed(TMP_FILE, std::ios::binary);
            Inventory p{};
            int pIdx = 0;
            while (placed.read(reinterpret_cast<char*>(&p), REC_SIZE) && pIdx < i) {
                if (memcmp(&p, &item, REC_SIZE) == 0) {
                    already_placed = true;
                    break;
                }
                ++pIdx;
            }
            placed.close();
            if (!already_placed)
                if (!found || item.cost_per_unit > max_record.cost_per_unit) {
                    max_record = item;
                    max_index = index;
                    found = true;
                }
            ++index;
        }
        tmp.close();
        if (found && max_index >= i) {
            std::ifstream src(TMP_FILE, std::ios::binary);
            std::ofstream out(WORK_FILE, std::ios::binary);
            Inventory temp{};
            int cur = 0;
            while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
                if (cur < i)
                    out.write(reinterpret_cast<const char*>(&temp), REC_SIZE);
                else if (cur == i)
                    out.write(reinterpret_cast<const char*>(&max_record), REC_SIZE);
                else if (cur != max_index)
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
    int filter_type = SelectCategoryForSearch(category, MAX_LEN);
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int matches = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        bool category_match = false;
        if (filter_type == 2)
            category_match = true;
        else if (filter_type == 0)
            category_match = IsStringEmpty(item.category);
        else
            category_match = (_stricmp(item.category, category) == 0);
        if (category_match && item.weight >= low && item.weight <= high) {
            WriteToTmpFile(item);
            matches++;
        }
    }
    src.close();
    if (matches == 0) {
        std::cout << "No items found.\n";
        system("pause");
        system("cls");
        return;
    }
    SortTmpFileByCost_Desc();
    std::cout << "\n=== RESULTS (sorted by cost/unit DESC) ===\n";
    std::cout << "Found: " << matches << " item(s)\n";
    if (filter_type == 0)
        std::cout << "Filter: Items WITHOUT category\n";
    else if (filter_type == 1)
        std::cout << "Filter: Category = " << category << "\n";
    else
        std::cout << "Filter: All categories\n";
    std::cout << "Weight range: " << std::fixed << std::setprecision(2) << low << " - " << high << " kg\n\n";
    PrintItemTable(TMP_FILE);
    remove(TMP_FILE);
    system("pause");
    system("cls");
}

void ViewInventoryByCategory() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        system("cls");
        return;
    }
    ClearTmpFile();
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    Inventory item{};
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) 
        tmp.write(reinterpret_cast<char*>(&item), REC_SIZE);
    src.close(); tmp.close();
    int tmp_n = GetTmpRecordCount();
    for (int i = 1; i < tmp_n; ++i) {
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
            int cat_cmp = _stricmp(curr.category, key.category);
            int name_cmp = (cat_cmp == 0) ? _stricmp(curr.item_name, key.item_name) : cat_cmp;
            if (name_cmp > 0) {
                std::fstream tOut(TMP_FILE, std::ios::binary | std::ios::in | std::ios::out);
                tOut.seekp((j + 1) * REC_SIZE);
                tOut.write(reinterpret_cast<char*>(&curr), REC_SIZE);
                tOut.close();
                --j;
            }
            else 
                break;
        }
        std::fstream tOut(TMP_FILE, std::ios::binary | std::ios::in | std::ios::out);
        tOut.seekp((j + 1) * REC_SIZE);
        tOut.write(reinterpret_cast<char*>(&key), REC_SIZE);
        tOut.close();
    }
    std::cout << "\n=== INVENTORY BY CATEGORY ===\n";
    char current_cat[MAX_LEN] = "";
    bool first_item = true;
    std::ifstream tView(TMP_FILE, std::ios::binary);
    Inventory view_item{};
    while (tView.read(reinterpret_cast<char*>(&view_item), REC_SIZE)) {
        bool current_empty = IsStringEmpty(current_cat);
        bool view_empty = IsStringEmpty(view_item.category);
        bool category_changed = (current_empty != view_empty) || (!current_empty && !view_empty && _stricmp(current_cat, view_item.category) != 0);
        if (category_changed || first_item) {
            strcpy_s(current_cat, view_item.category);
            if (view_empty)
                std::cout << "\n>>> Category: (none/empty) <<<\n";
            else
                std::cout << "\n>>> Category: " << current_cat << " <<<\n";
            first_item = false;
        }
        std::cout << " - " << view_item.item_name << " (x" << view_item.quantity 
                  << ", " << std::fixed << std::setprecision(2) << view_item.weight << "kg)\n";
    }
    tView.close();
    int max_cost_index = -1, max_weight_index = -1;
    double max_total_cost = -1, max_total_weight = -1;
    for (int i = 0; i < n; ++i)
        if (ReadRecordAt(i, item)) {
            double total_cost = static_cast<double>(item.cost_per_unit) * item.quantity;
            double total_weight = item.weight * item.quantity;
            if (total_cost > max_total_cost) {
                max_total_cost = total_cost;
                max_cost_index = i;
            }
            if (total_weight > max_total_weight) {
                max_total_weight = total_weight;
                max_weight_index = i;
            }
        }
    std::cout << "\n=== ANALYSIS ===\n";
    if (max_cost_index != -1 && ReadRecordAt(max_cost_index, item))
        std::cout << "Most expensive set: " << item.item_name << " | Total: " << max_total_cost << " gold\n";
    if (max_weight_index != -1 && ReadRecordAt(max_weight_index, item))
        std::cout << "Heaviest set: " << item.item_name << " | Total: " << max_total_weight << " kg\n";
    remove(TMP_FILE);
    system("pause");
    system("cls");
}

void GenerateReport() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        system("cls");
        return;
    }
    std::ofstream txt("inventory_report.txt");
    if (!txt) {
        std::cout << "Error creating report!\n";
        system("pause");
        system("cls");
        return;
    }
    txt << "===== INVENTORY REPORT =====\n";
    txt << "Generated: " << __DATE__ << " " << __TIME__ << "\n\n";
    Inventory item{};
    int total_items = 0, total_weight = 0, total_cost = 0;
    std::ifstream src(FILENAME, std::ios::binary);
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        txt << "Item: " << item.item_name << "\n";
        txt << " Category: " << (IsStringEmpty(item.category) ? "(none)" : item.category) << "\n";
        txt << " Quest: " << (item.quest ? "Yes" : "No") << "\n";
        txt << " Cost/unit: " << item.cost_per_unit << " | Qty: " << item.quantity 
            << " | Total: " << item.Full_cost() << "\n";
        txt << " Weight/unit: " << std::fixed << std::setprecision(2) << item.weight 
            << " | Total weight: " << (item.weight * item.quantity) << "\n\n";
        total_items += item.quantity;
        total_weight += static_cast<int>(item.weight * item.quantity);
        total_cost += item.Full_cost();
    }
    src.close();
    txt << "===== SUMMARY =====\n";
    txt << "Total unique items: " << n << "\n";
    txt << "Total quantity: " << total_items << "\n";
    txt << "Total weight: " << total_weight << " kg\n";
    txt << "Total value: " << total_cost << " gold\n";
    txt.close();
    std::cout << "Report saved to 'inventory_report.txt'\n";
    system("pause");
    system("cls");
}

void HelpWithOverload() {
    int n = GetRecordCount();
    if (n == 0) {
        std::cout << "Inventory is empty.\n";
        system("pause");
        system("cls");
        return;
    }
    int worst_index = -1;
    double worst_ratio = 1e9;
    std::ifstream src(FILENAME, std::ios::binary);
    Inventory item{};
    int index = 0;
    while (src.read(reinterpret_cast<char*>(&item), REC_SIZE)) {
        if (!item.quest && item.weight > 0.001) {
            double ratio = static_cast<double>(item.cost_per_unit) / item.weight;
            if (ratio < worst_ratio) {
                worst_ratio = ratio;
                worst_index = index;
            }
        }
        ++index;
    }
    src.close();
    if (worst_index == -1)
        std::cout << "No suitable non-quest items found.\n";
    else {
        Inventory worst_item{};
        ReadRecordAt(worst_index, worst_item);
        std::cout << "\n=== LEAST COST-EFFECTIVE ITEM (NON-QUEST) ===\n";
        PrintItem(worst_item);
        std::cout << "Cost/Weight ratio: " << worst_ratio << " gold/kg\n";
        std::cout << "Consider dropping this item first!\n";
    }
    system("pause");
    system("cls");
}