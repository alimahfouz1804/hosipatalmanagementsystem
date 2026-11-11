#include "hospital.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace fs = std::filesystem;
int checks = 0;
void CheckImpl(bool ok, int line) {
    checks++;
    if (!ok) {
        throw std::runtime_error("Failed persistence check at line " + std::to_string(line));
    }
}
#define Check(condition) CheckImpl((condition), __LINE__)

class TestFiles {
public:
    fs::path folder;
    TestFiles() {
        folder = fs::current_path() /
                 ("hospital-file-tests-" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!fs::create_directory(folder)) {
            throw std::runtime_error("Cannot create test folder");
        }
    }
    ~TestFiles() {
        std::error_code ignored;
        for (const auto& entry : fs::directory_iterator(folder, ignored)) {
            fs::remove(entry.path(), ignored);
        }
        fs::remove(folder, ignored);
    }
    fs::path File(const char* name) const {
        return folder / name;
    }
};

std::string ReadFile(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot read test file");
    }
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}
void WriteFile(const fs::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    output.close();
    if (!output) {
        throw std::runtime_error("Cannot write test file");
    }
}
std::string Replace(std::string text, const std::string& from, const std::string& to) {
    size_t position = text.find(from);
    if (position == std::string::npos) {
        throw std::runtime_error("Missing test fixture text");
    }
    text.replace(position, from.size(), to);
    return text;
}

void Populate(Hospital& h) {
    Patient p1(1, "Ali \"A\"", "Van Haddad", 30, "Follow-up: C:\\reports\nSecond line\tcheck"),
        p2(2, "Maya", "Nasser", 0, ""), p3(3, "Omar", "Salem", 120, "Recovered");
    Doctor d1(20, "Sara", "Khalil", "General medicine"), d2(21, "Lina", "Salem", "Surgery");
    Nurse n(30, "Noor", "Abou Ali", "Emergency department");
    Room r(10, "Shared ward", 2);
    Appointment a1(40, 1, 20, "2026-11-12", "10:00", 60), a2(41, 2, 21, "2026-11-12", "10:00", 45);
    Check(h.AddPatient(&p3) && h.AddPatient(&p1) && h.AddPatient(&p2));
    Check(h.AddDoctor(&d2) && h.AddDoctor(&d1) && h.AddNurse(&n) && h.AddRoom(&r));
    Check(h.AssignRoom(1, 10) && h.AssignRoom(2, 10));
    Check(h.AddAppointment(&a2) && h.AddAppointment(&a1));
}

void TestRoundTripAndChanges(const TestFiles& files) {
    Hospital original, restored;
    std::string error = "old error";
    Populate(original);
    auto saved = files.File("roundtrip.txt");
    auto copy = files.File("copy.txt");
    Check(original.SaveToFile(saved, error) && error.empty());
    Check(restored.LoadFromFile(saved, error) && error.empty());
    Check(restored.Count(1) == 3 && restored.Count(2) == 2 && restored.Count(3) == 1);
    Check(restored.Count(4) == 1 && restored.Count(5) == 2);
    Check(restored.PatientRoom(1) == 10 && restored.PatientRoom(2) == 10 &&
          restored.PatientRoom(3) == 0);
    Check(restored.RoomOccupancy(10) == 2);
    Check(std::string(restored.FindPatient(1)->getFname()) == "Ali \"A\"");
    Check(std::string(restored.FindPatient(1)->getDiagnosis()) ==
          "Follow-up: C:\\reports\nSecond line\tcheck");
    Check(restored.FindPatient(2)->getAge() == 0 && restored.FindPatient(3)->getAge() == 120);
    Check(std::string(restored.FindDoctor(20)->getSpecialty()) == "General medicine");
    Check(std::string(restored.FindNurse(30)->getDepartment()) == "Emergency department");
    Check(std::string(restored.FindRoom(10)->getType()) == "Shared ward");
    Check(restored.FindAppointment(40)->getDurationMinutes() == 60);
    Check(restored.SaveToFile(copy, error) && ReadFile(saved) == ReadFile(copy));
    Check(!restored.DeleteDoctor(20) && !restored.DeletePatient(1) && !restored.DeleteRoom(10));
    Patient changed(1, "Updated", "Patient", 31, "Discharged");
    Check(restored.UpdatePatient(1, &changed) && restored.ReleaseRoom(1));
    Check(restored.DeleteAppointment(40) && restored.DeleteDoctor(20));
    Check(restored.DeleteAppointment(41) && restored.DeletePatient(2) && restored.DeleteNurse(30));
    Check(restored.SaveToFile(saved, error));
    Check(original.LoadFromFile(saved, error));
    Check(original.Count(1) == 2 && original.Count(2) == 1 && original.Count(3) == 0 &&
          original.Count(5) == 0);
    Check(!original.FindPatient(2) && !original.FindDoctor(20) && !original.FindNurse(30));
    Check(original.PatientRoom(1) == 0 && original.RoomOccupancy(10) == 0);
    Check(std::string(original.FindPatient(1)->getFname()) == "Updated");
    Hospital empty;
    Check(empty.SaveToFile(saved, error) && original.LoadFromFile(saved, error));
    for (int type = 1; type <= 5; type++) {
        Check(original.Count(type) == 0);
    }
}

void TestCapacityAndLongText(const TestFiles& files) {
    Hospital full, restored;
    std::string error, name(79, 'X'), diagnosis(119, 'D');
    for (int id = 100; id >= 1; id--) {
        Patient p(id, name.c_str(), name.c_str(), 50, diagnosis.c_str());
        Doctor d(id, name.c_str(), name.c_str(), name.c_str());
        Nurse n(id, name.c_str(), name.c_str(), name.c_str());
        Room r(id, name.c_str(), 1);
        Appointment a(id, id, id, "2026-11-12", "23:30", 30);
        Check(full.AddPatient(&p) && full.AddDoctor(&d) && full.AddNurse(&n) && full.AddRoom(&r));
        Check(full.AssignRoom(id, id) && full.AddAppointment(&a));
    }
    auto saved = files.File("capacity.txt"), copy = files.File("capacity-copy.txt");
    Check(full.SaveToFile(saved, error) && restored.LoadFromFile(saved, error));
    for (int type = 1; type <= 5; type++) {
        Check(restored.Count(type) == 100);
    }
    Check(restored.SaveToFile(copy, error) && ReadFile(saved) == ReadFile(copy));
    for (int id = 1; id <= 100; id++) {
        Check(restored.PatientRoom(id) == id && restored.RoomOccupancy(id) == 1);
        Check(std::string(restored.FindPatient(id)->getDiagnosis()) == diagnosis);
    }
    Patient overflow(101, "Extra", "Patient", 20, "");
    Check(!restored.AddPatient(&overflow));
}

const std::string valid =
    "HOSPITAL_OOP_ARRAYS 1\n"
    "ROOMS 1\n10 \"Shared ward\" 2\n"
    "DOCTORS 2\n20 \"Sara\" \"Khalil\" \"General\"\n21 \"Lina\" \"Salem\" \"Surgery\"\n"
    "NURSES 1\n30 \"Noor\" \"Ali\" \"Ward\"\n"
    "PATIENTS 2\n1 \"Ali\" \"Haddad\" 30 \"\" 10\n2 \"Maya\" \"Nasser\" 25 \"Checkup\" 10\n"
    "APPOINTMENTS 2\n40 1 20 \"2026-11-12\" \"10:00\" 60\n41 2 21 \"2026-11-12\" \"10:00\" "
    "30\nEND\n";

void TestInvalidFiles(const TestFiles& files) {
    Hospital target;
    Populate(target);
    std::string error;
    auto bad = files.File("invalid.txt"), before = files.File("before.txt"),
         after = files.File("after.txt");
    Check(target.SaveToFile(before, error));
    const std::string unchanged = ReadFile(before);
    auto Reject = [&](const std::string& content) {
        WriteFile(bad, content);
        Check(!target.LoadFromFile(bad, error) && !error.empty());
        Check(ReadFile(bad) == content);
        Check(target.SaveToFile(after, error) && ReadFile(after) == unchanged);
    };
    Reject("");
    Reject("unrelated file\n");
    Reject(Replace(valid, "HOSPITAL_OOP_ARRAYS 1", "HOSPITAL_OOP_ARRAYS 99"));
    Reject(Replace(valid, "ROOMS 1", "ROOMS -1"));
    Reject(Replace(valid, "DOCTORS 2", "DOCTORS 101"));
    Reject(Replace(valid, "NURSES 1", "NURSES 9999999999999999999999"));
    Reject(Replace(valid, "PATIENTS 2", "PATIENTS 2abc"));
    Reject(Replace(valid, "ROOMS", "PATIENTS"));
    Reject(Replace(valid, "END\n", ""));
    Reject(valid + "unexpected trailing data\n");
    Reject(valid.substr(0, valid.find("APPOINTMENTS") + 20));
    Reject(Replace(valid, "10 \"Shared ward\" 2", "10 \"Shared ward\" 1"));
    Reject(Replace(valid, "10 \"Shared ward\" 2", "10 \"Shared ward\" 0"));
    Reject(Replace(valid, "ROOMS 1\n10 \"Shared ward\" 2",
                   "ROOMS 2\n10 \"Shared ward\" 2\n10 \"Duplicate\" 2"));
    Reject(Replace(valid, "21 \"Lina\"", "20 \"Lina\""));
    Reject(Replace(valid, "NURSES 1\n30 \"Noor\" \"Ali\" \"Ward\"",
                   "NURSES 2\n30 \"Noor\" \"Ali\" \"Ward\"\n30 \"Noor\" \"Ali\" \"Ward\""));
    Reject(Replace(valid, "2 \"Maya\"", "1 \"Maya\""));
    Reject(Replace(valid, "41 2 21", "40 2 21"));
    Reject(Replace(valid, "30 \"\" 10", "121 \"\" 10"));
    Reject(Replace(valid, "30 \"\" 10", "30 \"\" 999"));
    Reject(Replace(valid, "30 \"\" 10", "30 \"\" -1"));
    Reject(Replace(valid, "\"Ali\" \"Haddad\"", "\" \" \"Haddad\""));
    Reject(Replace(valid, "\"Sara\"", "\"" + std::string(80, 'S') + "\""));
    Reject(Replace(valid, "\"Checkup\"", "\"" + std::string(120, 'D') + "\""));
    Reject(Replace(valid, "\"Sara\"", "Sara"));
    Reject(Replace(valid, "\"Sara\"", "\"Sara\"extra"));
    Reject(Replace(valid, "\"Sara\"", std::string("\"Sa\0ra\"", 7)));
    Reject(Replace(valid, "\"Surgery\"", "\"\""));
    Reject(Replace(valid, "\"Ward\"", "\"\""));
    Reject(Replace(valid, "40 1 20", "40 99 20"));
    Reject(Replace(valid, "40 1 20", "40 1 99"));
    Reject(Replace(valid, "41 2 21", "41 2 20"));
    Reject(Replace(valid, "41 2 21", "41 1 21"));
    Reject(Replace(valid, "2026-11-12", "2026-02-30"));
    Reject(Replace(valid, "\"10:00\" 60", "\"23:45\" 60"));
    Reject(Replace(valid, "\"10:00\" 60", "\"10:00\" 0"));
    Reject(Replace(valid, "\"10:00\" 60", "\"24:00\" 60"));
    Reject(std::string(1024 * 1024 + 1, 'X'));

    std::string windows;
    for (char c : valid) {
        if (c == '\n') {
            windows += '\r';
        }
        windows += c;
    }
    WriteFile(bad, windows + "\r\n \t");
    Check(target.LoadFromFile(bad, error) && error.empty());
    Check(target.Count(1) == 2 && target.RoomOccupancy(10) == 2);
}

void TestFileErrors(const TestFiles& files) {
    Hospital h;
    Populate(h);
    std::string error;
    auto missing = files.File("missing-parent") / "data.txt";
    Check(!h.SaveToFile(missing, error) && !error.empty());
    Check(!h.LoadFromFile(missing, error) && !error.empty());
    Check(!h.SaveToFile(fs::path(), error) && !error.empty());
    auto directory = files.File("directory");
    Check(fs::create_directory(directory));
    Check(!h.SaveToFile(directory, error) && fs::is_directory(directory));
    Check(!h.LoadFromFile(directory, error));
    auto saved = files.File("locked.txt");
    Check(h.SaveToFile(saved, error));
    fs::path oldTemporary = saved;
    oldTemporary += ".tmp";
    fs::path otherTemporary = saved;
    otherTemporary += ".tmp.1";
    WriteFile(oldTemporary, "previous temporary data");
    WriteFile(otherTemporary, "another temporary file");
    Check(h.SaveToFile(saved, error));
    Check(ReadFile(oldTemporary) == "previous temporary data");
    Check(ReadFile(otherTemporary) == "another temporary file");
    fs::path newTemporary = saved;
    newTemporary += ".tmp.2";
    Check(!fs::exists(newTemporary));
    Check(fs::remove(oldTemporary) && fs::remove(otherTemporary));
    const auto previous = ReadFile(saved);
    Patient change(1, "Changed", "Patient", 40, "");
    Check(h.UpdatePatient(1, &change));
#ifdef _WIN32
    HANDLE lock = CreateFileW(saved.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    Check(lock != INVALID_HANDLE_VALUE);
    bool savedWhileLocked = h.SaveToFile(saved, error);
    CloseHandle(lock);
    Check(!savedWhileLocked && !error.empty());
    Check(ReadFile(saved) == previous);
    for (const auto& entry : fs::directory_iterator(files.folder)) {
        Check(entry.path().filename().string().find(".tmp") == std::string::npos);
    }
#endif
    Check(h.SaveToFile(saved, error) && error.empty());
    Check(ReadFile(saved) != previous);

    auto unicode = files.folder / fs::path(L"hospital-\u0645\u0644\u0641.txt");
    Hospital copy;
    Check(h.SaveToFile(unicode, error) && copy.LoadFromFile(unicode, error));
    Check(std::string(copy.FindPatient(1)->getFname()) == "Changed");
}

int main() {
    try {
        TestFiles files;
        TestRoundTripAndChanges(files);
        TestCapacityAndLongText(files);
        TestInvalidFiles(files);
        TestFileErrors(files);
        std::cout << "All persistence tests passed: " << checks << " checks.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
