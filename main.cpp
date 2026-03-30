#ifdef UNICODE
#undef UNICODE
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <commctrl.h>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <string>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// ==================== СТРУКТУРА ИНВЕНТАРЯ ====================
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
const size_t REC_SIZE = sizeof(Inventory);
// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
HWND g_hMainWnd;
HWND g_hListView;
HWND g_hStatusBar;
HINSTANCE g_hInst;
// ID элементов управления
#define IDC_LISTVIEW    1001
#define IDC_BTN_ADD     1002
#define IDC_BTN_EDIT    1003
#define IDC_BTN_DELETE  1004
#define IDC_BTN_SEARCH  1005
#define IDC_BTN_SORT_WEIGHT 1006
#define IDC_BTN_SORT_QTY    1007
#define IDC_BTN_SORT_NAME   1008
#define IDC_BTN_REPORT  1009
#define IDC_STATUSBAR   1010
// ID для диалога добавления/редактирования
#define IDC_EDIT_NAME     2001
#define IDC_EDIT_CATEGORY 2002
#define IDC_EDIT_COST     2003
#define IDC_EDIT_WEIGHT   2004
#define IDC_EDIT_QTY      2005
#define IDC_CHECK_QUEST   2006
// ==================== ФУНКЦИИ РАБОТЫ С ФАЙЛОМ ====================
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
void AddRecord(const Inventory& item) {
    std::ofstream file(FILENAME, std::ios::binary | std::ios::app);
    if (file) {
        file.write(reinterpret_cast<const char*>(&item), REC_SIZE);
    }
}
void DeleteRecordAt(int index) {
    int n = GetRecordCount();
    if (index < 0 || index >= n) return;
    
    const char* TMP_FILE = "inventory_tmp.bin";
    std::ifstream src(FILENAME, std::ios::binary);
    std::ofstream tmp(TMP_FILE, std::ios::binary);
    
    Inventory temp{};
    int i = 0;
    while (src.read(reinterpret_cast<char*>(&temp), REC_SIZE)) {
        if (i != index)
            tmp.write(reinterpret_cast<char*>(&temp), REC_SIZE);
        ++i;
    }
    src.close();
    tmp.close();
    
    remove(FILENAME);
    rename(TMP_FILE, FILENAME);
}
void SwapRecords(int idx1, int idx2) {
    if (idx1 == idx2) return;
    Inventory a{}, b{};
    ReadRecordAt(idx1, a);
    ReadRecordAt(idx2, b);
    WriteRecordAt(idx1, b);
    WriteRecordAt(idx2, a);
}
int LinearSearchByName(const char* name) {
    int n = GetRecordCount();
    Inventory temp{};
    for (int i = 0; i < n; ++i) {
        if (ReadRecordAt(i, temp) && _stricmp(temp.item_name, name) == 0)
            return i;
    }
    return -1;
}
// ==================== СОРТИРОВКИ ====================
void SortByWeight() {
    int n = GetRecordCount();
    if (n <= 1) return;
    
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
}
void SortByQuantity() {
    int n = GetRecordCount();
    if (n <= 1) return;
    
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
}
void SortByName() {
    int n = GetRecordCount();
    if (n <= 1) return;
    
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
}
// ==================== ГЕНЕРАЦИЯ ОТЧЁТА ====================
void GenerateReport() {
    int n = GetRecordCount();
    if (n == 0) {
        MessageBox(g_hMainWnd, "Инвентарь пуст!", "Отчёт", MB_ICONINFORMATION);
        return;
    }
    
    std::ofstream txt("inventory_report.txt");
    if (!txt) {
        MessageBox(g_hMainWnd, "Ошибка создания отчёта!", "Ошибка", MB_ICONERROR);
        return;
    }
    
    txt << "===== INVENTORY REPORT =====\n";
    txt << "Generated: " << __DATE__ << " " << __TIME__ << "\n\n";
    
    Inventory item{};
    int totalItems = 0, totalCost = 0;
    double totalWeight = 0;
    
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
            totalWeight += item.weight * item.quantity;
            totalCost += item.Full_cost();
        }
    }
    
    txt << "===== SUMMARY =====\n";
    txt << "Total unique items: " << n << "\n";
    txt << "Total quantity: " << totalItems << "\n";
    txt << "Total weight: " << totalWeight << " kg\n";
    txt << "Total value: " << totalCost << " gold\n";
    txt.close();
    
    MessageBox(g_hMainWnd, "Отчёт сохранён в 'inventory_report.txt'", "Отчёт", MB_ICONINFORMATION);
    ShellExecute(NULL, "open", "inventory_report.txt", NULL, NULL, SW_SHOWNORMAL);
}
// ==================== LISTVIEW ФУНКЦИИ ====================
void InitListView(HWND hList) {
    ListView_SetExtendedListViewStyle(hList, 
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    
    LVCOLUMN lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    
    const char* columns[] = {"Название", "Квест", "Цена", "Категория", "Вес", "Кол-во", "Всего"};
    int widths[] = {150, 60, 80, 120, 80, 70, 100};
    
    for (int i = 0; i < 7; i++) {
        lvc.iSubItem = i;
        lvc.pszText = (LPSTR)columns[i];
        lvc.cx = widths[i];
        ListView_InsertColumn(hList, i, &lvc);
    }
}
void RefreshListView() {
    ListView_DeleteAllItems(g_hListView);
    
    int n = GetRecordCount();
    Inventory item{};
    
    for (int i = 0; i < n; i++) {
        if (ReadRecordAt(i, item)) {
            LVITEM lvi = {};
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = i;
            lvi.lParam = i;
            lvi.pszText = item.item_name;
            int idx = ListView_InsertItem(g_hListView, &lvi);
            
            ListView_SetItemText(g_hListView, idx, 1, (LPSTR)(item.quest ? "Да" : "Нет"));
            
            char buf[64];
            sprintf(buf, "%d", item.cost_per_unit);
            ListView_SetItemText(g_hListView, idx, 2, buf);
            
            ListView_SetItemText(g_hListView, idx, 3, item.category);
            
            sprintf(buf, "%.2f", item.weight);
            ListView_SetItemText(g_hListView, idx, 4, buf);
            
            sprintf(buf, "%d", item.quantity);
            ListView_SetItemText(g_hListView, idx, 5, buf);
            
            sprintf(buf, "%d", item.Full_cost());
            ListView_SetItemText(g_hListView, idx, 6, buf);
        }
    }
    
    // Обновление статусбара
    char status[128];
    sprintf(status, "Предметов: %d", n);
    SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)status);
}
int GetSelectedIndex() {
    return ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
}
// ==================== ДИАЛОГ ДОБАВЛЕНИЯ/РЕДАКТИРОВАНИЯ ====================
Inventory g_DialogItem;
bool g_IsEditMode = false;
INT_PTR CALLBACK ItemDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Центрирование диалога
            RECT rc, rcOwner;
            GetWindowRect(g_hMainWnd, &rcOwner);
            GetWindowRect(hDlg, &rc);
            int x = rcOwner.left + (rcOwner.right - rcOwner.left - (rc.right - rc.left)) / 2;
            int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - (rc.bottom - rc.top)) / 2;
            SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            
            // Заполнение полей при редактировании
            if (g_IsEditMode) {
                SetWindowText(hDlg, "Редактировать предмет");
                SetDlgItemText(hDlg, IDC_EDIT_NAME, g_DialogItem.item_name);
                SetDlgItemText(hDlg, IDC_EDIT_CATEGORY, g_DialogItem.category);
                SetDlgItemInt(hDlg, IDC_EDIT_COST, g_DialogItem.cost_per_unit, FALSE);
                
                char buf[32];
                sprintf(buf, "%.2f", g_DialogItem.weight);
                SetDlgItemText(hDlg, IDC_EDIT_WEIGHT, buf);
                
                SetDlgItemInt(hDlg, IDC_EDIT_QTY, g_DialogItem.quantity, FALSE);
                CheckDlgButton(hDlg, IDC_CHECK_QUEST, g_DialogItem.quest ? BST_CHECKED : BST_UNCHECKED);
            } else {
                SetWindowText(hDlg, "Добавить предмет");
            }
            return TRUE;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: {
                    // Получение данных из полей
                    GetDlgItemText(hDlg, IDC_EDIT_NAME, g_DialogItem.item_name, MAX_LEN);
                    GetDlgItemText(hDlg, IDC_EDIT_CATEGORY, g_DialogItem.category, MAX_LEN);
                    g_DialogItem.cost_per_unit = GetDlgItemInt(hDlg, IDC_EDIT_COST, NULL, FALSE);
                    
                    char buf[32];
                    GetDlgItemText(hDlg, IDC_EDIT_WEIGHT, buf, 32);
                    g_DialogItem.weight = atof(buf);
                    
                    g_DialogItem.quantity = GetDlgItemInt(hDlg, IDC_EDIT_QTY, NULL, FALSE);
                    g_DialogItem.quest = (IsDlgButtonChecked(hDlg, IDC_CHECK_QUEST) == BST_CHECKED);
                    
                    // Валидация
                    if (strlen(g_DialogItem.item_name) == 0) {
                        MessageBox(hDlg, "Введите название предмета!", "Ошибка", MB_ICONWARNING);
                        return TRUE;
                    }
                    
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                }
                
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
            
        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}
// Создание диалога программно (без ресурсов)
INT_PTR ShowItemDialog(HWND hParent, bool editMode) {
    g_IsEditMode = editMode;
    if (!editMode) {
        memset(&g_DialogItem, 0, sizeof(g_DialogItem));
    }
    
    // Создание шаблона диалога в памяти
    #pragma pack(push, 4)
    struct {
        DLGTEMPLATE dlg;
        WORD menu, wndClass, title;
        // Контролы добавим через CreateWindow в WM_INITDIALOG
    } dlgTemplate = {};
    #pragma pack(pop)
    
    dlgTemplate.dlg.style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    dlgTemplate.dlg.dwExtendedStyle = 0;
    dlgTemplate.dlg.cdit = 0;
    dlgTemplate.dlg.x = 0;
    dlgTemplate.dlg.y = 0;
    dlgTemplate.dlg.cx = 200;
    dlgTemplate.dlg.cy = 180;
    
    // Используем обычное окно вместо DialogBox для большего контроля
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        WC_DIALOG,
        editMode ? "Редактировать предмет" : "Добавить предмет",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 320,
        hParent,
        NULL,
        g_hInst,
        NULL
    );
    
    if (!hDlg) return IDCANCEL;
    
    // Центрирование
    RECT rc, rcOwner;
    GetWindowRect(hParent, &rcOwner);
    GetWindowRect(hDlg, &rc);
    int x = rcOwner.left + (rcOwner.right - rcOwner.left - (rc.right - rc.left)) / 2;
    int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - (rc.bottom - rc.top)) / 2;
    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // Создание контролов
    int yPos = 15;
    int labelWidth = 80;
    int editWidth = 220;
    int height = 22;
    int spacing = 30;
    
    CreateWindow("STATIC", "Название:", WS_VISIBLE | WS_CHILD, 
        15, yPos, labelWidth, height, hDlg, NULL, g_hInst, NULL);
    HWND hName = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 
        100, yPos, editWidth, height, hDlg, (HMENU)IDC_EDIT_NAME, g_hInst, NULL);
    yPos += spacing;
    
    CreateWindow("STATIC", "Категория:", WS_VISIBLE | WS_CHILD, 
        15, yPos, labelWidth, height, hDlg, NULL, g_hInst, NULL);
    HWND hCat = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 
        100, yPos, editWidth, height, hDlg, (HMENU)IDC_EDIT_CATEGORY, g_hInst, NULL);
    yPos += spacing;
    
    CreateWindow("STATIC", "Цена:", WS_VISIBLE | WS_CHILD, 
        15, yPos, labelWidth, height, hDlg, NULL, g_hInst, NULL);
    HWND hCost = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_NUMBER, 
        100, yPos, editWidth, height, hDlg, (HMENU)IDC_EDIT_COST, g_hInst, NULL);
    yPos += spacing;
    
    CreateWindow("STATIC", "Вес:", WS_VISIBLE | WS_CHILD, 
        15, yPos, labelWidth, height, hDlg, NULL, g_hInst, NULL);
    HWND hWeight = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
        100, yPos, editWidth, height, hDlg, (HMENU)IDC_EDIT_WEIGHT, g_hInst, NULL);
    yPos += spacing;
    
    CreateWindow("STATIC", "Кол-во:", WS_VISIBLE | WS_CHILD, 
        15, yPos, labelWidth, height, hDlg, NULL, g_hInst, NULL);
    HWND hQty = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_NUMBER, 
        100, yPos, editWidth, height, hDlg, (HMENU)IDC_EDIT_QTY, g_hInst, NULL);
    yPos += spacing;
    
    HWND hQuest = CreateWindow("BUTTON", "Квестовый предмет", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 
        100, yPos, 150, height, hDlg, (HMENU)IDC_CHECK_QUEST, g_hInst, NULL);
    yPos += spacing + 10;
    
    HWND hOK = CreateWindow("BUTTON", "OK", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 
        80, yPos, 80, 28, hDlg, (HMENU)IDOK, g_hInst, NULL);
    HWND hCancel = CreateWindow("BUTTON", "Отмена", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
        180, yPos, 80, 28, hDlg, (HMENU)IDCANCEL, g_hInst, NULL);
    
    // Шрифт
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    EnumChildWindows(hDlg, [](HWND hwnd, LPARAM lParam) -> BOOL {
        SendMessage(hwnd, WM_SETFONT, lParam, TRUE);
        return TRUE;
    }, (LPARAM)hFont);
    
    // Заполнение при редактировании
    if (editMode) {
        SetDlgItemText(hDlg, IDC_EDIT_NAME, g_DialogItem.item_name);
        SetDlgItemText(hDlg, IDC_EDIT_CATEGORY, g_DialogItem.category);
        SetDlgItemInt(hDlg, IDC_EDIT_COST, g_DialogItem.cost_per_unit, FALSE);
        
        char buf[32];
        sprintf(buf, "%.2f", g_DialogItem.weight);
        SetDlgItemText(hDlg, IDC_EDIT_WEIGHT, buf);
        
        SetDlgItemInt(hDlg, IDC_EDIT_QTY, g_DialogItem.quantity, FALSE);
        CheckDlgButton(hDlg, IDC_CHECK_QUEST, g_DialogItem.quest ? BST_CHECKED : BST_UNCHECKED);
    }
    
    // Модальный цикл
    EnableWindow(hParent, FALSE);
    ShowWindow(hDlg, SW_SHOW);
    SetFocus(hName);
    
    INT_PTR result = IDCANCEL;
    MSG msg;
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.hwnd == hDlg || IsChild(hDlg, msg.hwnd)) {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                result = IDCANCEL;
                break;
            }
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
                msg.message = WM_COMMAND;
                msg.wParam = IDOK;
            }
        }
        
        if (msg.message == WM_COMMAND) {
            if (LOWORD(msg.wParam) == IDOK) {
                // Получение данных
                GetDlgItemText(hDlg, IDC_EDIT_NAME, g_DialogItem.item_name, MAX_LEN);
                GetDlgItemText(hDlg, IDC_EDIT_CATEGORY, g_DialogItem.category, MAX_LEN);
                g_DialogItem.cost_per_unit = GetDlgItemInt(hDlg, IDC_EDIT_COST, NULL, FALSE);
                
                char buf[32];
                GetDlgItemText(hDlg, IDC_EDIT_WEIGHT, buf, 32);
                g_DialogItem.weight = atof(buf);
                
                g_DialogItem.quantity = GetDlgItemInt(hDlg, IDC_EDIT_QTY, NULL, FALSE);
                g_DialogItem.quest = (IsDlgButtonChecked(hDlg, IDC_CHECK_QUEST) == BST_CHECKED);
                
                if (strlen(g_DialogItem.item_name) == 0) {
                    MessageBox(hDlg, "Введите название предмета!", "Ошибка", MB_ICONWARNING);
                    continue;
                }
                
                result = IDOK;
                break;
            }
            if (LOWORD(msg.wParam) == IDCANCEL) {
                result = IDCANCEL;
                break;
            }
        }
        
        if (!IsWindow(hDlg)) break;
        
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    EnableWindow(hParent, TRUE);
    DestroyWindow(hDlg);
    SetForegroundWindow(hParent);
    
    return result;
}
// ==================== ДИАЛОГ ПОИСКА ====================
void ShowSearchDialog() {
    char searchName[MAX_LEN] = "";
    
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        WC_DIALOG,
        "Поиск по названию",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 120,
        g_hMainWnd,
        NULL,
        g_hInst,
        NULL
    );
    
    if (!hDlg) return;
    
    // Центрирование
    RECT rc, rcOwner;
    GetWindowRect(g_hMainWnd, &rcOwner);
    GetWindowRect(hDlg, &rc);
    int x = rcOwner.left + (rcOwner.right - rcOwner.left - (rc.right - rc.left)) / 2;
    int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - (rc.bottom - rc.top)) / 2;
    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    CreateWindow("STATIC", "Название:", WS_VISIBLE | WS_CHILD, 
        15, 20, 70, 22, hDlg, NULL, g_hInst, NULL);
    HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 
        90, 18, 230, 24, hDlg, (HMENU)1001, g_hInst, NULL);
    
    HWND hOK = CreateWindow("BUTTON", "Найти", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 
        90, 55, 80, 28, hDlg, (HMENU)IDOK, g_hInst, NULL);
    HWND hCancel = CreateWindow("BUTTON", "Отмена", 
        WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
        180, 55, 80, 28, hDlg, (HMENU)IDCANCEL, g_hInst, NULL);
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    EnumChildWindows(hDlg, [](HWND hwnd, LPARAM lParam) -> BOOL {
        SendMessage(hwnd, WM_SETFONT, lParam, TRUE);
        return TRUE;
    }, (LPARAM)hFont);
    
    EnableWindow(g_hMainWnd, FALSE);
    SetFocus(hEdit);
    
    MSG msg;
    bool found = false;
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) break;
        
        if (msg.message == WM_COMMAND || (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)) {
            if (LOWORD(msg.wParam) == IDOK || msg.wParam == VK_RETURN) {
                GetDlgItemText(hDlg, 1001, searchName, MAX_LEN);
                if (strlen(searchName) > 0) {
                    int pos = LinearSearchByName(searchName);
                    if (pos >= 0) {
                        ListView_SetItemState(g_hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                        ListView_SetItemState(g_hListView, pos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        ListView_EnsureVisible(g_hListView, pos, FALSE);
                        found = true;
                    } else {
                        MessageBox(hDlg, "Предмет не найден!", "Поиск", MB_ICONINFORMATION);
                        continue;
                    }
                }
                break;
            }
            if (LOWORD(msg.wParam) == IDCANCEL) break;
        }
        
        if (!IsWindow(hDlg)) break;
        
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    EnableWindow(g_hMainWnd, TRUE);
    DestroyWindow(hDlg);
    SetForegroundWindow(g_hMainWnd);
}
// ==================== ГЛАВНОЕ ОКНО ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Панель кнопок
            int btnY = 10;
            int btnW = 120;
            int btnH = 30;
            int btnX = 10;
            int spacing = 130;
            
            CreateWindow("BUTTON", "Добавить", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_ADD, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Редактировать", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_EDIT, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Удалить", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_DELETE, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Поиск", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_SEARCH, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Отчёт", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_REPORT, g_hInst, NULL);
            
            // Вторая строка - сортировки
            btnY = 50;
            btnX = 10;
            
            CreateWindow("BUTTON", "Сорт. по весу", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_SORT_WEIGHT, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Сорт. по кол-ву", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_SORT_QTY, g_hInst, NULL);
            btnX += spacing;
            
            CreateWindow("BUTTON", "Сорт. по имени", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                btnX, btnY, btnW, btnH, hwnd, (HMENU)IDC_BTN_SORT_NAME, g_hInst, NULL);
            
            // ListView
            g_hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                10, 90, 760, 400, hwnd, (HMENU)IDC_LISTVIEW, g_hInst, NULL);
            
            InitListView(g_hListView);
            
            // Status Bar
            g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
                0, 0, 0, 0, hwnd, (HMENU)IDC_STATUSBAR, g_hInst, NULL);
            
            // Шрифт для всех контролов
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            EnumChildWindows(hwnd, [](HWND child, LPARAM lParam) -> BOOL {
                SendMessage(child, WM_SETFONT, lParam, TRUE);
                return TRUE;
            }, (LPARAM)hFont);
            
            RefreshListView();
            return 0;
        }
        
        case WM_SIZE: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Изменение размера ListView
            int statusHeight = 22;
            MoveWindow(g_hListView, 10, 90, rc.right - 20, rc.bottom - 90 - statusHeight - 10, TRUE);
            
            // Изменение размера StatusBar
            SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
            return 0;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BTN_ADD:
                    if (ShowItemDialog(hwnd, false) == IDOK) {
                        // Проверка на дубликат
                        if (LinearSearchByName(g_DialogItem.item_name) >= 0) {
                            MessageBox(hwnd, "Предмет с таким именем уже существует!", 
                                "Ошибка", MB_ICONWARNING);
                        } else {
                            AddRecord(g_DialogItem);
                            RefreshListView();
                        }
                    }
                    break;
                    
                case IDC_BTN_EDIT: {
                    int sel = GetSelectedIndex();
                    if (sel < 0) {
                        MessageBox(hwnd, "Выберите предмет для редактирования!", 
                            "Внимание", MB_ICONINFORMATION);
                        break;
                    }
                    ReadRecordAt(sel, g_DialogItem);
                    if (ShowItemDialog(hwnd, true) == IDOK) {
                        WriteRecordAt(sel, g_DialogItem);
                        RefreshListView();
                    }
                    break;
                }
                
                case IDC_BTN_DELETE: {
                    int sel = GetSelectedIndex();
                    if (sel < 0) {
                        MessageBox(hwnd, "Выберите предмет для удаления!", 
                            "Внимание", MB_ICONINFORMATION);
                        break;
                    }
                    Inventory item{};
                    ReadRecordAt(sel, item);
                    
                    char msg[256];
                    sprintf(msg, "Удалить '%s'?", item.item_name);
                    if (MessageBox(hwnd, msg, "Подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        DeleteRecordAt(sel);
                        RefreshListView();
                    }
                    break;
                }
                
                case IDC_BTN_SEARCH:
                    ShowSearchDialog();
                    break;
                    
                case IDC_BTN_SORT_WEIGHT:
                    SortByWeight();
                    RefreshListView();
                    MessageBox(hwnd, "Отсортировано по весу (пузырьковая сортировка)", 
                        "Сортировка", MB_ICONINFORMATION);
                    break;
                    
                case IDC_BTN_SORT_QTY:
                    SortByQuantity();
                    RefreshListView();
                    MessageBox(hwnd, "Отсортировано по количеству (сортировка выбором)", 
                        "Сортировка", MB_ICONINFORMATION);
                    break;
                    
                case IDC_BTN_SORT_NAME:
                    SortByName();
                    RefreshListView();
                    MessageBox(hwnd, "Отсортировано по имени (сортировка вставками)", 
                        "Сортировка", MB_ICONINFORMATION);
                    break;
                    
                case IDC_BTN_REPORT:
                    GenerateReport();
                    break;
            }
            return 0;
            
        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->idFrom == IDC_LISTVIEW && pnmh->code == NM_DBLCLK) {
                // Двойной клик - редактирование
                int sel = GetSelectedIndex();
                if (sel >= 0) {
                    ReadRecordAt(sel, g_DialogItem);
                    if (ShowItemDialog(hwnd, true) == IDOK) {
                        WriteRecordAt(sel, g_DialogItem);
                        RefreshListView();
                    }
                }
            }
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
// ==================== ТОЧКА ВХОДА ====================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;
    
    // Инициализация Common Controls
    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Регистрация класса окна
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "InventoryManagerClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Ошибка регистрации класса окна!", "Ошибка", MB_ICONERROR);
        return 1;
    }
    
    // Создание главного окна
    g_hMainWnd = CreateWindowEx(
        0,
        "InventoryManagerClass",
        "Inventory Manager - Win32 GUI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hMainWnd) {
        MessageBox(NULL, "Ошибка создания окна!", "Ошибка", MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}