#include "hospital.h"
#include "gui_input.h"
#include <map>
#include <stdexcept>
int checks = 0;
void Check(bool result) {
    checks++;
    if (!result) {
        cerr << "Failed extended check " << checks << "\n";
        throw runtime_error("Extended test failed");
    }
}
unsigned int seed = 123456789;
int Next(int limit) {
    seed = seed * 1664525u + 1013904223u;
    return (int)((seed >> 8) % limit);
}

void TestInput() {
    int value = 0;
    Check(ParseNumber("0", value) && value == 0);
    Check(ParseNumber("  25  ", value) && value == 25);
    Check(ParseNumber("-2147483648", value) && value == (-2147483647 - 1));
    Check(ParseNumber("2147483647", value) && value == 2147483647);
    for (const char* bad : {"", " ", "1x", "2.5", "1 2", "--1", "2147483648", "-2147483649",
                            "999999999999999999999"}) {
        Check(!ParseNumber(bad, value));
    }
    Check(Utf8ToWide("").empty() && WideToUtf8(L"").empty());
    Check(Utf8ToWide("Ali") == L"Ali");
    wstring arabic = L"\u0639\u0644\u064a \u0645\u062d\u0645\u062f";
    Check(Utf8ToWide(WideToUtf8(arabic)) == arabic);
    wstring emoji = L"\xD83D\xDE00";
    Check(Utf8ToWide(WideToUtf8(emoji)) == emoji);
    Check(Utf8ToWide(string(1, (char)0xFF)).empty());
    Check(WideToUtf8(wstring(1, (wchar_t)0xD800)).empty());
    string name = WideToUtf8(arabic);
    Patient p(1, name.c_str(), "Name", 30, "");
    Check(p.IsValid() && Utf8ToWide(p.getFname()) == arabic);
}
void TestRandomPatient() {
    PatientArray records;
    map<int, Patient> expected;
    for (int step = 0; step < 2000; step++) {
        int id = Next(140) + 1, oldId = Next(140) + 1, op = Next(4), stamp = step;
        string marker = "Value " + to_string(stamp);
        Patient value(id, marker.c_str(), "Name", stamp % 121, marker.c_str());
        if (op == 0) {
            bool ok = expected.size() < 100 && expected.find(id) == expected.end();
            Check(records.InsertPatient(&value) == ok);
            if (ok) {
                expected[id] = value;
            }
        } else if (op == 1) {
            bool ok = expected.find(oldId) != expected.end();
            Check(records.DeleteElement(oldId) == ok);
            if (ok) {
                expected.erase(oldId);
            }
        } else if (op == 2) {
            bool ok = expected.find(oldId) != expected.end() &&
                      (oldId == id || expected.find(id) == expected.end());
            Check(records.ModifyElement(oldId, &value) == ok);
            if (ok) {
                expected.erase(oldId);
                expected[id] = value;
            }
        } else {
            int index = records.FindElement(id);
            Check((index != -1) == (expected.find(id) != expected.end()));
        }
        Check(records.GetElementNb() == (int)expected.size());
        Check(records.IsEmpty() == expected.empty());
        Check(records.IsFull() == (expected.size() == 100));
        Check(!records.GetAt(-1) && !records.GetAt(records.GetElementNb()));
        int i = 0;
        for (const auto& entry : expected) {
            const Patient* actual = records.GetAt(i++);
            Check(actual && actual->getId() == entry.first);
            Check(strcmp(actual->getDiagnosis(), entry.second.getDiagnosis()) == 0);
        }
    }
}
void TestRandomDoctor() {
    DoctorArray records;
    map<int, Doctor> expected;
    for (int step = 0; step < 2000; step++) {
        int id = Next(140) + 1, oldId = Next(140) + 1, op = Next(4), stamp = step;
        string marker = "Value " + to_string(stamp);
        Doctor value(id, "First", "Last", marker.c_str());
        if (op == 0) {
            bool ok = expected.size() < 100 && expected.find(id) == expected.end();
            Check(records.InsertDoctor(&value) == ok);
            if (ok) {
                expected[id] = value;
            }
        } else if (op == 1) {
            bool ok = expected.find(oldId) != expected.end();
            Check(records.DeleteElement(oldId) == ok);
            if (ok) {
                expected.erase(oldId);
            }
        } else if (op == 2) {
            bool ok = expected.find(oldId) != expected.end() &&
                      (oldId == id || expected.find(id) == expected.end());
            Check(records.ModifyElement(oldId, &value) == ok);
            if (ok) {
                expected.erase(oldId);
                expected[id] = value;
            }
        } else {
            int index = records.FindElement(id);
            Check((index != -1) == (expected.find(id) != expected.end()));
        }
        Check(records.GetElementNb() == (int)expected.size());
        Check(records.IsEmpty() == expected.empty());
        Check(records.IsFull() == (expected.size() == 100));
        Check(!records.GetAt(-1) && !records.GetAt(records.GetElementNb()));
        int i = 0;
        for (const auto& entry : expected) {
            const Doctor* actual = records.GetAt(i++);
            Check(actual && actual->getId() == entry.first);
            Check(strcmp(actual->getSpecialty(), entry.second.getSpecialty()) == 0);
        }
    }
}
void TestRandomNurse() {
    NurseArray records;
    map<int, Nurse> expected;
    for (int step = 0; step < 2000; step++) {
        int id = Next(140) + 1, oldId = Next(140) + 1, op = Next(4), stamp = step;
        string marker = "Value " + to_string(stamp);
        Nurse value(id, "First", "Last", marker.c_str());
        if (op == 0) {
            bool ok = expected.size() < 100 && expected.find(id) == expected.end();
            Check(records.InsertNurse(&value) == ok);
            if (ok) {
                expected[id] = value;
            }
        } else if (op == 1) {
            bool ok = expected.find(oldId) != expected.end();
            Check(records.DeleteElement(oldId) == ok);
            if (ok) {
                expected.erase(oldId);
            }
        } else if (op == 2) {
            bool ok = expected.find(oldId) != expected.end() &&
                      (oldId == id || expected.find(id) == expected.end());
            Check(records.ModifyElement(oldId, &value) == ok);
            if (ok) {
                expected.erase(oldId);
                expected[id] = value;
            }
        } else {
            int index = records.FindElement(id);
            Check((index != -1) == (expected.find(id) != expected.end()));
        }
        Check(records.GetElementNb() == (int)expected.size());
        Check(records.IsEmpty() == expected.empty());
        Check(records.IsFull() == (expected.size() == 100));
        Check(!records.GetAt(-1) && !records.GetAt(records.GetElementNb()));
        int i = 0;
        for (const auto& entry : expected) {
            const Nurse* actual = records.GetAt(i++);
            Check(actual && actual->getId() == entry.first);
            Check(strcmp(actual->getDepartment(), entry.second.getDepartment()) == 0);
        }
    }
}
void TestRandomRoom() {
    RoomArray records;
    map<int, Room> expected;
    for (int step = 0; step < 2000; step++) {
        int id = Next(140) + 1, oldId = Next(140) + 1, op = Next(4), stamp = step;
        string marker = "Value " + to_string(stamp);
        Room value(id, marker.c_str(), stamp % 100 + 1);
        if (op == 0) {
            bool ok = expected.size() < 100 && expected.find(id) == expected.end();
            Check(records.InsertRoom(&value) == ok);
            if (ok) {
                expected[id] = value;
            }
        } else if (op == 1) {
            bool ok = expected.find(oldId) != expected.end();
            Check(records.DeleteElement(oldId) == ok);
            if (ok) {
                expected.erase(oldId);
            }
        } else if (op == 2) {
            bool ok = expected.find(oldId) != expected.end() &&
                      (oldId == id || expected.find(id) == expected.end());
            Check(records.ModifyElement(oldId, &value) == ok);
            if (ok) {
                expected.erase(oldId);
                expected[id] = value;
            }
        } else {
            int index = records.FindElement(id);
            Check((index != -1) == (expected.find(id) != expected.end()));
        }
        Check(records.GetElementNb() == (int)expected.size());
        Check(records.IsEmpty() == expected.empty());
        Check(records.IsFull() == (expected.size() == 100));
        Check(!records.GetAt(-1) && !records.GetAt(records.GetElementNb()));
        int i = 0;
        for (const auto& entry : expected) {
            const Room* actual = records.GetAt(i++);
            Check(actual && actual->getId() == entry.first);
            Check(strcmp(actual->getType(), entry.second.getType()) == 0);
        }
    }
}
void TestRandomAppointment() {
    AppointmentArray records;
    map<int, Appointment> expected;
    for (int step = 0; step < 2000; step++) {
        int id = Next(140) + 1, oldId = Next(140) + 1, op = Next(4), stamp = step;
        string marker = "Value " + to_string(stamp);
        Appointment value(id, 1, 1, "2026-11-12", "10:00", stamp % 100 + 1);
        if (op == 0) {
            bool ok = expected.size() < 100 && expected.find(id) == expected.end();
            Check(records.InsertAppointment(&value) == ok);
            if (ok) {
                expected[id] = value;
            }
        } else if (op == 1) {
            bool ok = expected.find(oldId) != expected.end();
            Check(records.DeleteElement(oldId) == ok);
            if (ok) {
                expected.erase(oldId);
            }
        } else if (op == 2) {
            bool ok = expected.find(oldId) != expected.end() &&
                      (oldId == id || expected.find(id) == expected.end());
            Check(records.ModifyElement(oldId, &value) == ok);
            if (ok) {
                expected.erase(oldId);
                expected[id] = value;
            }
        } else {
            int index = records.FindElement(id);
            Check((index != -1) == (expected.find(id) != expected.end()));
        }
        Check(records.GetElementNb() == (int)expected.size());
        Check(records.IsEmpty() == expected.empty());
        Check(records.IsFull() == (expected.size() == 100));
        Check(!records.GetAt(-1) && !records.GetAt(records.GetElementNb()));
        int i = 0;
        for (const auto& entry : expected) {
            const Appointment* actual = records.GetAt(i++);
            Check(actual && actual->getId() == entry.first);
            Check(actual->getDurationMinutes() == entry.second.getDurationMinutes());
        }
    }
}
void TestHospitalLimits() {
    Hospital h;
    for (int id = 1; id <= 100; id++) {
        Patient p(id, "Ali", "Haddad", 30, "");
        Doctor d(id, "Sara", "Khalil", "General");
        Nurse n(id, "Lina", "Salem", "Ward");
        Room r(id, "Ward", 1);
        Appointment a(id, id, id, "2026-11-12", "10:00", 30);
        Check(h.AddPatient(&p));
        Check(h.AddDoctor(&d));
        Check(h.AddNurse(&n));
        Check(h.AddRoom(&r));
        Check(h.AddAppointment(&a));
    }
    Patient p(101, "Ali", "Name", 30, "");
    Doctor d(101, "Sara", "Name", "General");
    Nurse n(101, "Lina", "Name", "Ward");
    Room r(101, "Ward", 1);
    Appointment a(101, 1, 1, "2026-11-13", "10:00", 30);
    Check(!h.AddPatient(&p) && !h.AddDoctor(&d) && !h.AddNurse(&n) && !h.AddRoom(&r) &&
          !h.AddAppointment(&a));
    for (int id = 1; id <= 100; id++) {
        Check(h.GetPatientAt(id - 1)->getId() == id);
        Check(h.GetDoctorAt(id - 1)->getId() == id);
        Check(h.GetNurseAt(id - 1)->getId() == id);
        Check(h.GetRoomAt(id - 1)->getId() == id);
        Check(h.GetAppointmentAt(id - 1)->getId() == id);
    }
    Check(!h.GetPatientAt(-1) && !h.GetPatientAt(100));
    Check(!h.GetDoctorAt(-1) && !h.GetDoctorAt(100));
    Check(!h.GetNurseAt(-1) && !h.GetNurseAt(100));
    Check(!h.GetRoomAt(-1) && !h.GetRoomAt(100));
    Check(!h.GetAppointmentAt(-1) && !h.GetAppointmentAt(100));
    Appointment replacement(50, 50, 50, "2026-11-13", "10:00", 60);
    Check(h.UpdateAppointment(50, &replacement));
    Check(h.FindAppointment(50)->getDurationMinutes() == 60);
    Check(!h.DeletePatient(50) && !h.DeleteDoctor(50));
}
int main() {
    TestInput();
    TestRandomPatient();
    TestRandomDoctor();
    TestRandomNurse();
    TestRandomRoom();
    TestRandomAppointment();
    TestHospitalLimits();
    cout << "Extended tests passed: " << checks << " checks.\n";
}
