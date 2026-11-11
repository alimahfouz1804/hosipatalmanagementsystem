#pragma once
#include "appointment.h"
class AppointmentArray {
private:
    static const int MAX_APPOINTMENTS = 100;
    Appointment a[MAX_APPOINTMENTS];
    int nbele;

public:
    const Appointment* GetAt(int index) const {
        if (index < 0 || index >= nbele) {
            return nullptr;
        }
        return &a[index];
    }
    AppointmentArray() {
        Initialize();
    }
    void CopyAppointment(Appointment* destination, const Appointment* source) {
        *destination = *source;
    }
    void Initialize() {
        nbele = 0;
    }
    bool IsEmpty() const {
        return nbele == 0;
    }
    bool IsFull() const {
        return nbele == MAX_APPOINTMENTS;
    }
    int GetElementNb() const {
        return nbele;
    }
    int FindElement(int id) const {
        for (int i = 0; i < nbele; i++) {
            if (a[i].getId() == id) {
                return i;
            }
        }
        return -1;
    }
    bool IsInArray(int id) const {
        return FindElement(id) != -1;
    }
    bool InsertAppointment(const Appointment* value) {
        if (!value || !value->IsValid()) {
            return false;
        }
        if (IsFull() || value->getId() <= 0 || IsInArray(value->getId())) {
            return false;
        }
        Appointment copy;
        CopyAppointment(&copy, value);
        int pos;
        for (pos = 0; pos < nbele; pos++) {
            if (a[pos].getId() > copy.getId()) {
                break;
            }
        }
        for (int i = nbele; i > pos; i--) {
            CopyAppointment(&a[i], &a[i - 1]);
        }
        CopyAppointment(&a[pos], &copy);
        nbele++;
        return true;
    }
    bool DeleteAtPosition(int pos) {
        if (pos < 0 || pos >= nbele) {
            return false;
        }
        for (int i = pos; i < nbele - 1; i++) {
            CopyAppointment(&a[i], &a[i + 1]);
        }
        nbele--;
        return true;
    }
    bool DeleteElement(int id) {
        return DeleteAtPosition(FindElement(id));
    }
    bool ModifyElement(int oldId, const Appointment* value) {
        if (!value || !value->IsValid()) {
            return false;
        }
        int pos = FindElement(oldId);
        if (pos == -1 || value->getId() <= 0) {
            return false;
        }
        if (oldId != value->getId() && IsInArray(value->getId())) {
            return false;
        }
        Appointment copy;
        CopyAppointment(&copy, value);
        DeleteAtPosition(pos);
        return InsertAppointment(&copy);
    }
    void DeleteAllElement() {
        nbele = 0;
    }
    bool PrintElement(int id) const {
        int i = FindElement(id);
        if (i == -1) {
            return false;
        }
        cout << "id: " << a[i].getId() << " / " << "patientId: " << a[i].getPatientId() << " / "
             << "doctorId: " << a[i].getDoctorId() << " / " << "date: " << a[i].getDate() << " / "
             << "time: " << a[i].getTime() << " / duration: " << a[i].getDurationMinutes()
             << " minutes\n";
        return true;
    }
    void PrintArray() const {
        if (IsEmpty()) {
            cout << "The array is empty\n";
        }
        for (int i = 0; i < nbele; i++) {
            cout << "id: " << a[i].getId() << " / " << "patientId: " << a[i].getPatientId() << " / "
                 << "doctorId: " << a[i].getDoctorId() << " / " << "date: " << a[i].getDate()
                 << " / " << "time: " << a[i].getTime()
                 << " / duration: " << a[i].getDurationMinutes() << " minutes\n";
        }
    }
};
