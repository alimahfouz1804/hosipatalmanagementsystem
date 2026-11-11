#pragma once
#include "validation.h"
#include <iostream>
#include <cstring>
using namespace std;
class Patient {
private:
    bool inputValid = true;
    int id{};
    char fname[80]{};
    char lname[80]{};
    int age{};
    int roomId = 0;
    char diagnosis[120]{};

public:
    bool IsValid() const {
        return inputValid && id > 0 && HasText(fname) && HasText(lname) && age >= 0 && age <= 120;
    }
    Patient() = default;
    Patient(int id, const char* fname, const char* lname, int age, const char* diagnosis) {
        this->id = id;
        if (!fname || strlen(fname) >= sizeof(this->fname)) {
            inputValid = false;
        } else {
            strcpy_s(this->fname, sizeof(this->fname), fname);
        }
        if (!lname || strlen(lname) >= sizeof(this->lname)) {
            inputValid = false;
        } else {
            strcpy_s(this->lname, sizeof(this->lname), lname);
        }
        this->age = age;
        if (!diagnosis || strlen(diagnosis) >= sizeof(this->diagnosis)) {
            inputValid = false;
        } else {
            strcpy_s(this->diagnosis, sizeof(this->diagnosis), diagnosis);
        }
    }
    int getRoomId() const {
        return roomId;
    }
    bool SetRoomId(int room) {
        if (room < 0) {
            return false;
        }
        roomId = room;
        return true;
    }
    int getId() const {
        return id;
    }
    const char* getFname() const {
        return fname;
    }
    const char* getLname() const {
        return lname;
    }
    int getAge() const {
        return age;
    }
    const char* getDiagnosis() const {
        return diagnosis;
    }
};
