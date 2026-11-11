#pragma once
#include "validation.h"
#include <iostream>
#include <cstring>
using namespace std;
class Doctor {
private:
    bool inputValid = true;
    int id{};
    char fname[80]{};
    char lname[80]{};
    char specialty[80]{};

public:
    bool IsValid() const {
        return inputValid && id > 0 && HasText(fname) && HasText(lname) && HasText(specialty);
    }
    Doctor() = default;
    Doctor(int id, const char* fname, const char* lname, const char* specialty) {
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
        if (!specialty || strlen(specialty) >= sizeof(this->specialty)) {
            inputValid = false;
        } else {
            strcpy_s(this->specialty, sizeof(this->specialty), specialty);
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
    const char* getSpecialty() const {
        return specialty;
    }
};
