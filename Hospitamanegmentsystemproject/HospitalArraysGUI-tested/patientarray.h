#pragma once
#include "patient.h"
class PatientArray {
private:
    static const int MAX_PATIENTS = 100;
    Patient a[MAX_PATIENTS];
    int nbele;

public:
    const Patient* GetAt(int index) const {
        if (index < 0 || index >= nbele) {
            return nullptr;
        }
        return &a[index];
    }
    PatientArray() {
        Initialize();
    }
    void CopyPatient(Patient* destination, const Patient* source) {
        *destination = *source;
    }
    void Initialize() {
        nbele = 0;
    }
    bool IsEmpty() const {
        return nbele == 0;
    }
    bool IsFull() const {
        return nbele == MAX_PATIENTS;
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
    bool InsertPatient(const Patient* value) {
        if (!value || !value->IsValid()) {
            return false;
        }
        if (IsFull() || value->getId() <= 0 || IsInArray(value->getId())) {
            return false;
        }
        Patient copy;
        CopyPatient(&copy, value);
        int pos;
        for (pos = 0; pos < nbele; pos++) {
            if (a[pos].getId() > copy.getId()) {
                break;
            }
        }
        for (int i = nbele; i > pos; i--) {
            CopyPatient(&a[i], &a[i - 1]);
        }
        CopyPatient(&a[pos], &copy);
        nbele++;
        return true;
    }
    bool DeleteAtPosition(int pos) {
        if (pos < 0 || pos >= nbele) {
            return false;
        }
        for (int i = pos; i < nbele - 1; i++) {
            CopyPatient(&a[i], &a[i + 1]);
        }
        nbele--;
        return true;
    }
    bool DeleteElement(int id) {
        return DeleteAtPosition(FindElement(id));
    }
    bool ModifyElement(int oldId, const Patient* value) {
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
        Patient copy;
        CopyPatient(&copy, value);
        DeleteAtPosition(pos);
        return InsertPatient(&copy);
    }
    void DeleteAllElement() {
        nbele = 0;
    }
    bool PrintElement(int id) const {
        int i = FindElement(id);
        if (i == -1) {
            return false;
        }
        cout << "id: " << a[i].getId() << " / " << "fname: " << a[i].getFname() << " / "
             << "lname: " << a[i].getLname() << " / " << "age: " << a[i].getAge() << " / "
             << "diagnosis: " << a[i].getDiagnosis() << "\n";
        return true;
    }
    void PrintArray() const {
        if (IsEmpty()) {
            cout << "The array is empty\n";
        }
        for (int i = 0; i < nbele; i++) {
            cout << "id: " << a[i].getId() << " / " << "fname: " << a[i].getFname() << " / "
                 << "lname: " << a[i].getLname() << " / " << "age: " << a[i].getAge() << " / "
                 << "diagnosis: " << a[i].getDiagnosis() << "\n";
        }
    }
};
