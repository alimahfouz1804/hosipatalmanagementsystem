

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <stdexcept>

int dialogAnswer = IDYES;
std::vector<std::wstring> dialogs;
int WINAPI TestMessageBox(HWND, LPCWSTR message, LPCWSTR, UINT flags) {
    dialogs.push_back(message);
    return (flags & MB_YESNO) == MB_YESNO ? dialogAnswer : IDOK;
}
#define MessageBoxW TestMessageBox
#define WinMain HospitalGuiEntryForTests
#include "gui.cpp"
#undef WinMain
#undef MessageBoxW

namespace fs = std::filesystem;
int checks = 0;
void CheckImpl(bool result, int line) {
    checks++;
    if (!result) {
        throw std::runtime_error("GUI persistence check failed at line " + std::to_string(line) +
                                 " (Windows error " + std::to_string(GetLastError()) + ")");
    }
}
#define Check(condition) CheckImpl((condition), __LINE__)

std::string ReadSaved(const fs::path& file) {
    std::ifstream input(file, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}
void OpenTestWindow() {
    refreshing = false;
    category = 1;
    selectedId = 0;
    searchId = 0;
    scheduleType = 0;
    scheduleId = 0;
    HWND testWindow =
        CreateWindowExW(0, L"HospitalGuiPersistenceTest", L"Hospital GUI test", WS_OVERLAPPEDWINDOW,
                        0, 0, 1240, 740, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    Check(testWindow != nullptr);
    Check(IsWindow(saveFileButton) && IsWindow(reloadFileButton));
}
void Fields(int type, const std::vector<std::wstring>& values) {
    category = type;
    searchId = 0;
    scheduleType = 0;
    SendMessageW(categoryBox, CB_SETCURSEL, type - 1, 0);
    Configure();
    Refresh();
    for (size_t i = 0; i < values.size(); i++) {
        SetWindowTextW(fieldEdits[i], values[i].c_str());
    }
}
void CheckSaved() {
    Check(!unsavedChanges);
    Check(Text(statusLabel).find("saved") != std::string::npos);
    Hospital copy;
    std::string error;
    Check(copy.LoadFromFile(dataFile, error));
    for (int type = 1; type <= 5; type++) {
        Check(copy.Count(type) == hospital.Count(type));
    }
    auto comparison = dataFile.parent_path() / "comparison.txt";
    Check(hospital.SaveToFile(comparison, error));
    Check(ReadSaved(dataFile) == ReadSaved(comparison));
}

int main() {
    fs::path folder = fs::current_path() /
                      ("hospital-gui-tests-" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        Check(fs::create_directory(folder));
        dataFile = folder / "hospital_data.txt";
        INITCOMMONCONTROLSEX controls = {sizeof(controls),
                                         ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
        Check(InitCommonControlsEx(&controls) != FALSE);
        backgroundBrush = CreateSolidBrush(RGB(246, 248, 251));
        headerBrush = CreateSolidBrush(RGB(25, 43, 69));
        WNDCLASSW type = {};
        type.lpfnWndProc = WindowProc;
        type.hInstance = GetModuleHandleW(nullptr);
        type.lpszClassName = L"HospitalGuiPersistenceTest";
        type.hbrBackground = backgroundBrush;
        Check(RegisterClassW(&type) != 0);
        OpenTestWindow();
        Check(Text(statusLabel).find("New hospital") != std::string::npos);

        Fields(1, {L"1", L"Ali \"A\"", L"Haddad", L"30", L"Follow-up"});
        Action(1001);
        CheckSaved();
        Fields(2, {L"20", L"Sara", L"Khalil", L"General medicine"});
        Action(1001);
        CheckSaved();
        Fields(3, {L"30", L"Noor", L"Ali", L"Emergency department"});
        Action(1001);
        CheckSaved();
        Fields(4, {L"10", L"Shared ward", L"2"});
        Action(1001);
        CheckSaved();
        Fields(1, {L"1"});
        SetWindowTextW(roomEdit, L"10");
        Action(1008);
        CheckSaved();
        Check(hospital.PatientRoom(1) == 10);
        Fields(5, {L"40", L"1", L"20", L"2026-11-12", L"10:00", L"45"});
        Action(1001);
        CheckSaved();
        Check(hospital.FindAppointment(40)->getDurationMinutes() == 45);

        std::string beforeActions = ReadSaved(dataFile);
        Fields(1, {L"1"});
        Action(1004);
        Check(searchId == 1 && selectedId == 1);
        Check(Text(fieldEdits[1]) == "Ali \"A\"");
        Action(1005);
        Check(selectedId == 0 && Text(fieldEdits[0]).empty());
        Action(1006);
        Check(searchId == 0 && scheduleType == 0);
        Check(ReadSaved(dataFile) == beforeActions);

        LVITEMW selected = {};
        selected.stateMask = LVIS_SELECTED;
        selected.state = LVIS_SELECTED;
        SendMessageW(table, LVM_SETITEMSTATE, 0, (LPARAM)&selected);
        Check(selectedId == 1 && Text(fieldEdits[0]) == "1");

        Fields(1, {L"bad", L"Name", L"Patient", L"30", L""});
        Action(1001);
        Check(Text(statusLabel).find("positive") != std::string::npos);
        Fields(1, {L"2", L"Name", L"Patient", L"121", L""});
        Action(1001);
        Check(hospital.Count(1) == 1 && ReadSaved(dataFile) == beforeActions);
        Fields(2, {L"21", L"Name", L"Doctor", L""});
        Action(1001);
        Check(hospital.Count(2) == 1 && ReadSaved(dataFile) == beforeActions);
        Fields(3, {L"31", L"Name", L"Nurse", L""});
        Action(1001);
        Check(hospital.Count(3) == 1 && ReadSaved(dataFile) == beforeActions);
        Fields(4, {L"11", L"Ward", L"0"});
        Action(1001);
        Check(hospital.Count(4) == 1 && ReadSaved(dataFile) == beforeActions);
        Fields(5, {L"41", L"999", L"20", L"2026-11-12", L"11:00", L"30"});
        Action(1001);
        Check(hospital.Count(5) == 1 && ReadSaved(dataFile) == beforeActions);
        Fields(5, {L"41", L"1", L"20", L"2026-11-12", L"10:15", L"30"});
        Action(1001);
        Check(hospital.Count(5) == 1 && ReadSaved(dataFile) == beforeActions);

        Fields(2, {L"20", L"Sara", L"Khalil", L"Surgery"});
        Action(1002);
        CheckSaved();
        Check(std::string(hospital.FindDoctor(20)->getSpecialty()) == "Surgery");
        Fields(3, {L"30", L"Noor", L"Ali", L"Ward"});
        Action(1002);
        CheckSaved();
        Check(std::string(hospital.FindNurse(30)->getDepartment()) == "Ward");
        Fields(4, {L"10", L"Private room", L"1"});
        Action(1002);
        CheckSaved();
        SendMessageW(availableButton, BM_SETCHECK, BST_CHECKED, 0);
        Action(1007);
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 0);
        Action(1006);
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        Fields(4, {L"10", L"Private room", L"2"});
        Action(1002);
        CheckSaved();
        SendMessageW(availableButton, BM_SETCHECK, BST_CHECKED, 0);
        Action(1007);
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        Fields(5, {L"40", L"1", L"20", L"2026-11-13", L"09:00", L"45"});
        Action(1002);
        CheckSaved();
        Fields(5, {L"41", L"1", L"20", L"2026-11-14", L"08:00", L"30"});
        Action(1001);
        CheckSaved();
        Fields(5, {L"42", L"1", L"20", L"2026-11-12", L"09:00", L"30"});
        Action(1001);
        CheckSaved();

        Fields(1, {L"1"});
        Action(1010);
        Check(category == 5 && scheduleType == 1 && scheduleId == 1);
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 3);
        int orderedIds[3] = {42, 40, 41};
        for (int i = 0; i < 3; i++) {
            LVITEMW item = {};
            item.mask = LVIF_PARAM;
            item.iItem = i;
            SendMessageW(table, LVM_GETITEMW, 0, (LPARAM)&item);
            Check(item.lParam == orderedIds[i]);
        }
        Fields(2, {L"20"});
        Action(1010);
        Check(category == 5 && scheduleType == 2 && scheduleId == 20);
        Action(1006);
        Check(scheduleType == 0 && SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 3);
        Fields(5, {L"41"});
        Action(1003);
        CheckSaved();
        Fields(5, {L"42"});
        Action(1003);
        CheckSaved();
        Check(hospital.Count(5) == 1);

        SendMessageW(categoryBox, CB_SETCURSEL, 3, 0);
        WindowProc(windowHandle, WM_COMMAND, MAKEWPARAM(100, CBN_SELCHANGE), (LPARAM)categoryBox);
        Check(category == 4 && SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        MINMAXINFO minimum = {};
        WindowProc(windowHandle, WM_GETMINMAXINFO, 0, (LPARAM)&minimum);
        Check(minimum.ptMinTrackSize.x == 1100 && minimum.ptMinTrackSize.y == 720);
        Fields(1, {});
        Check(SetWindowPos(windowHandle, nullptr, 0, 0, minimum.ptMinTrackSize.x,
                           minimum.ptMinTrackSize.y, SWP_NOMOVE | SWP_NOZORDER) != FALSE);
        RECT scheduleArea, statusArea;
        GetWindowRect(scheduleButton, &scheduleArea);
        GetWindowRect(statusLabel, &statusArea);
        Check(scheduleArea.bottom + 10 <= statusArea.top);
        WindowProc(windowHandle, WM_SIZE, 0, 0);
        RECT tableArea;
        GetClientRect(table, &tableArea);
        Check(tableArea.right > 0 && tableArea.bottom > 0);

        WindowProc(windowHandle, WM_CLOSE, 0, 0);
        Check(!IsWindow(windowHandle));
        hospital = Hospital();
        std::string error;
        Check(hospital.LoadFromFile(dataFile, error));
        loadedSavedData = true;
        OpenTestWindow();
        Check(Text(statusLabel).find("Saved records loaded") != std::string::npos);
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        Check(hospital.PatientRoom(1) == 10 &&
              hospital.FindAppointment(40)->getDurationMinutes() == 45);
        for (int typeId = 1; typeId <= 5; typeId++) {
            Fields(typeId, {});
            Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        }

        Fields(1, {L"1"});
        std::string beforePatientDeletion = ReadSaved(dataFile);
        dialogAnswer = IDNO;
        Action(1003);
        Check(hospital.FindPatient(1) != nullptr && ReadSaved(dataFile) == beforePatientDeletion);
        dialogAnswer = IDYES;
        Action(1003);
        Check(hospital.FindPatient(1) != nullptr && ReadSaved(dataFile) == beforePatientDeletion);
        Check(Text(statusLabel).find("Cancel") != std::string::npos);

        Fields(1, {L"1", L"Updated", L"Patient", L"31", L"Recovered"});
        Action(1002);
        CheckSaved();
        Check(hospital.PatientRoom(1) == 10 && hospital.FindPatient(1)->getAge() == 31);
        Action(1009);
        CheckSaved();
        Check(hospital.PatientRoom(1) == 0);
        SetWindowTextW(roomEdit, L"10");
        Action(1008);
        CheckSaved();
        auto unchanged = ReadSaved(dataFile);
        Fields(1, {L"1", L"Duplicate", L"Patient", L"31", L""});
        Action(1001);
        Check(ReadSaved(dataFile) == unchanged && !unsavedChanges);
        Fields(2, {L"20"});
        Action(1003);
        Check(hospital.FindDoctor(20) != nullptr && ReadSaved(dataFile) == unchanged);

        Hospital replacement;
        Patient replacementPatient(99, "Reloaded", "Patient", 20, "");
        Check(replacement.AddPatient(&replacementPatient) &&
              replacement.SaveToFile(dataFile, error));
        dialogAnswer = IDNO;
        Action(1012);
        Check(hospital.FindPatient(1) != nullptr && hospital.FindPatient(99) == nullptr);
        dialogAnswer = IDYES;
        Action(1012);
        Check(hospital.FindPatient(1) == nullptr && hospital.FindPatient(99) != nullptr &&
              !unsavedChanges);
        Check(selectedId == 0 && searchId == 0 && scheduleType == 0);
        Fields(1, {});
        Check(SendMessageW(table, LVM_GETITEMCOUNT, 0, 0) == 1);
        {
            std::ofstream invalid(dataFile, std::ios::binary);
            invalid << "broken file\n";
        }
        Action(1012);
        Check(hospital.FindPatient(99) != nullptr && ReadSaved(dataFile) == "broken file\n");
        Check(Text(statusLabel).find("Reload failed") != std::string::npos);
        Action(1011);
        CheckSaved();

        HANDLE lock = CreateFileW(dataFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        Check(lock != INVALID_HANDLE_VALUE);
        unchanged = ReadSaved(dataFile);
        Fields(1, {L"99", L"Unsaved", L"Patient", L"21", L""});
        Action(1002);
        Check(unsavedChanges && ReadSaved(dataFile) == unchanged);
        Check(hospital.FindPatient(99)->getAge() == 21);
        Check(Text(statusLabel).find("Save failed") != std::string::npos);
        WindowProc(windowHandle, WM_CLOSE, 0, 0);
        Check(IsWindow(windowHandle) && unsavedChanges);
        CloseHandle(lock);
        Action(1011);
        CheckSaved();
        Check(ReadSaved(dataFile) != unchanged);

        Fields(1, {L"99"});
        unchanged = ReadSaved(dataFile);
        lock = CreateFileW(dataFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        Check(lock != INVALID_HANDLE_VALUE);
        Action(1003);
        Check(hospital.FindPatient(99) == nullptr && unsavedChanges);
        Check(ReadSaved(dataFile) == unchanged);
        WindowProc(windowHandle, WM_CLOSE, 0, 0);
        Check(IsWindow(windowHandle) && unsavedChanges);
        CloseHandle(lock);
        Action(1011);
        CheckSaved();
        Check(hospital.Count(1) == 0);
        WindowProc(windowHandle, WM_CLOSE, 0, 0);
        Check(!IsWindow(windowHandle));
        Hospital afterDeletion;
        Check(afterDeletion.LoadFromFile(dataFile, error));
        Check(afterDeletion.FindPatient(99) == nullptr && afterDeletion.Count(1) == 0);
        DeleteObject(backgroundBrush);
        DeleteObject(headerBrush);
        for (const auto& entry : fs::directory_iterator(folder)) {
            fs::remove(entry.path());
        }
        fs::remove(folder);
        std::cout << "GUI persistence tests passed: " << checks << " checks.\n";
        return 0;
    } catch (const std::exception& error) {
        if (IsWindow(windowHandle)) {
            DestroyWindow(windowHandle);
        }
        std::cerr << error.what() << "\nTest folder: " << folder.string() << '\n';
        return 1;
    }
}
