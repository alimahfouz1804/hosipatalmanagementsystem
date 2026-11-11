#pragma once
#include <cstring>
#include <cctype>

inline bool HasText(const char* text) {
    if (!text) {
        return false;
    }
    for (int i = 0; text[i]; i++) {
        if (!isspace((unsigned char)text[i])) {
            return true;
        }
    }
    return false;
}
inline bool ValidDate(const char* date) {
    if (!date || strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) {
            return false;
        }
    }
    int year =
        (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
    int month = (date[5] - '0') * 10 + date[6] - '0';
    int day = (date[8] - '0') * 10 + date[9] - '0';
    if (year < 1 || month < 1 || month > 12) {
        return false;
    }
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) {
        days[1] = 29;
    }
    return day >= 1 && day <= days[month - 1];
}
inline bool ValidTime(const char* time) {
    if (!time || strlen(time) != 5 || time[2] != ':') {
        return false;
    }
    for (int i = 0; i < 5; i++) {
        if (i != 2 && (time[i] < '0' || time[i] > '9')) {
            return false;
        }
    }
    int hour = (time[0] - '0') * 10 + time[1] - '0';
    int minute = (time[3] - '0') * 10 + time[4] - '0';
    return hour < 24 && minute < 60;
}
