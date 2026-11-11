#pragma once
#include "room.h"
class RoomArray {
private:
    static const int MAX_ROOMS = 100;
    Room a[MAX_ROOMS];
    int nbele;

public:
    const Room* GetAt(int index) const {
        if (index < 0 || index >= nbele) {
            return nullptr;
        }
        return &a[index];
    }
    RoomArray() {
        Initialize();
    }
    void CopyRoom(Room* destination, const Room* source) {
        *destination = *source;
    }
    void Initialize() {
        nbele = 0;
    }
    bool IsEmpty() const {
        return nbele == 0;
    }
    bool IsFull() const {
        return nbele == MAX_ROOMS;
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
    bool InsertRoom(const Room* value) {
        if (!value || !value->IsValid()) {
            return false;
        }
        if (IsFull() || value->getId() <= 0 || IsInArray(value->getId())) {
            return false;
        }
        Room copy;
        CopyRoom(&copy, value);
        int pos;
        for (pos = 0; pos < nbele; pos++) {
            if (a[pos].getId() > copy.getId()) {
                break;
            }
        }
        for (int i = nbele; i > pos; i--) {
            CopyRoom(&a[i], &a[i - 1]);
        }
        CopyRoom(&a[pos], &copy);
        nbele++;
        return true;
    }
    bool DeleteAtPosition(int pos) {
        if (pos < 0 || pos >= nbele) {
            return false;
        }
        for (int i = pos; i < nbele - 1; i++) {
            CopyRoom(&a[i], &a[i + 1]);
        }
        nbele--;
        return true;
    }
    bool DeleteElement(int id) {
        return DeleteAtPosition(FindElement(id));
    }
    bool ModifyElement(int oldId, const Room* value) {
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
        Room copy;
        CopyRoom(&copy, value);
        DeleteAtPosition(pos);
        return InsertRoom(&copy);
    }
    void DeleteAllElement() {
        nbele = 0;
    }
    bool PrintElement(int id) const {
        int i = FindElement(id);
        if (i == -1) {
            return false;
        }
        cout << "id: " << a[i].getId() << " / " << "type: " << a[i].getType() << " / "
             << "capacity: " << a[i].getCapacity() << "\n";
        return true;
    }
    void PrintArray() const {
        if (IsEmpty()) {
            cout << "The array is empty\n";
        }
        for (int i = 0; i < nbele; i++) {
            cout << "id: " << a[i].getId() << " / " << "type: " << a[i].getType() << " / "
                 << "capacity: " << a[i].getCapacity() << "\n";
        }
    }
};
