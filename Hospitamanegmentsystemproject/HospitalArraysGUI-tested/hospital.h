#pragma once
#include "patientarray.h"
#include "doctorarray.h"
#include "nursearray.h"
#include "roomarray.h"
#include "appointmentarray.h"
#include <filesystem>
#include <string>

class Hospital {
private:
    PatientArray patients;
    DoctorArray doctors;
    NurseArray nurses;
    RoomArray rooms;
    AppointmentArray appointments;

    bool HasAppointment(int id, bool isPatient) const {
        for (int i = 0; i < appointments.GetElementNb(); i++) {
            const Appointment* a = appointments.GetAt(i);
            if ((isPatient && a->getPatientId() == id) || (!isPatient && a->getDoctorId() == id)) {
                return true;
            }
        }
        return false;
    }
    bool CanBook(const Appointment* value, int ignoreId) {
        if (!value || !value->IsValid()) {
            return false;
        }
        if (!patients.IsInArray(value->getPatientId()) ||
            !doctors.IsInArray(value->getDoctorId())) {
            return false;
        }
        for (int i = 0; i < appointments.GetElementNb(); i++) {
            const Appointment* a = appointments.GetAt(i);
            if (a->getId() == ignoreId) {
                continue;
            }
            int start = value->getStartMinutes();
            int end = start + value->getDurationMinutes();
            int otherStart = a->getStartMinutes();
            int otherEnd = otherStart + a->getDurationMinutes();

            bool overlap =
                strcmp(a->getDate(), value->getDate()) == 0 && start < otherEnd && otherStart < end;
            if (overlap && (a->getDoctorId() == value->getDoctorId() ||
                            a->getPatientId() == value->getPatientId())) {
                return false;
            }
        }
        return true;
    }

public:
    bool SaveToFile(const std::filesystem::path& filename, std::string& error) const;
    bool LoadFromFile(const std::filesystem::path& filename, std::string& error);

    const Patient* GetPatientAt(int index) const {
        return patients.GetAt(index);
    }
    const Doctor* GetDoctorAt(int index) const {
        return doctors.GetAt(index);
    }
    const Nurse* GetNurseAt(int index) const {
        return nurses.GetAt(index);
    }
    const Room* GetRoomAt(int index) const {
        return rooms.GetAt(index);
    }
    const Appointment* GetAppointmentAt(int index) const {
        return appointments.GetAt(index);
    }
    bool AddPatient(const Patient* value) {
        if (value == nullptr) {
            return false;
        }
        if (value->getRoomId() != 0) {
            return false;
        }
        return patients.InsertPatient(value);
    }
    bool AddDoctor(const Doctor* value) {
        return doctors.InsertDoctor(value);
    }
    bool AddNurse(const Nurse* value) {
        return nurses.InsertNurse(value);
    }
    bool AddRoom(const Room* value) {
        return rooms.InsertRoom(value);
    }
    bool UpdatePatient(int id, const Patient* value) {
        if (!value || value->getId() != id) {
            return false;
        }
        int pos = patients.FindElement(id);
        if (pos == -1) {
            return false;
        }
        Patient copy = *value;
        copy.SetRoomId(patients.GetAt(pos)->getRoomId());
        return patients.ModifyElement(id, &copy);
    }
    bool UpdateDoctor(int id, const Doctor* value) {
        if (value == nullptr) {
            return false;
        }
        if (value->getId() != id) {
            return false;
        }
        return doctors.ModifyElement(id, value);
    }
    bool UpdateNurse(int id, const Nurse* value) {
        if (value == nullptr) {
            return false;
        }
        if (value->getId() != id) {
            return false;
        }
        return nurses.ModifyElement(id, value);
    }
    int RoomOccupancy(int roomId) const {
        int count = 0;
        for (int i = 0; i < patients.GetElementNb(); i++) {
            if (patients.GetAt(i)->getRoomId() == roomId) {
                count++;
            }
        }
        return count;
    }
    bool UpdateRoom(int id, const Room* value) {
        if (!value || value->getId() != id || value->getCapacity() < RoomOccupancy(id)) {
            return false;
        }
        return rooms.ModifyElement(id, value);
    }
    bool AssignRoom(int patientId, int roomId) {
        int p = patients.FindElement(patientId);
        int r = rooms.FindElement(roomId);
        if (p == -1 || r == -1) {
            return false;
        }
        if (patients.GetAt(p)->getRoomId() == roomId) {
            return true;
        }
        if (RoomOccupancy(roomId) >= rooms.GetAt(r)->getCapacity()) {
            return false;
        }
        Patient copy = *patients.GetAt(p);
        copy.SetRoomId(roomId);
        return patients.ModifyElement(patientId, &copy);
    }
    bool ReleaseRoom(int patientId) {
        int p = patients.FindElement(patientId);
        if (p == -1) {
            return false;
        }
        Patient copy = *patients.GetAt(p);
        copy.SetRoomId(0);
        return patients.ModifyElement(patientId, &copy);
    }
    bool AddAppointment(const Appointment* value) {
        if (!CanBook(value, 0)) {
            return false;
        }
        return appointments.InsertAppointment(value);
    }
    bool UpdateAppointment(int id, const Appointment* value) {
        if (value == nullptr) {
            return false;
        }
        if (value->getId() != id) {
            return false;
        }
        if (!CanBook(value, id)) {
            return false;
        }
        return appointments.ModifyElement(id, value);
    }
    bool DeleteAppointment(int id) {
        return appointments.DeleteElement(id);
    }
    bool DeleteNurse(int id) {
        return nurses.DeleteElement(id);
    }
    bool DeletePatient(int id) {
        if (HasAppointment(id, true)) {
            return false;
        }
        return patients.DeleteElement(id);
    }
    bool DeleteDoctor(int id) {
        if (HasAppointment(id, false)) {
            return false;
        }
        return doctors.DeleteElement(id);
    }
    bool DeleteRoom(int id) {
        if (RoomOccupancy(id) > 0) {
            return false;
        }
        return rooms.DeleteElement(id);
    }
    int PatientRoom(int id) {
        int p = patients.FindElement(id);
        if (p == -1) {
            return -1;
        }
        return patients.GetAt(p)->getRoomId();
    }

    int Count(int type) const {
        if (type == 1) {
            return patients.GetElementNb();
        }
        if (type == 2) {
            return doctors.GetElementNb();
        }
        if (type == 3) {
            return nurses.GetElementNb();
        }
        if (type == 4) {
            return rooms.GetElementNb();
        }
        if (type == 5) {
            return appointments.GetElementNb();
        }
        return -1;
    }
    const Patient* FindPatient(int id) const {
        return patients.GetAt(patients.FindElement(id));
    }
    const Doctor* FindDoctor(int id) const {
        return doctors.GetAt(doctors.FindElement(id));
    }
    const Nurse* FindNurse(int id) const {
        return nurses.GetAt(nurses.FindElement(id));
    }
    const Room* FindRoom(int id) const {
        return rooms.GetAt(rooms.FindElement(id));
    }
    const Appointment* FindAppointment(int id) const {
        return appointments.GetAt(appointments.FindElement(id));
    }
    bool Search(int type, int id) const {
        bool found = false;
        if (type == 1) {
            found = patients.PrintElement(id);
        } else if (type == 2) {
            found = doctors.PrintElement(id);
        } else if (type == 3) {
            found = nurses.PrintElement(id);
        } else if (type == 4) {
            found = rooms.PrintElement(id);
        } else if (type == 5) {
            found = appointments.PrintElement(id);
        }
        if (found && type == 1) {
            cout << "Room: " << FindPatient(id)->getRoomId() << "\n";
        }
        if (found && type == 4) {
            cout << "Available places: " << FindRoom(id)->getCapacity() - RoomOccupancy(id) << "\n";
        }
        return found;
    }
    void DisplayCategory(int type) const {
        if (type == 1) {
            patients.PrintArray();
        } else if (type == 2) {
            doctors.PrintArray();
        } else if (type == 3) {
            nurses.PrintArray();
        } else if (type == 4) {
            rooms.PrintArray();
        } else if (type == 5) {
            appointments.PrintArray();
        } else {
            cout << "Invalid record type.\n";
        }
    }

    bool DisplaySchedule(int id, bool forDoctor) const {
        if ((forDoctor && !FindDoctor(id)) || (!forDoctor && !FindPatient(id))) {
            return false;
        }
        int indexes[100];
        int count = 0;
        for (int i = 0; i < appointments.GetElementNb(); i++) {
            const Appointment* a = appointments.GetAt(i);
            if ((forDoctor && a->getDoctorId() == id) || (!forDoctor && a->getPatientId() == id)) {
                indexes[count++] = i;
            }
        }

        for (int i = 1; i < count; i++) {
            int key = indexes[i];
            int j = i - 1;
            while (j >= 0) {
                const Appointment* left = appointments.GetAt(indexes[j]);
                const Appointment* right = appointments.GetAt(key);
                int order = strcmp(left->getDate(), right->getDate());
                if (order < 0 ||
                    (order == 0 && left->getStartMinutes() <= right->getStartMinutes())) {
                    break;
                }
                indexes[j + 1] = indexes[j];
                j--;
            }
            indexes[j + 1] = key;
        }
        if (count == 0) {
            cout << "No appointments.\n";
        }
        for (int i = 0; i < count; i++) {
            appointments.PrintElement(appointments.GetAt(indexes[i])->getId());
        }
        return true;
    }
    void DisplayAvailableRooms() const {
        bool found = false;
        for (int i = 0; i < rooms.GetElementNb(); i++) {
            const Room* room = rooms.GetAt(i);
            int available = room->getCapacity() - RoomOccupancy(room->getId());
            if (available > 0) {
                rooms.PrintElement(room->getId());
                cout << "Available places: " << available << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No available rooms.\n";
        }
    }
    void DisplaySummary() const {
        cout << "Patients: " << Count(1) << "/100\nDoctors: " << Count(2)
             << "/100\nNurses: " << Count(3) << "/100\nRooms: " << Count(4)
             << "/100\nAppointments: " << Count(5) << "/100\n";
        int capacity = 0, occupied = 0;
        for (int i = 0; i < rooms.GetElementNb(); i++) {
            capacity += rooms.GetAt(i)->getCapacity();
            occupied += RoomOccupancy(rooms.GetAt(i)->getId());
        }
        cout << "Occupied places: " << occupied << "\nAvailable places: " << capacity - occupied
             << "\n";
    }
    void Display() {
        cout << "\nPATIENTS\n";
        patients.PrintArray();
        for (int i = 0; i < patients.GetElementNb(); i++) {
            cout << "Patient " << patients.GetAt(i)->getId()
                 << " room: " << patients.GetAt(i)->getRoomId() << "\n";
        }
        cout << "\nDOCTORS\n";
        doctors.PrintArray();
        cout << "\nNURSES\n";
        nurses.PrintArray();
        cout << "\nROOMS\n";
        rooms.PrintArray();
        for (int i = 0; i < rooms.GetElementNb(); i++) {
            cout << "Room " << rooms.GetAt(i)->getId()
                 << " occupied: " << RoomOccupancy(rooms.GetAt(i)->getId()) << "\n";
        }
        cout << "\nAPPOINTMENTS\n";
        appointments.PrintArray();
    }
};
