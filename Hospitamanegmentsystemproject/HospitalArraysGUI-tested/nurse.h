#pragma once
#include "validation.h"
#include <iostream>
#include <cstring>
using namespace std;
class Nurse {
private:
    bool inputValid = true;
    int id{};
    char fname[80]{};
    char lname[80]{};
    char department[80]{};

public:
    bool IsValid() const {
        return inputValid && id > 0 && HasText(fname) && HasText(lname) && HasText(department);
    }
    Nurse() = default;
    Nurse(int id, const char* fname, const char* lname, const char* department) {
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
        if (!department || strlen(department) >= sizeof(this->department)) {
            inputValid = false;
        } else {
            strcpy_s(this->department, sizeof(this->department), department);
        }
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
    const char* getDepartment() const {
        return department;
    }
};
