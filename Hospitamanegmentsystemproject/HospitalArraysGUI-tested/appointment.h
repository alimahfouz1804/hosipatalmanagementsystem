#pragma once
#include "validation.h"
#include <iostream>
#include <cstring>
using namespace std;
class Appointment {
private:
    bool inputValid = true;
    int id{};
    int patientId{};
    int doctorId{};
    char date[11]{};
    char time[6]{};
    int durationMinutes = 30;

public:
    bool IsValid() const {
        return inputValid && id > 0 && patientId > 0 && doctorId > 0 && ValidDate(date) &&
               ValidTime(time) && durationMinutes > 0 && durationMinutes <= 1440 &&
               getStartMinutes() + durationMinutes <= 1440;
    }
    Appointment() = default;
    Appointment(int id, int patientId, int doctorId, const char* date, const char* time,
                int durationMinutes = 30) {
        this->durationMinutes = durationMinutes;
        this->id = id;
        this->patientId = patientId;
        this->doctorId = doctorId;
        if (!date || strlen(date) >= sizeof(this->date)) {
            inputValid = false;
        } else {
            strcpy_s(this->date, sizeof(this->date), date);
        }
        if (!time || strlen(time) >= sizeof(this->time)) {
            inputValid = false;
        } else {
            strcpy_s(this->time, sizeof(this->time), time);
        }
    }
    int getId() const {
        return id;
    }
    int getDurationMinutes() const {
        return durationMinutes;
    }
    int getStartMinutes() const {
        return ((time[0] - '0') * 10 + time[1] - '0') * 60 + (time[3] - '0') * 10 + time[4] - '0';
    }
    int getPatientId() const {
        return patientId;
    }
    int getDoctorId() const {
        return doctorId;
    }
    const char* getDate() const {
        return date;
    }
    const char* getTime() const {
        return time;
    }
};
