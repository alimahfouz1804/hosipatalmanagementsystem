#include "hospital.h"
#include <stdexcept>
#include <sstream>
#include <string>
int checks = 0;
void Check(bool ok) {
    checks++;
    if (!ok) {
        cerr << "Failed check: " << checks << "\\n";
        throw runtime_error("Validation test failed");
    }
}
void TestExtendedValidation() {
    char longName[81];
    for (int i = 0; i < 80; i++) {
        longName[i] = 'A';
    }
    longName[80] = 0;
    Hospital h;
    Patient p(1, "Ali", "Haddad", 30, "");
    Check(h.AddPatient(&p));
    Patient longPatient(2, longName, "Name", 30, "");
    Check(!h.AddPatient(&longPatient));
    Patient badUpdate(1, longName, "Name", 30, "");
    Check(!h.UpdatePatient(1, &badUpdate));
    Patient validUpdate(1, "Ali", "Haddad", 31, "Follow-up");
    Check(h.UpdatePatient(1, &validUpdate));
    Patient wrongId(2, "Ali", "Haddad", 31, "");
    Check(!h.UpdatePatient(1, &wrongId));
    Doctor d(1, "Sara", "Khalil", "General");
    Check(h.AddDoctor(&d));
    Doctor badDoctor(1, " ", "Khalil", "General");
    Check(!h.UpdateDoctor(1, &badDoctor));
    Doctor longDoctor(2, longName, "Name", "General");
    Check(!h.AddDoctor(&longDoctor));
    Nurse n(1, "Lina", "Salem", "Ward");
    Check(h.AddNurse(&n));
    Nurse badNurse(1, "Lina", "Salem", "");
    Check(!h.UpdateNurse(1, &badNurse));
    Nurse longNurse(2, longName, "Salem", "Ward");
    Check(!h.AddNurse(&longNurse));
    Room r(1, "Ward", 2);
    Check(h.AddRoom(&r));
    Room badRoom(1, "Ward", 0);
    Check(!h.UpdateRoom(1, &badRoom));
    Check(h.AssignRoom(1, 1));
    Check(h.UpdatePatient(1, &validUpdate) && h.PatientRoom(1) == 1);
    Check(!h.AddPatient(nullptr) && !h.UpdatePatient(1, nullptr));
}
void TestDurations() {
    Hospital h;
    Patient p1(1, "Ali", "Haddad", 30, ""), p2(2, "Maya", "Nasser", 25, "");
    Doctor d1(1, "Sara", "Khalil", "General"), d2(2, "Lina", "Salem", "General");
    Check(h.AddPatient(&p1) && h.AddPatient(&p2));
    Check(h.AddDoctor(&d1) && h.AddDoctor(&d2));
    Appointment first(1, 1, 1, "2026-11-12", "10:00", 60);
    Check(h.AddAppointment(&first));
    Appointment overlapDoctor(2, 2, 1, "2026-11-12", "10:30", 30);
    Appointment overlapPatient(2, 1, 2, "2026-11-12", "09:45", 30);
    Appointment enclosing(2, 2, 1, "2026-11-12", "09:00", 180);
    Check(!h.AddAppointment(&overlapDoctor));
    Check(!h.AddAppointment(&overlapPatient));
    Check(!h.AddAppointment(&enclosing));
    Appointment adjacent(2, 2, 1, "2026-11-12", "11:00", 30);
    Check(h.AddAppointment(&adjacent));
    Appointment differentDay(3, 1, 1, "2026-11-13", "10:00", 60);
    Check(h.AddAppointment(&differentDay));
    Appointment independent(4, 2, 2, "2026-11-12", "10:00", 30);
    Check(h.AddAppointment(&independent));
    Appointment conflictingUpdate(2, 2, 1, "2026-11-12", "10:59", 30);
    Check(!h.UpdateAppointment(2, &conflictingUpdate));
    Check(h.UpdateAppointment(2, &adjacent));
    Appointment zero(5, 1, 1, "2026-11-12", "12:00", 0);
    Appointment negative(5, 1, 1, "2026-11-12", "12:00", -1);
    Appointment crossesMidnight(5, 1, 1, "2026-11-12", "23:45", 30);
    Check(!h.AddAppointment(&zero) && !h.AddAppointment(&negative));
    Check(!h.AddAppointment(&crossesMidnight));
    Appointment midnightEnd(5, 1, 1, "2026-11-12", "23:30", 30);
    Check(h.AddAppointment(&midnightEnd));
}
void TestCapacity() {
    {
        PatientArray records;
        for (int id = 100; id >= 1; id--) {
            Patient value(id, "Ali", "Haddad", 30, "");
            Check(records.InsertPatient(&value));
        }
        Check(records.IsFull() && records.GetElementNb() == 100);
        Check(records.FindElement(1) == 0 && records.FindElement(100) == 99);
        Patient extra(101, "Ali", "Haddad", 30, "");
        Check(!records.InsertPatient(&extra));
        Patient replacement(50, "Ali", "Haddad", 30, "");
        Check(records.ModifyElement(50, &replacement));
        Check(records.GetElementNb() == 100);
        Check(records.DeleteElement(50));
        Check(records.InsertPatient(&extra));
        records.DeleteAllElement();
        Check(records.IsEmpty());
    }
    {
        DoctorArray records;
        for (int id = 100; id >= 1; id--) {
            Doctor value(id, "Sara", "Khalil", "General");
            Check(records.InsertDoctor(&value));
        }
        Check(records.IsFull() && records.GetElementNb() == 100);
        Check(records.FindElement(1) == 0 && records.FindElement(100) == 99);
        Doctor extra(101, "Sara", "Khalil", "General");
        Check(!records.InsertDoctor(&extra));
        Doctor replacement(50, "Sara", "Khalil", "General");
        Check(records.ModifyElement(50, &replacement));
        Check(records.GetElementNb() == 100);
        Check(records.DeleteElement(50));
        Check(records.InsertDoctor(&extra));
        records.DeleteAllElement();
        Check(records.IsEmpty());
    }
    {
        NurseArray records;
        for (int id = 100; id >= 1; id--) {
            Nurse value(id, "Lina", "Salem", "Ward");
            Check(records.InsertNurse(&value));
        }
        Check(records.IsFull() && records.GetElementNb() == 100);
        Check(records.FindElement(1) == 0 && records.FindElement(100) == 99);
        Nurse extra(101, "Lina", "Salem", "Ward");
        Check(!records.InsertNurse(&extra));
        Nurse replacement(50, "Lina", "Salem", "Ward");
        Check(records.ModifyElement(50, &replacement));
        Check(records.GetElementNb() == 100);
        Check(records.DeleteElement(50));
        Check(records.InsertNurse(&extra));
        records.DeleteAllElement();
        Check(records.IsEmpty());
    }
    {
        RoomArray records;
        for (int id = 100; id >= 1; id--) {
            Room value(id, "Ward", 1);
            Check(records.InsertRoom(&value));
        }
        Check(records.IsFull() && records.GetElementNb() == 100);
        Check(records.FindElement(1) == 0 && records.FindElement(100) == 99);
        Room extra(101, "Ward", 1);
        Check(!records.InsertRoom(&extra));
        Room replacement(50, "Ward", 1);
        Check(records.ModifyElement(50, &replacement));
        Check(records.GetElementNb() == 100);
        Check(records.DeleteElement(50));
        Check(records.InsertRoom(&extra));
        records.DeleteAllElement();
        Check(records.IsEmpty());
    }
    {
        AppointmentArray records;
        for (int id = 100; id >= 1; id--) {
            Appointment value(id, 1, 1, "2026-11-12", "10:00", 30);
            Check(records.InsertAppointment(&value));
        }
        Check(records.IsFull() && records.GetElementNb() == 100);
        Check(records.FindElement(1) == 0 && records.FindElement(100) == 99);
        Appointment extra(101, 1, 1, "2026-11-12", "10:00", 30);
        Check(!records.InsertAppointment(&extra));
        Appointment replacement(50, 1, 1, "2026-11-12", "10:00", 30);
        Check(records.ModifyElement(50, &replacement));
        Check(records.GetElementNb() == 100);
        Check(records.DeleteElement(50));
        Check(records.InsertAppointment(&extra));
        records.DeleteAllElement();
        Check(records.IsEmpty());
    }
}

void TestHospitalOperations() {
    Hospital h;
    Check(h.Count(1) == 0 && h.Count(5) == 0 && h.Count(99) == -1);
    Check(!h.FindPatient(1) && !h.FindDoctor(1) && !h.FindNurse(1));
    Check(!h.FindRoom(1) && !h.FindAppointment(1));
    Check(!h.DeletePatient(1) && !h.DeleteDoctor(1) && !h.DeleteNurse(1));
    Check(!h.DeleteRoom(1) && !h.DeleteAppointment(1));
    Check(!h.AddDoctor(nullptr) && !h.AddNurse(nullptr) && !h.AddRoom(nullptr));
    Check(!h.AddAppointment(nullptr) && !h.UpdateAppointment(1, nullptr));
    Check(!h.UpdateDoctor(1, nullptr) && !h.UpdateNurse(1, nullptr) && !h.UpdateRoom(1, nullptr));
    Check(!h.AssignRoom(1, 1) && !h.ReleaseRoom(1) && h.PatientRoom(1) == -1);
    Patient p(1, "Ali", "Haddad", 30, "");
    Doctor d(1, "Sara", "Khalil", "General");
    Nurse n(1, "Lina", "Salem", "Ward");
    Room r(1, "Ward", 2);
    Check(!h.UpdatePatient(1, &p) && !h.UpdateDoctor(1, &d));
    Check(!h.UpdateNurse(1, &n) && !h.UpdateRoom(1, &r));
    Check(h.AddPatient(&p) && h.AddDoctor(&d) && h.AddNurse(&n) && h.AddRoom(&r));
    Check(!h.AddPatient(&p) && !h.AddDoctor(&d) && !h.AddNurse(&n) && !h.AddRoom(&r));
    Patient pNew(1, "Updated", "Patient", 40, "Recovered");
    Doctor dNew(1, "Updated", "Doctor", "Surgery");
    Nurse nNew(1, "Updated", "Nurse", "Surgery");
    Room rNew(1, "Private", 3);
    Check(h.UpdatePatient(1, &pNew) && h.UpdateDoctor(1, &dNew));
    Check(h.UpdateNurse(1, &nNew) && h.UpdateRoom(1, &rNew));
    Check(h.FindPatient(1)->getAge() == 40);
    Check(strcmp(h.FindDoctor(1)->getSpecialty(), "Surgery") == 0);
    Check(strcmp(h.FindNurse(1)->getDepartment(), "Surgery") == 0);
    Check(h.FindRoom(1)->getCapacity() == 3);
    Check(h.AssignRoom(1, 1) && h.AssignRoom(1, 1) && h.RoomOccupancy(1) == 1);
    Appointment a(1, 1, 1, "2026-11-12", "10:00", 30);
    Check(h.AddAppointment(&a) && !h.AddAppointment(&a));
    Appointment update(1, 1, 1, "2026-11-12", "11:00", 45);
    Check(h.UpdateAppointment(1, &update));
    Check(h.FindAppointment(1)->getDurationMinutes() == 45);
    Appointment missing(1, 99, 1, "2026-11-12", "11:00", 45);
    Check(!h.UpdateAppointment(1, &missing));
    Check(h.FindAppointment(1)->getPatientId() == 1);
    Appointment wrongId(2, 1, 1, "2026-11-12", "11:00", 45);
    Check(!h.UpdateAppointment(1, &wrongId));
    Check(!h.DeleteDoctor(1) && !h.DeletePatient(1) && !h.DeleteRoom(1));
    Check(h.Count(1) == 1 && h.Count(5) == 1 && h.RoomOccupancy(1) == 1);
    Check(h.DeleteAppointment(1) && h.DeletePatient(1));
    Check(h.RoomOccupancy(1) == 0);
    Check(h.DeleteDoctor(1) && h.DeleteNurse(1) && h.DeleteRoom(1));
    for (int type = 1; type <= 5; type++) {
        Check(h.Count(type) == 0);
    }
}
void TestTextAndDateBoundaries() {
    char exact[80], tooLong[81], diagnosis[121];
    for (int i = 0; i < 79; i++) {
        exact[i] = 'X';
    }
    exact[79] = 0;
    for (int i = 0; i < 80; i++) {
        tooLong[i] = 'X';
    }
    tooLong[80] = 0;
    for (int i = 0; i < 120; i++) {
        diagnosis[i] = 'X';
    }
    diagnosis[120] = 0;
    Check(Patient(1, exact, exact, 0, "").IsValid());
    Check(Patient(1, exact, exact, 120, "").IsValid());
    Check(!Patient(1, "Ali", tooLong, 20, "").IsValid());
    Check(!Patient(1, "Ali", "Name", 20, diagnosis).IsValid());
    Check(!Patient(1, nullptr, "Name", 20, "").IsValid());
    Check(!Patient(0, "Ali", "Name", 20, "").IsValid());
    Check(!Patient(-1, "Ali", "Name", 20, "").IsValid());
    Check(!Patient(1, "\t", "Name", 20, "").IsValid());
    Check(!Doctor(1, "Ali", "Name", tooLong).IsValid());
    Check(!Nurse(1, "Ali", "Name", tooLong).IsValid());
    Check(!Room(1, tooLong, 1).IsValid());
    Check(Room(1, "Ward", 100).IsValid() && !Room(1, "Ward", 101).IsValid());
    Check(!Room(1, " ", 1).IsValid());
    Check(!ValidDate(nullptr) && !ValidTime(nullptr));
    Check(ValidDate("2000-02-29") && !ValidDate("1900-02-29"));
    Check(!ValidDate("2026-00-01") && !ValidDate("2026-13-01"));
    Check(!ValidDate("2026-01-00") && !ValidDate("2026-01-32"));
    Check(!ValidDate("2026/01/01") && !ValidDate("2026-1-01"));
    Check(!ValidTime("9:00") && !ValidTime("aa:00") && ValidTime("00:00"));
    Check(!Appointment(1, 1, 1, "2026-11-12suffix", "10:00", 30).IsValid());
    Check(!Appointment(1, 1, 1, "2026-11-12", "10:00suffix", 30).IsValid());
    Check(!Appointment(1, 1, 1, nullptr, "10:00", 30).IsValid());
    Check(!Appointment(1, 1, 1, "2026-11-12", nullptr, 30).IsValid());
    Check(!Appointment(1, 1, 1, "2026-11-12", "00:00", 1441).IsValid());
    Check(Appointment(1, 1, 1, "2026-11-12", "00:00", 1440).IsValid());
}
void TestPatientArrayEdges() {
    PatientArray a;
    Check(!a.GetAt(-1) && !a.GetAt(0));
    Check(!a.InsertPatient(nullptr) && !a.ModifyElement(1, nullptr));
    Check(!a.DeleteAtPosition(-1) && !a.DeleteAtPosition(0));
    Patient first(3, "Ali", "Haddad", 30, ""), middle(2, "Ali", "Haddad", 30, ""),
        last(1, "Ali", "Haddad", 30, "");
    Check(a.InsertPatient(&first) && a.InsertPatient(&middle) && a.InsertPatient(&last));
    Check(a.GetAt(0)->getId() == 1 && a.GetAt(2)->getId() == 3);
    Check(!a.GetAt(3) && !a.DeleteAtPosition(3));
    Check(!a.InsertPatient(&first));
    Check(!a.ModifyElement(1, &middle));
    Check(a.GetElementNb() == 3 && a.FindElement(1) == 0);
    Patient replacement(4, "Ali", "Haddad", 30, "");
    Check(a.ModifyElement(2, &replacement));
    Check(a.FindElement(2) == -1 && a.FindElement(4) == 2);
    const Patient* inside = a.GetAt(0);
    Check(a.ModifyElement(1, inside));
    Check(a.GetElementNb() == 3);
    Check(a.DeleteElement(1) && a.GetAt(0)->getId() == 3);
    Check(a.DeleteElement(4) && a.DeleteElement(3) && a.IsEmpty());
    a.Initialize();
    Check(a.IsEmpty());
}
void TestDoctorArrayEdges() {
    DoctorArray a;
    Check(!a.GetAt(-1) && !a.GetAt(0));
    Check(!a.InsertDoctor(nullptr) && !a.ModifyElement(1, nullptr));
    Check(!a.DeleteAtPosition(-1) && !a.DeleteAtPosition(0));
    Doctor first(3, "Sara", "Khalil", "General"), middle(2, "Sara", "Khalil", "General"),
        last(1, "Sara", "Khalil", "General");
    Check(a.InsertDoctor(&first) && a.InsertDoctor(&middle) && a.InsertDoctor(&last));
    Check(a.GetAt(0)->getId() == 1 && a.GetAt(2)->getId() == 3);
    Check(!a.GetAt(3) && !a.DeleteAtPosition(3));
    Check(!a.InsertDoctor(&first));
    Check(!a.ModifyElement(1, &middle));
    Check(a.GetElementNb() == 3 && a.FindElement(1) == 0);
    Doctor replacement(4, "Sara", "Khalil", "General");
    Check(a.ModifyElement(2, &replacement));
    Check(a.FindElement(2) == -1 && a.FindElement(4) == 2);
    const Doctor* inside = a.GetAt(0);
    Check(a.ModifyElement(1, inside));
    Check(a.GetElementNb() == 3);
    Check(a.DeleteElement(1) && a.GetAt(0)->getId() == 3);
    Check(a.DeleteElement(4) && a.DeleteElement(3) && a.IsEmpty());
    a.Initialize();
    Check(a.IsEmpty());
}
void TestNurseArrayEdges() {
    NurseArray a;
    Check(!a.GetAt(-1) && !a.GetAt(0));
    Check(!a.InsertNurse(nullptr) && !a.ModifyElement(1, nullptr));
    Check(!a.DeleteAtPosition(-1) && !a.DeleteAtPosition(0));
    Nurse first(3, "Lina", "Salem", "Ward"), middle(2, "Lina", "Salem", "Ward"),
        last(1, "Lina", "Salem", "Ward");
    Check(a.InsertNurse(&first) && a.InsertNurse(&middle) && a.InsertNurse(&last));
    Check(a.GetAt(0)->getId() == 1 && a.GetAt(2)->getId() == 3);
    Check(!a.GetAt(3) && !a.DeleteAtPosition(3));
    Check(!a.InsertNurse(&first));
    Check(!a.ModifyElement(1, &middle));
    Check(a.GetElementNb() == 3 && a.FindElement(1) == 0);
    Nurse replacement(4, "Lina", "Salem", "Ward");
    Check(a.ModifyElement(2, &replacement));
    Check(a.FindElement(2) == -1 && a.FindElement(4) == 2);
    const Nurse* inside = a.GetAt(0);
    Check(a.ModifyElement(1, inside));
    Check(a.GetElementNb() == 3);
    Check(a.DeleteElement(1) && a.GetAt(0)->getId() == 3);
    Check(a.DeleteElement(4) && a.DeleteElement(3) && a.IsEmpty());
    a.Initialize();
    Check(a.IsEmpty());
}
void TestRoomArrayEdges() {
    RoomArray a;
    Check(!a.GetAt(-1) && !a.GetAt(0));
    Check(!a.InsertRoom(nullptr) && !a.ModifyElement(1, nullptr));
    Check(!a.DeleteAtPosition(-1) && !a.DeleteAtPosition(0));
    Room first(3, "Ward", 1), middle(2, "Ward", 1), last(1, "Ward", 1);
    Check(a.InsertRoom(&first) && a.InsertRoom(&middle) && a.InsertRoom(&last));
    Check(a.GetAt(0)->getId() == 1 && a.GetAt(2)->getId() == 3);
    Check(!a.GetAt(3) && !a.DeleteAtPosition(3));
    Check(!a.InsertRoom(&first));
    Check(!a.ModifyElement(1, &middle));
    Check(a.GetElementNb() == 3 && a.FindElement(1) == 0);
    Room replacement(4, "Ward", 1);
    Check(a.ModifyElement(2, &replacement));
    Check(a.FindElement(2) == -1 && a.FindElement(4) == 2);
    const Room* inside = a.GetAt(0);
    Check(a.ModifyElement(1, inside));
    Check(a.GetElementNb() == 3);
    Check(a.DeleteElement(1) && a.GetAt(0)->getId() == 3);
    Check(a.DeleteElement(4) && a.DeleteElement(3) && a.IsEmpty());
    a.Initialize();
    Check(a.IsEmpty());
}
void TestAppointmentArrayEdges() {
    AppointmentArray a;
    Check(!a.GetAt(-1) && !a.GetAt(0));
    Check(!a.InsertAppointment(nullptr) && !a.ModifyElement(1, nullptr));
    Check(!a.DeleteAtPosition(-1) && !a.DeleteAtPosition(0));
    Appointment first(3, 1, 1, "2026-11-12", "10:00", 30),
        middle(2, 1, 1, "2026-11-12", "10:00", 30), last(1, 1, 1, "2026-11-12", "10:00", 30);
    Check(a.InsertAppointment(&first) && a.InsertAppointment(&middle) &&
          a.InsertAppointment(&last));
    Check(a.GetAt(0)->getId() == 1 && a.GetAt(2)->getId() == 3);
    Check(!a.GetAt(3) && !a.DeleteAtPosition(3));
    Check(!a.InsertAppointment(&first));
    Check(!a.ModifyElement(1, &middle));
    Check(a.GetElementNb() == 3 && a.FindElement(1) == 0);
    Appointment replacement(4, 1, 1, "2026-11-12", "10:00", 30);
    Check(a.ModifyElement(2, &replacement));
    Check(a.FindElement(2) == -1 && a.FindElement(4) == 2);
    const Appointment* inside = a.GetAt(0);
    Check(a.ModifyElement(1, inside));
    Check(a.GetElementNb() == 3);
    Check(a.DeleteElement(1) && a.GetAt(0)->getId() == 3);
    Check(a.DeleteElement(4) && a.DeleteElement(3) && a.IsEmpty());
    a.Initialize();
    Check(a.IsEmpty());
}
void TestDisplayAndSchedule() {
    Hospital h;
    Patient p(1, "Ali", "Haddad", 30, "");
    Doctor d(1, "Sara", "Khalil", "General");
    Check(h.AddPatient(&p) && h.AddDoctor(&d));
    Appointment later(1, 1, 1, "2026-11-13", "11:00", 30);
    Appointment early(3, 1, 1, "2026-11-12", "09:00", 30);
    Appointment mid(2, 1, 1, "2026-11-12", "10:00", 30);
    Check(h.AddAppointment(&later) && h.AddAppointment(&early) && h.AddAppointment(&mid));
    stringstream output;
    streambuf* previous = cout.rdbuf(output.rdbuf());
    bool doctorOk = h.DisplaySchedule(1, true);
    bool patientOk = h.DisplaySchedule(1, false);
    bool missing = h.DisplaySchedule(99, true);
    bool search = h.Search(1, 1);
    bool absent = h.Search(1, 99);
    for (int type = 1; type <= 5; type++) {
        h.DisplayCategory(type);
    }
    h.DisplayAvailableRooms();
    h.DisplaySummary();
    h.Display();
    cout.rdbuf(previous);
    Check(doctorOk && patientOk && !missing && search && !absent);
    string text = output.str();
    Check(text.find("2026-11-12 / time: 09:00") < text.find("2026-11-12 / time: 10:00"));
    Check(text.find("2026-11-12 / time: 10:00") < text.find("2026-11-13 / time: 11:00"));
    Check(text.find("No available rooms.") != (size_t)-1);
    Check(h.FindAppointment(1)->getId() == 1 && h.Count(5) == 3);
}

int main() {
    TestHospitalOperations();
    TestTextAndDateBoundaries();
    TestPatientArrayEdges();
    TestDoctorArrayEdges();
    TestNurseArrayEdges();
    TestRoomArrayEdges();
    TestAppointmentArrayEdges();
    TestDisplayAndSchedule();

    TestExtendedValidation();
    TestDurations();
    TestCapacity();
    Hospital h;
    Patient p(1, "Ali", "Haddad", 30, "");
    Patient p2(2, "Maya", "Nasser", 25, "");
    Patient bad(3, "  ", "Name", 20, "");
    Patient badAge(3, "Ali", "Name", -1, "");
    Patient tooOld(3, "Ali", "Name", 121, "");
    Check(!h.AddPatient(&bad) && !h.AddPatient(&badAge) && !h.AddPatient(&tooOld));
    Check(h.AddPatient(&p) && h.AddPatient(&p2));
    Doctor d(1, "Sara", "Khalil", "Cardiology");
    Doctor d2(2, "Lina", "Salem", "General");
    Doctor blank(3, "", "Salem", "General");
    Check(!h.AddDoctor(&blank));
    Check(h.AddDoctor(&d) && h.AddDoctor(&d2));
    Room r(1, "Ward", 1), r2(2, "Ward", 2), invalid(3, "Ward", 0);
    Check(!h.AddRoom(&invalid));
    Check(h.AddRoom(&r) && h.AddRoom(&r2));
    Check(h.AssignRoom(1, 1));
    Check(!h.AssignRoom(2, 1));
    Check(!h.AssignRoom(99, 1) && !h.AssignRoom(1, 99));
    Check(h.AssignRoom(1, 2) && h.RoomOccupancy(1) == 0);
    Check(h.AssignRoom(2, 2) && h.RoomOccupancy(2) == 2);
    Room smaller(2, "Ward", 1);
    Check(!h.UpdateRoom(2, &smaller) && !h.DeleteRoom(2));
    Check(h.ReleaseRoom(1) && h.PatientRoom(1) == 0);
    Check(h.UpdateRoom(2, &smaller));
    Check(ValidDate("2024-02-29") && !ValidDate("2025-02-29"));
    Check(!ValidDate("2026-04-31") && !ValidDate("0000-01-01"));
    Check(ValidTime("23:59") && !ValidTime("24:00") && !ValidTime("10:60"));
    Appointment a(1, 1, 1, "2026-11-12", "10:00");
    Appointment clashDoctor(2, 2, 1, "2026-11-12", "10:00");
    Appointment clashPatient(2, 1, 2, "2026-11-12", "10:00");
    Appointment invalidDate(2, 2, 2, "2026-02-30", "10:00");
    Appointment missing(2, 99, 2, "2026-11-12", "11:00");
    Appointment later(2, 2, 2, "2026-11-12", "11:00");
    Check(h.AddAppointment(&a));
    Check(!h.AddAppointment(&clashDoctor) && !h.AddAppointment(&clashPatient));
    Check(!h.AddAppointment(&invalidDate) && !h.AddAppointment(&missing));
    Check(h.AddAppointment(&later));
    Appointment conflictUpdate(2, 2, 1, "2026-11-12", "10:00");
    Check(!h.UpdateAppointment(2, &conflictUpdate));
    Check(h.UpdateAppointment(1, &a));
    Check(!h.DeletePatient(1) && !h.DeleteDoctor(1));
    Check(h.DeleteAppointment(1));
    Check(h.DeletePatient(1) && h.DeleteDoctor(1));
    Check(h.DeleteAppointment(2) && h.DeletePatient(2));
    Check(h.RoomOccupancy(2) == 0 && h.DeleteRoom(2));
    cout << "All tests passed: " << checks << " checks.\n";
}
