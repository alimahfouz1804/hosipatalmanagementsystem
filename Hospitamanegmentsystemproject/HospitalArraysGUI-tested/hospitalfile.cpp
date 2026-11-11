#include "hospital.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {
bool ReadInt(std::istream& input, int& value) {
    std::string token;
    if (!(input >> token)) {
        return false;
    }
    std::stringstream number(token);
    number.imbue(std::locale::classic());
    if (!(number >> value)) {
        return false;
    }
    char extra;
    if (number >> extra) {
        return false;
    }
    return true;
}

bool ReadText(std::istream& input, std::string& value, size_t capacity) {
    input >> std::ws;
    if (input.peek() != '"' || !(input >> std::quoted(value))) {
        return false;
    }
    if (value.size() >= capacity || value.find('\0') != std::string::npos) {
        return false;
    }
    int next = input.peek();
    if (next == EOF) {
        return true;
    }
    return std::isspace((unsigned char)next) != 0;
}

bool ReadSection(std::istream& input, const char* expected, int& count) {
    std::string section;
    if (!(input >> section)) {
        return false;
    }
    if (section != expected) {
        return false;
    }
    if (!ReadInt(input, count)) {
        return false;
    }
    if (count < 0 || count > 100) {
        return false;
    }
    return true;
}

bool Fail(std::string& error, const char* message) {
    error = message;
    return false;
}
}

bool Hospital::SaveToFile(const std::filesystem::path& filename, std::string& error) const {
    error.clear();
    if (filename.empty()) {
        return Fail(error, "The data filename is empty.");
    }
    std::error_code status;
    bool exists = std::filesystem::exists(filename, status);
    if (status) {
        return Fail(error, "Cannot check the data path.");
    }
    if (exists && !std::filesystem::is_regular_file(filename, status)) {
        return Fail(error, "The data path must be a regular file.");
    }
    if (status) {
        return Fail(error, "Cannot check the data path.");
    }

    std::filesystem::path temporary = filename;
    temporary += ".tmp";
    int number = 1;
    while (std::filesystem::exists(temporary, status)) {
        temporary = filename;
        temporary += ".tmp." + std::to_string(number);
        number++;
        if (number > 1000) {
            return Fail(error, "Too many temporary save files.");
        }
    }
    if (status) {
        return Fail(error, "Cannot create a temporary save file.");
    }
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        return Fail(error, "Cannot create the save file. Check its folder and permissions.");
    }
    output.imbue(std::locale::classic());

    output << "HOSPITAL_OOP_ARRAYS 1\n";
    output << "ROOMS " << rooms.GetElementNb() << '\n';
    for (int i = 0; i < rooms.GetElementNb(); i++) {
        const Room* r = rooms.GetAt(i);
        output << r->getId() << ' ' << std::quoted(r->getType()) << ' ' << r->getCapacity() << '\n';
    }
    output << "DOCTORS " << doctors.GetElementNb() << '\n';
    for (int i = 0; i < doctors.GetElementNb(); i++) {
        const Doctor* d = doctors.GetAt(i);
        output << d->getId() << ' ' << std::quoted(d->getFname()) << ' '
               << std::quoted(d->getLname()) << ' ' << std::quoted(d->getSpecialty()) << '\n';
    }
    output << "NURSES " << nurses.GetElementNb() << '\n';
    for (int i = 0; i < nurses.GetElementNb(); i++) {
        const Nurse* n = nurses.GetAt(i);
        output << n->getId() << ' ' << std::quoted(n->getFname()) << ' '
               << std::quoted(n->getLname()) << ' ' << std::quoted(n->getDepartment()) << '\n';
    }
    output << "PATIENTS " << patients.GetElementNb() << '\n';
    for (int i = 0; i < patients.GetElementNb(); i++) {
        const Patient* p = patients.GetAt(i);
        output << p->getId() << ' ' << std::quoted(p->getFname()) << ' '
               << std::quoted(p->getLname()) << ' ' << p->getAge() << ' '
               << std::quoted(p->getDiagnosis()) << ' ' << p->getRoomId() << '\n';
    }
    output << "APPOINTMENTS " << appointments.GetElementNb() << '\n';
    for (int i = 0; i < appointments.GetElementNb(); i++) {
        const Appointment* a = appointments.GetAt(i);
        output << a->getId() << ' ' << a->getPatientId() << ' ' << a->getDoctorId() << ' '
               << std::quoted(a->getDate()) << ' ' << std::quoted(a->getTime()) << ' '
               << a->getDurationMinutes() << '\n';
    }
    output << "END\n";
    output.close();
    if (!output) {
        std::filesystem::remove(temporary, status);
        return Fail(error, "Writing the data failed. The previous save is unchanged.");
    }

#ifdef _WIN32
    bool replaced = MoveFileExW(temporary.c_str(), filename.c_str(),
                                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::filesystem::rename(temporary, filename, status);
    bool replaced = !status;
#endif
    if (!replaced) {
        std::filesystem::remove(temporary, status);
        return Fail(error, "Cannot replace the data file. The previous save is unchanged.");
    }
    return true;
}

bool Hospital::LoadFromFile(const std::filesystem::path& filename, std::string& error) {
    error.clear();
    std::error_code status;
    std::uintmax_t size = std::filesystem::file_size(filename, status);
    if (status) {
        return Fail(error, "Cannot read the data file. Check its path and permissions.");
    }

    if (size > 1024 * 1024) {
        return Fail(error, "The data file is too large.");
    }
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        return Fail(error, "Cannot open the data file.");
    }
    input.imbue(std::locale::classic());
    std::string marker;
    int version, count;
    if (!(input >> marker) || marker != "HOSPITAL_OOP_ARRAYS" || !ReadInt(input, version) ||
        version != 1) {
        return Fail(error, "Invalid data file header or unsupported version.");
    }

    Hospital loaded;
    if (!ReadSection(input, "ROOMS", count)) {
        return Fail(error, "Invalid room section.");
    }
    for (int i = 0; i < count; i++) {
        int id, capacity;
        std::string type;
        if (!ReadInt(input, id) || !ReadText(input, type, 80) || !ReadInt(input, capacity)) {
            return Fail(error, "Invalid room record.");
        }
        Room room(id, type.c_str(), capacity);
        if (!loaded.AddRoom(&room)) {
            return Fail(error, "Invalid or duplicate room.");
        }
    }
    if (!ReadSection(input, "DOCTORS", count)) {
        return Fail(error, "Invalid doctor section.");
    }
    for (int i = 0; i < count; i++) {
        int id;
        std::string first, last, specialty;
        if (!ReadInt(input, id) || !ReadText(input, first, 80) || !ReadText(input, last, 80) ||
            !ReadText(input, specialty, 80)) {
            return Fail(error, "Invalid doctor record.");
        }
        Doctor doctor(id, first.c_str(), last.c_str(), specialty.c_str());
        if (!loaded.AddDoctor(&doctor)) {
            return Fail(error, "Invalid or duplicate doctor.");
        }
    }
    if (!ReadSection(input, "NURSES", count)) {
        return Fail(error, "Invalid nurse section.");
    }
    for (int i = 0; i < count; i++) {
        int id;
        std::string first, last, department;
        if (!ReadInt(input, id) || !ReadText(input, first, 80) || !ReadText(input, last, 80) ||
            !ReadText(input, department, 80)) {
            return Fail(error, "Invalid nurse record.");
        }
        Nurse nurse(id, first.c_str(), last.c_str(), department.c_str());
        if (!loaded.AddNurse(&nurse)) {
            return Fail(error, "Invalid or duplicate nurse.");
        }
    }
    if (!ReadSection(input, "PATIENTS", count)) {
        return Fail(error, "Invalid patient section.");
    }
    for (int i = 0; i < count; i++) {
        int id, age, roomId;
        std::string first, last, diagnosis;
        if (!ReadInt(input, id) || !ReadText(input, first, 80) || !ReadText(input, last, 80) ||
            !ReadInt(input, age) || !ReadText(input, diagnosis, 120) || !ReadInt(input, roomId)) {
            return Fail(error, "Invalid patient record.");
        }
        Patient patient(id, first.c_str(), last.c_str(), age, diagnosis.c_str());
        if (!loaded.AddPatient(&patient) || roomId < 0 ||
            (roomId != 0 && !loaded.AssignRoom(id, roomId))) {
            return Fail(error,
                        "Invalid/duplicate patient, missing room or room capacity exceeded.");
        }
    }
    if (!ReadSection(input, "APPOINTMENTS", count)) {
        return Fail(error, "Invalid appointment section.");
    }
    for (int i = 0; i < count; i++) {
        int id, patientId, doctorId, duration;
        std::string date, time;
        if (!ReadInt(input, id) || !ReadInt(input, patientId) || !ReadInt(input, doctorId) ||
            !ReadText(input, date, 11) || !ReadText(input, time, 6) || !ReadInt(input, duration)) {
            return Fail(error, "Invalid appointment record.");
        }
        Appointment appointment(id, patientId, doctorId, date.c_str(), time.c_str(), duration);
        if (!loaded.AddAppointment(&appointment)) {
            return Fail(
                error,
                "Invalid/duplicate appointment, missing patient/doctor or schedule conflict.");
        }
    }
    if (!(input >> marker) || marker != "END") {
        return Fail(error, "The data file is incomplete.");
    }
    input >> std::ws;
    if (!input.eof() || input.bad()) {
        return Fail(error, "Unexpected content after the saved records.");
    }

    *this = loaded;
    return true;
}
