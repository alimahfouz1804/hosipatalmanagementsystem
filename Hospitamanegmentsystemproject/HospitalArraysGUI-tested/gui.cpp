#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifdef _MSC_VER
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>
#include <sstream>
#include "hospital.h"
#include "gui_input.h"

Hospital hospital;
HWND windowHandle, titleLabel, subtitleLabel, categoryBox, formTitle;
HWND fieldLabels[6], fieldEdits[6], table, statusLabel, countLabel;
HWND addButton, updateButton, deleteButton, findButton, clearButton, showAllButton;
HWND saveFileButton, reloadFileButton;
HWND roomLabel, roomEdit, assignButton, releaseButton, scheduleButton, availableButton;
HFONT normalFont, titleFont, headingFont;
HBRUSH backgroundBrush, headerBrush;
int category = 1, selectedId = 0, searchId = 0;
int scheduleType = 0, scheduleId = 0;
bool refreshing = false;
std::filesystem::path dataFile;
bool unsavedChanges = false, loadedSavedData = false;
const wchar_t* categories[] = {L"Patients", L"Doctors", L"Nurses", L"Rooms", L"Appointments"};
const wchar_t* fieldNames[5][6] = {
    {L"Patient ID", L"First name", L"Last name", L"Age (0-120)", L"Diagnosis (optional)", L""},
    {L"Doctor ID", L"First name", L"Last name", L"Specialty", L"", L""},
    {L"Nurse ID", L"First name", L"Last name", L"Department", L"", L""},
    {L"Room ID", L"Room type", L"Capacity (1-100)", L"", L"", L""},
    {L"Appointment ID", L"Patient ID", L"Doctor ID", L"Date (YYYY-MM-DD)", L"Time (HH:MM)",
     L"Duration (minutes)"}};
const int fieldCounts[] = {5, 4, 4, 3, 6};
const wchar_t* columns[5][7] = {
    {L"ID", L"First name", L"Last name", L"Age", L"Diagnosis", L"Room", L""},
    {L"ID", L"First name", L"Last name", L"Specialty", L"", L"", L""},
    {L"ID", L"First name", L"Last name", L"Department", L"", L"", L""},
    {L"ID", L"Type", L"Capacity", L"Occupied", L"Available", L"", L""},
    {L"ID", L"Patient", L"Doctor", L"Date", L"Time", L"Minutes", L""}};
const int columnCounts[] = {6, 4, 4, 5, 6};

wstring Wide(const string& value) {
    return Utf8ToWide(value);
}
string Text(HWND control) {
    int length = GetWindowTextLengthW(control);
    wchar_t* wide = new wchar_t[length + 1];
    GetWindowTextW(control, wide, length + 1);
    string value = WideToUtf8(wide);
    delete[] wide;
    return value;
}
void SetText(HWND control, const string& value) {
    SetWindowTextW(control, Wide(value).c_str());
}
void Status(const wchar_t* message) {
    SetWindowTextW(statusLabel, message);
}
bool SaveRecords(const wchar_t* successMessage) {
    string error;
    if (!hospital.SaveToFile(dataFile, error)) {
        wstring message =
            L"Could not save records: " + Wide(error) +
            L"\nChanges are still in memory. Check the data folder and use Save records to retry.";
        Status(L"Save failed. Changes are still in memory. Use Save records to retry.");
        MessageBoxW(windowHandle, message.c_str(), L"Save failed", MB_OK | MB_ICONERROR);
        return false;
    }
    unsavedChanges = false;
    Status(successMessage);
    return true;
}
void PersistChange(const wchar_t* successMessage) {
    unsavedChanges = true;
    SaveRecords(successMessage);
}

bool InitializeStorage() {
    wchar_t module[32768];
    DWORD length = GetModuleFileNameW(nullptr, module, 32768);
    if (!length || length >= 32768) {
        MessageBoxW(nullptr, L"Cannot locate the application folder.", L"Hospital data",
                    MB_OK | MB_ICONERROR);
        return false;
    }
    dataFile = std::filesystem::path(module).parent_path() / L"hospital_data.txt";
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return false;
    }
    if (argc == 2) {
        dataFile = argv[1];
    }
    LocalFree(argv);
    if (argc > 2) {
        MessageBoxW(nullptr, L"Usage: HospitalArraysGUI.exe [data-file]", L"Hospital data",
                    MB_OK | MB_ICONERROR);
        return false;
    }
    std::error_code status;
    dataFile = std::filesystem::absolute(dataFile, status);
    if (status) {
        MessageBoxW(nullptr, L"Cannot resolve the data-file path.", L"Hospital data",
                    MB_OK | MB_ICONERROR);
        return false;
    }
    bool exists = std::filesystem::exists(dataFile, status);
    string error;
    if (status || (exists && !hospital.LoadFromFile(dataFile, error))) {
        if (status) {
            error = "Cannot access the data file: " + status.message();
        }
        wstring message = L"Cannot load saved data: " + Wide(error) + L"\n\n" + dataFile.wstring() +
                          L"\n\nThe file is unchanged. Restore a valid backup or start with a "
                          L"different data-file path.";
        MessageBoxW(nullptr, message.c_str(), L"Hospital data", MB_OK | MB_ICONERROR);
        return false;
    }
    loadedSavedData = exists;
    return true;
}
bool Number(const string& text, int& value) {
    return ParseNumber(text, value);
}
HWND Control(const wchar_t* type, const wchar_t* text, DWORD style, int id) {
    DWORD extraStyle = 0;
    if (wcscmp(type, WC_EDITW) == 0) {
        extraStyle = WS_EX_CLIENTEDGE;
    }
    HWND control =
        CreateWindowExW(extraStyle, type, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
                        windowHandle, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(control, WM_SETFONT, (WPARAM)normalFont, TRUE);
    return control;
}
void Place(HWND control, int x, int y, int w, int h) {
    MoveWindow(control, x, y, w, h, TRUE);
}

void Layout() {
    RECT rect;
    GetClientRect(windowHandle, &rect);
    int width = rect.right, height = rect.bottom;
    Place(titleLabel, 24, 16, 600, 36);
    Place(subtitleLabel, 26, 57, width - 50, 24);
    Place(categoryBox, 24, 105, 282, 180);
    Place(formTitle, 24, 148, 282, 28);
    for (int i = 0; i < 6; i++) {
        Place(fieldLabels[i], 24, 176 + i * 44, 282, 18);
        Place(fieldEdits[i], 24, 194 + i * 44, 282, 26);
    }
    int actionY = 176 + fieldCounts[category - 1] * 44;
    Place(addButton, 24, actionY, 88, 32);
    Place(updateButton, 120, actionY, 88, 32);
    Place(deleteButton, 216, actionY, 90, 32);
    Place(findButton, 24, actionY + 40, 136, 32);
    Place(clearButton, 168, actionY + 40, 138, 32);
    int extraY = actionY + 86;
    Place(roomLabel, 24, extraY, 282, 20);
    Place(roomEdit, 24, extraY + 22, 80, 28);
    Place(assignButton, 112, extraY + 22, 194, 28);
    Place(releaseButton, 24, extraY + 58, 282, 30);
    Place(scheduleButton, 24, extraY + 96, 282, 30);
    if (category == 2) {
        Place(scheduleButton, 24, extraY, 282, 32);
    }
    Place(showAllButton, 334, 105, 145, 32);
    Place(availableButton, 492, 105, 230, 32);
    Place(saveFileButton, width - 316, 105, 138, 32);
    Place(reloadFileButton, width - 166, 105, 142, 32);
    Place(countLabel, 334, 150, width - 358, 24);
    Place(table, 334, 184, width - 358, height - 260);
    Place(statusLabel, 24, height - 58, width - 48, 38);
    int cols = columnCounts[category - 1], availableWidth = width - 378;
    for (int i = 0; i < cols; i++) {
        int colWidth = (availableWidth - 55) / (cols - 1);
        if (i == 0) {
            colWidth = 55;
        }
        SendMessageW(table, LVM_SETCOLUMNWIDTH, i, colWidth);
    }
}
void ClearForm() {
    selectedId = 0;
    if (table) {
        LVITEMW state = {};
        state.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        SendMessageW(table, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&state);
    }
    for (int i = 0; i < 6; i++) {
        SetWindowTextW(fieldEdits[i], L"");
    }
    SetWindowTextW(roomEdit, L"");
    if (category == 5) {
        SetWindowTextW(fieldEdits[5], L"30");
    }
}
void Configure() {
    ClearForm();
    wstring heading = L"Manage ";
    heading += categories[category - 1];
    SetWindowTextW(formTitle, heading.c_str());
    for (int i = 0; i < 6; i++) {
        bool visible = i < fieldCounts[category - 1];
        SetWindowTextW(fieldLabels[i], fieldNames[category - 1][i]);
        int show = SW_HIDE;
        if (visible) {
            show = SW_SHOW;
        }
        ShowWindow(fieldLabels[i], show);
        ShowWindow(fieldEdits[i], show);
        SendMessageW(fieldEdits[i], EM_SETLIMITTEXT, 511, 0);
    }
    HWND roomControls[4] = {roomLabel, roomEdit, assignButton, releaseButton};
    int showRooms = SW_HIDE;
    if (category == 1) {
        showRooms = SW_SHOW;
    }
    for (int i = 0; i < 4; i++) {
        ShowWindow(roomControls[i], showRooms);
    }
    ShowWindow(scheduleButton, SW_HIDE);
    if (category == 1 || category == 2) {
        ShowWindow(scheduleButton, SW_SHOW);
    }
    ShowWindow(availableButton, SW_HIDE);
    if (category == 4) {
        ShowWindow(availableButton, SW_SHOW);
    }
    SendMessageW(availableButton, BM_SETCHECK, BST_UNCHECKED, 0);
    refreshing = true;
    SendMessageW(table, LVM_DELETEALLITEMS, 0, 0);
    while (SendMessageW(table, LVM_DELETECOLUMN, 0, 0)) {
    }
    for (int i = 0; i < columnCounts[category - 1]; i++) {
        LVCOLUMNW column = {};
        column.mask = LVCF_TEXT | LVCF_WIDTH;
        column.pszText = const_cast<wchar_t*>(columns[category - 1][i]);
        column.cx = 110;
        SendMessageW(table, LVM_INSERTCOLUMNW, i, (LPARAM)&column);
    }
    refreshing = false;
    Layout();
}
void AddRow(int id, const string values[], int count) {
    LVITEMW item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = (int)SendMessageW(table, LVM_GETITEMCOUNT, 0, 0);
    wstring first = Wide(values[0]);
    item.pszText = const_cast<wchar_t*>(first.c_str());
    item.lParam = id;
    int row = (int)SendMessageW(table, LVM_INSERTITEMW, 0, (LPARAM)&item);
    for (int i = 1; i < count; i++) {
        wstring value = Wide(values[i]);
        LVITEMW cell = {};
        cell.iSubItem = i;
        cell.pszText = const_cast<wchar_t*>(value.c_str());
        SendMessageW(table, LVM_SETITEMTEXTW, row, (LPARAM)&cell);
    }
}
int CALLBACK CompareAppointments(LPARAM left, LPARAM right, LPARAM) {
    const Appointment* a = hospital.FindAppointment((int)left);
    const Appointment* b = hospital.FindAppointment((int)right);
    if (!a || !b) {
        return 0;
    }
    int date = strcmp(a->getDate(), b->getDate());
    if (date) {
        return date;
    }
    if (a->getStartMinutes() != b->getStartMinutes()) {
        return a->getStartMinutes() - b->getStartMinutes();
    }
    return a->getId() - b->getId();
}
void Refresh() {
    refreshing = true;
    SendMessageW(table, LVM_DELETEALLITEMS, 0, 0);
    bool onlyAvailable = SendMessageW(availableButton, BM_GETCHECK, 0, 0) == BST_CHECKED;
    for (int i = 0; i < hospital.Count(category); i++) {
        int id = 0;
        if (category == 1) {
            id = hospital.GetPatientAt(i)->getId();
        }
        if (category == 2) {
            id = hospital.GetDoctorAt(i)->getId();
        }
        if (category == 3) {
            id = hospital.GetNurseAt(i)->getId();
        }
        if (category == 4) {
            id = hospital.GetRoomAt(i)->getId();
        }
        if (category == 5) {
            id = hospital.GetAppointmentAt(i)->getId();
        }
        if (searchId && searchId != id) {
            continue;
        }
        if (category == 1) {
            const Patient* p = hospital.FindPatient(id);
            string room = "Unassigned";
            if (p->getRoomId() != 0) {
                room = to_string(p->getRoomId());
            }
            string row[6] = {to_string(id),          p->getFname(),     p->getLname(),
                             to_string(p->getAge()), p->getDiagnosis(), room};
            AddRow(id, row, 6);
        }
        if (category == 2) {
            const Doctor* d = hospital.FindDoctor(id);
            string row[4] = {to_string(id), d->getFname(), d->getLname(), d->getSpecialty()};
            AddRow(id, row, 4);
        }
        if (category == 3) {
            const Nurse* n = hospital.FindNurse(id);
            string row[4] = {to_string(id), n->getFname(), n->getLname(), n->getDepartment()};
            AddRow(id, row, 4);
        }
        if (category == 4) {
            const Room* r = hospital.FindRoom(id);
            int occupied = hospital.RoomOccupancy(id), free = r->getCapacity() - occupied;
            if (onlyAvailable && free == 0) {
                continue;
            }
            string row[5] = {to_string(id), r->getType(), to_string(r->getCapacity()),
                             to_string(occupied), to_string(free)};
            AddRow(id, row, 5);
        }
        if (category == 5) {
            const Appointment* a = hospital.FindAppointment(id);
            if (scheduleType == 1 && a->getPatientId() != scheduleId) {
                continue;
            }
            if (scheduleType == 2 && a->getDoctorId() != scheduleId) {
                continue;
            }
            string row[6] = {to_string(id),
                             to_string(a->getPatientId()),
                             to_string(a->getDoctorId()),
                             a->getDate(),
                             a->getTime(),
                             to_string(a->getDurationMinutes())};
            AddRow(id, row, 6);
        }
    }

    if (category == 5 && scheduleType) {
        SendMessageW(table, LVM_SORTITEMS, 0, (LPARAM)CompareAppointments);
    }
    string counts = "Patients " + to_string(hospital.Count(1)) + "   |   Doctors " +
                    to_string(hospital.Count(2)) + "   |   Nurses " + to_string(hospital.Count(3)) +
                    "   |   Rooms " + to_string(hospital.Count(4)) + "   |   Appointments " +
                    to_string(hospital.Count(5));
    if (scheduleType == 1) {
        counts += "   |   Schedule for patient " + to_string(scheduleId);
    }
    if (scheduleType == 2) {
        counts += "   |   Schedule for doctor " + to_string(scheduleId);
    }
    SetText(countLabel, counts);
    refreshing = false;
}
void LoadRecord(int id) {
    selectedId = id;
    SetText(fieldEdits[0], to_string(id));
    if (category == 1) {
        const Patient* p = hospital.FindPatient(id);
        SetText(fieldEdits[1], p->getFname());
        SetText(fieldEdits[2], p->getLname());
        SetText(fieldEdits[3], to_string(p->getAge()));
        SetText(fieldEdits[4], p->getDiagnosis());
        if (p->getRoomId() == 0) {
            SetText(roomEdit, "");
        } else {
            SetText(roomEdit, to_string(p->getRoomId()));
        }
    }
    if (category == 2) {
        const Doctor* d = hospital.FindDoctor(id);
        SetText(fieldEdits[1], d->getFname());
        SetText(fieldEdits[2], d->getLname());
        SetText(fieldEdits[3], d->getSpecialty());
    }
    if (category == 3) {
        const Nurse* n = hospital.FindNurse(id);
        SetText(fieldEdits[1], n->getFname());
        SetText(fieldEdits[2], n->getLname());
        SetText(fieldEdits[3], n->getDepartment());
    }
    if (category == 4) {
        const Room* r = hospital.FindRoom(id);
        SetText(fieldEdits[1], r->getType());
        SetText(fieldEdits[2], to_string(r->getCapacity()));
    }
    if (category == 5) {
        const Appointment* a = hospital.FindAppointment(id);
        SetText(fieldEdits[1], to_string(a->getPatientId()));
        SetText(fieldEdits[2], to_string(a->getDoctorId()));
        SetText(fieldEdits[3], a->getDate());
        SetText(fieldEdits[4], a->getTime());
        SetText(fieldEdits[5], to_string(a->getDurationMinutes()));
    }
}
bool Exists(int type, int id) {
    if (type == 1) {
        return hospital.FindPatient(id) != nullptr;
    }
    if (type == 2) {
        return hospital.FindDoctor(id) != nullptr;
    }
    if (type == 3) {
        return hospital.FindNurse(id) != nullptr;
    }
    if (type == 4) {
        return hospital.FindRoom(id) != nullptr;
    }
    return hospital.FindAppointment(id) != nullptr;
}
void Save(bool update) {
    string v[6];
    for (int i = 0; i < 6; i++) {
        v[i] = Text(fieldEdits[i]);
    }
    int id;
    if (!Number(v[0], id) || id <= 0) {
        Status(L"Enter a positive whole-number ID.");
        return;
    }
    if (update && selectedId && selectedId != id) {
        Status(L"Keep the selected record ID unchanged.");
        return;
    }
    if (update && !Exists(category, id)) {
        Status(L"Record not found. Select a row or enter an existing ID.");
        return;
    }
    if (!update && Exists(category, id)) {
        Status(L"This ID already exists. Use Update or choose a new ID.");
        return;
    }
    if (!update && hospital.Count(category) == 100) {
        Status(L"This array is full (100 records).");
        return;
    }
    bool ok = false;
    if (category == 1) {
        int age;
        if (!Number(v[3], age)) {
            Status(L"Age must be a whole number from 0 to 120.");
            return;
        }
        Patient p(id, v[1].c_str(), v[2].c_str(), age, v[4].c_str());
        if (!p.IsValid()) {
            Status(
                L"Enter first/last names (max 79 bytes), age 0-120 and diagnosis up to 119 bytes.");
            return;
        }
        if (update) {
            ok = hospital.UpdatePatient(id, &p);
        } else {
            ok = hospital.AddPatient(&p);
        }
    }
    if (category == 2) {
        Doctor d(id, v[1].c_str(), v[2].c_str(), v[3].c_str());
        if (!d.IsValid()) {
            Status(L"First name, last name and specialty are required (max 79 bytes each).");
            return;
        }
        if (update) {
            ok = hospital.UpdateDoctor(id, &d);
        } else {
            ok = hospital.AddDoctor(&d);
        }
    }
    if (category == 3) {
        Nurse n(id, v[1].c_str(), v[2].c_str(), v[3].c_str());
        if (!n.IsValid()) {
            Status(L"First name, last name and department are required (max 79 bytes each).");
            return;
        }
        if (update) {
            ok = hospital.UpdateNurse(id, &n);
        } else {
            ok = hospital.AddNurse(&n);
        }
    }
    if (category == 4) {
        int capacity;
        if (!Number(v[2], capacity)) {
            Status(L"Capacity must be a whole number from 1 to 100.");
            return;
        }
        Room r(id, v[1].c_str(), capacity);
        if (!r.IsValid()) {
            Status(L"Enter a room type (max 79 bytes) and capacity from 1 to 100.");
            return;
        }
        if (update) {
            ok = hospital.UpdateRoom(id, &r);
        } else {
            ok = hospital.AddRoom(&r);
        }
        if (!ok) {
            Status(L"Capacity cannot be smaller than the number of patients in this room.");
            return;
        }
    }
    if (category == 5) {
        int patientId, doctorId, duration;
        if (!Number(v[1], patientId) || !Number(v[2], doctorId) || !Number(v[5], duration)) {
            Status(L"Patient ID, doctor ID and duration must be whole numbers.");
            return;
        }
        Appointment a(id, patientId, doctorId, v[3].c_str(), v[4].c_str(), duration);
        if (!a.IsValid()) {
            Status(L"Use a valid YYYY-MM-DD date, HH:MM time and positive duration ending by "
                   L"midnight.");
            return;
        }
        if (!hospital.FindPatient(patientId) || !hospital.FindDoctor(doctorId)) {
            Status(L"Create the patient and doctor before booking their appointment.");
            return;
        }
        if (update) {
            ok = hospital.UpdateAppointment(id, &a);
        } else {
            ok = hospital.AddAppointment(&a);
        }
        if (!ok) {
            Status(L"This appointment overlaps another appointment for the patient or doctor.");
            return;
        }
    }
    if (ok) {
        searchId = 0;
        scheduleType = 0;
        Refresh();
        LoadRecord(id);
        if (update) {
            PersistChange(L"Record updated and saved.");
        } else {
            PersistChange(L"Record added and saved. Clear the form to add another.");
        }
    } else {
        Status(L"Operation refused. Check the record data.");
    }
}
void DeleteRecord() {
    int id;
    if (!Number(Text(fieldEdits[0]), id) || !Exists(category, id)) {
        Status(L"Enter or select an existing record ID.");
        return;
    }
    if (MessageBoxW(windowHandle, L"Delete this record?", L"Confirm deletion",
                    MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return;
    }
    bool ok = false;
    if (category == 1) {
        ok = hospital.DeletePatient(id);
    }
    if (category == 2) {
        ok = hospital.DeleteDoctor(id);
    }
    if (category == 3) {
        ok = hospital.DeleteNurse(id);
    }
    if (category == 4) {
        ok = hospital.DeleteRoom(id);
    }
    if (category == 5) {
        ok = hospital.DeleteAppointment(id);
    }
    if (ok) {
        searchId = 0;
        scheduleType = 0;
        ClearForm();
        Refresh();
        PersistChange(L"Record deleted and saved.");
    } else if (category == 4) {
        Status(L"Release or transfer the patients before deleting this occupied room.");
    } else {
        Status(L"Cancel this patient's or doctor's appointments before deleting their record.");
    }
}
void Action(int id) {
    if (id == 1001) {
        Save(false);
    }
    if (id == 1002) {
        Save(true);
    }
    if (id == 1003) {
        DeleteRecord();
    }
    if (id == 1004) {
        int recordId;
        if (!Number(Text(fieldEdits[0]), recordId) || !Exists(category, recordId)) {
            Status(L"Record not found. Enter an existing ID.");
            return;
        }
        searchId = recordId;
        scheduleType = 0;
        Refresh();
        LoadRecord(recordId);
        Status(L"Record found.");
    }
    if (id == 1005) {
        ClearForm();
        Status(L"Form cleared. Enter a new record.");
    }
    if (id == 1006) {
        searchId = 0;
        scheduleType = 0;
        SendMessageW(availableButton, BM_SETCHECK, BST_UNCHECKED, 0);
        Refresh();
        Status(L"Showing all records in this category.");
    }
    if (id == 1007) {
        Refresh();
        if (SendMessageW(availableButton, BM_GETCHECK, 0, 0) == BST_CHECKED) {
            Status(L"Showing rooms with available places.");
        } else {
            Status(L"Showing all rooms.");
        }
    }
    if (id == 1008 || id == 1009) {
        int patientId, roomId;
        if (!Number(Text(fieldEdits[0]), patientId)) {
            Status(L"Enter or select a patient ID.");
            return;
        }
        bool ok;
        if (id == 1008) {
            if (!Number(Text(roomEdit), roomId)) {
                Status(L"Enter the room ID to assign.");
                return;
            }
            ok = hospital.AssignRoom(patientId, roomId);
        } else {
            ok = hospital.ReleaseRoom(patientId);
        }
        if (ok) {
            Refresh();
            LoadRecord(patientId);
            PersistChange(L"Patient room assignment updated and saved.");
        } else {
            Status(L"Check that the patient and room exist, and that the room has available "
                   L"capacity.");
        }
    }
    if (id == 1010) {
        int recordId;
        if (!Number(Text(fieldEdits[0]), recordId) || !Exists(category, recordId)) {
            Status(L"Enter or select an existing patient/doctor ID.");
            return;
        }
        int type = category;
        category = 5;
        SendMessageW(categoryBox, CB_SETCURSEL, 4, 0);
        Configure();
        scheduleType = type;
        scheduleId = recordId;
        searchId = 0;
        Refresh();
        Status(
            L"Appointments shown in date/time order. Show all restores the full appointment list.");
    }
    if (id == 1011) {
        SaveRecords(L"All hospital records saved.");
    }
    if (id == 1012) {
        if (MessageBoxW(
                windowHandle,
                L"Reload all records from the saved file? Any unsaved changes will be replaced.",
                L"Reload records", MB_YESNO | MB_ICONQUESTION) != IDYES) {
            return;
        }
        string error;
        if (!hospital.LoadFromFile(dataFile, error)) {
            wstring message =
                L"Could not reload records: " + Wide(error) + L"\nCurrent records are unchanged.";
            Status(L"Reload failed. Current records are unchanged.");
            MessageBoxW(windowHandle, message.c_str(), L"Reload failed", MB_OK | MB_ICONERROR);
            return;
        }
        unsavedChanges = false;
        searchId = 0;
        scheduleType = 0;
        ClearForm();
        Refresh();
        Status(L"All saved records reloaded.");
    }
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
        case WM_CREATE: {
            windowHandle = hwnd;
            normalFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
            titleFont = CreateFontW(-28, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
            headingFont = CreateFontW(-18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
            titleLabel = Control(L"STATIC", L"Hospital  /  Arrays", 0, 1);
            SendMessageW(titleLabel, WM_SETFONT, (WPARAM)titleFont, TRUE);
            wstring subtitle = L"Autosave file: " + dataFile.wstring();
            subtitleLabel = Control(L"STATIC", subtitle.c_str(), 0, 2);
            categoryBox =
                Control(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, 100);
            for (int i = 0; i < 5; i++) {
                SendMessageW(categoryBox, CB_ADDSTRING, 0, (LPARAM)categories[i]);
            }
            SendMessageW(categoryBox, CB_SETCURSEL, 0, 0);
            formTitle = Control(L"STATIC", L"Manage Patients", 0, 3);
            SendMessageW(formTitle, WM_SETFONT, (WPARAM)headingFont, TRUE);
            for (int i = 0; i < 6; i++) {
                fieldLabels[i] = Control(L"STATIC", L"", 0, 200 + i);
                fieldEdits[i] = Control(WC_EDITW, L"", ES_AUTOHSCROLL | WS_TABSTOP, 300 + i);
            }
            addButton = Control(L"BUTTON", L"Add", WS_TABSTOP, 1001);
            updateButton = Control(L"BUTTON", L"Update", WS_TABSTOP, 1002);
            deleteButton = Control(L"BUTTON", L"Delete", WS_TABSTOP, 1003);
            findButton = Control(L"BUTTON", L"Find by ID", WS_TABSTOP, 1004);
            clearButton = Control(L"BUTTON", L"Clear form", WS_TABSTOP, 1005);
            showAllButton = Control(L"BUTTON", L"Show all records", WS_TABSTOP, 1006);
            availableButton =
                Control(L"BUTTON", L"Only available rooms", BS_AUTOCHECKBOX | WS_TABSTOP, 1007);
            roomLabel = Control(L"STATIC", L"Room assignment (selected patient)", 0, 400);
            roomEdit = Control(WC_EDITW, L"", ES_AUTOHSCROLL | WS_TABSTOP, 401);
            assignButton = Control(L"BUTTON", L"Assign / transfer", WS_TABSTOP, 1008);
            releaseButton = Control(L"BUTTON", L"Release room", WS_TABSTOP, 1009);
            scheduleButton = Control(L"BUTTON", L"View appointments", WS_TABSTOP, 1010);
            saveFileButton = Control(L"BUTTON", L"Save records", WS_TABSTOP, 1011);
            reloadFileButton = Control(L"BUTTON", L"Reload records", WS_TABSTOP, 1012);
            countLabel = Control(L"STATIC", L"", 0, 4);
            table = CreateWindowExW(
                WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"Records",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                0, 0, 10, 10, hwnd, (HMENU)500, GetModuleHandleW(nullptr), nullptr);
            SendMessageW(table, WM_SETFONT, (WPARAM)normalFont, TRUE);
            SendMessageW(table, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                         LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
            statusLabel = Control(
                L"STATIC", L"Choose a category. Add a record, or select a row to update it.", 0, 5);
            Configure();
            Refresh();
            if (loadedSavedData) {
                Status(L"Saved records loaded. Changes are saved automatically.");
            } else {
                Status(L"New hospital. Add a record; changes are saved automatically.");
            }
            return 0;
        }
        case WM_SIZE:
            if (table) {
                Layout();
            }
            return 0;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* limits = (MINMAXINFO*)lp;
            limits->ptMinTrackSize.x = 1100;
            limits->ptMinTrackSize.y = 720;
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == 100 && HIWORD(wp) == CBN_SELCHANGE) {
                category = (int)SendMessageW(categoryBox, CB_GETCURSEL, 0, 0) + 1;
                searchId = 0;
                scheduleType = 0;
                Configure();
                Refresh();
                Status(L"Add a record or select a row to edit it.");
            } else if (HIWORD(wp) == BN_CLICKED) {
                Action(LOWORD(wp));
            }
            return 0;
        case WM_NOTIFY: {
            NMHDR* notification = (NMHDR*)lp;
            if (notification->hwndFrom == table && notification->code == LVN_ITEMCHANGED &&
                !refreshing) {
                NMLISTVIEW* change = (NMLISTVIEW*)lp;
                if ((change->uNewState & LVIS_SELECTED) && !(change->uOldState & LVIS_SELECTED)) {
                    LVITEMW item = {};
                    item.mask = LVIF_PARAM;
                    item.iItem = change->iItem;
                    SendMessageW(table, LVM_GETITEMW, 0, (LPARAM)&item);
                    LoadRecord((int)item.lParam);
                    Status(L"Record selected. Edit the form and choose Update.");
                }
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC dc = (HDC)wp;
            HWND control = (HWND)lp;
            if (control == titleLabel || control == subtitleLabel) {
                SetTextColor(dc, RGB(245, 248, 255));
                SetBkColor(dc, RGB(25, 43, 69));
                return (LRESULT)headerBrush;
            }
            if (control == statusLabel) {
                SetTextColor(dc, RGB(22, 80, 110));
            } else {
                SetTextColor(dc, RGB(38, 52, 70));
            }
            SetBkColor(dc, RGB(246, 248, 251));
            return (LRESULT)backgroundBrush;
        }
        case WM_ERASEBKGND: {
            HDC dc = (HDC)wp;
            RECT area;
            GetClientRect(hwnd, &area);
            FillRect(dc, &area, backgroundBrush);
            area.bottom = 90;
            FillRect(dc, &area, headerBrush);
            return 1;
        }
        case WM_CLOSE:
            if (unsavedChanges && !SaveRecords(L"All hospital records saved.")) {
                return 0;
            }
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            DeleteObject(normalFont);
            DeleteObject(titleFont);
            DeleteObject(headingFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int show) {
    if (!InitializeStorage()) {
        return 1;
    }
    INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    backgroundBrush = CreateSolidBrush(RGB(246, 248, 251));
    headerBrush = CreateSolidBrush(RGB(25, 43, 69));
    WNDCLASSW type = {};
    type.lpfnWndProc = WindowProc;
    type.hInstance = instance;
    type.lpszClassName = L"HospitalArraysWindow";
    type.hCursor = LoadCursor(nullptr, IDC_ARROW);
    type.hbrBackground = backgroundBrush;
    RegisterClassW(&type);
    HWND hwnd =
        CreateWindowExW(WS_EX_APPWINDOW, L"HospitalArraysWindow", L"Hospital Management - Arrays",
                        WS_OVERLAPPEDWINDOW, 20, 0, 1240, 740, nullptr, nullptr, instance, nullptr);
    if (!hwnd) {
        return 1;
    }
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hwnd, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    DeleteObject(backgroundBrush);
    DeleteObject(headerBrush);
    return (int)message.wParam;
}
