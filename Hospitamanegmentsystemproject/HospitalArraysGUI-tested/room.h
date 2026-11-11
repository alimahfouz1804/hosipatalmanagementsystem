#pragma once
#include "validation.h"
#include <iostream>
#include <cstring>
using namespace std;
class Room {
private:
    bool inputValid = true;
    int id{};
    char type[80]{};
    int capacity{};

public:
    bool IsValid() const {
        return inputValid && id > 0 && HasText(type) && capacity > 0 && capacity <= 100;
    }
    Room() = default;
    Room(int id, const char* type, int capacity) {
        this->id = id;
        if (!type || strlen(type) >= sizeof(this->type)) {
            inputValid = false;
        } else {
            strcpy_s(this->type, sizeof(this->type), type);
        }
        this->capacity = capacity;
    }
    int getId() const {
        return id;
    }
    const char* getType() const {
        return type;
    }
    int getCapacity() const {
        return capacity;
    }
};
