#include "hospital.h"
#include <sstream>
#include <string>

int ReadNumber(const char* prompt) {
    string line;
    int value;
    char extra;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            throw "Input closed";
        }
        stringstream input(line);
        if ((input >> value) && !(input >> extra)) {
            return value;
        }
        cout << "Please enter a whole number.\n";
    }
}
void ReadText(const char* prompt, char* text, int size) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            throw "Input closed";
        }
        if (line.length() < (unsigned int)size) {
            strcpy_s(text, size, line.c_str());
            return;
        }
        cout << "Text too long. Maximum " << size - 1 << " characters.\n";
    }
}
void ShowResult(bool success) {
    if (success) {
        cout << "Operation completed.\n";
    } else {
        cout << "Operation refused: check the ID, data, capacity or linked records.\n";
    }
}

int main() {
    Hospital hospital;
    try {
        int choice;
        do {
            cout << "\n===== HOSPITAL MENU =====\n"
                 << "1. Add patient       2. Update patient      3. Delete patient\n"
                 << "4. Add doctor        5. Update doctor       6. Delete doctor\n"
                 << "7. Add nurse         8. Update nurse        9. Delete nurse\n"
                 << "10. Add room         11. Update room        12. Delete room\n"
                 << "13. Add appointment  14. Update appointment 15. Delete appointment\n"
                 << "16. Assign/transfer room   17. Release room\n"
                 << "18. Display all records\n"
                 << "19. Search by ID      20. Display one category\n"
                 << "21. Doctor schedule   22. Patient appointments\n"
                 << "23. Available rooms   24. Summary\n"
                 << "0. Exit\n";
            choice = ReadNumber("Choice: ");

            if (choice == 1 || choice == 2) {
                int id = ReadNumber("Patient ID: ");
                char first[80], last[80], diagnosis[120];
                ReadText("First name: ", first, 80);
                ReadText("Last name: ", last, 80);
                int age = ReadNumber("Age (0-120): ");
                ReadText("Diagnosis: ", diagnosis, 120);
                Patient patient(id, first, last, age, diagnosis);
                if (choice == 1) {
                    ShowResult(hospital.AddPatient(&patient));
                } else {
                    ShowResult(hospital.UpdatePatient(id, &patient));
                }
            } else if (choice == 4 || choice == 5) {
                int id = ReadNumber("Doctor ID: ");
                char first[80], last[80], specialty[80];
                ReadText("First name: ", first, 80);
                ReadText("Last name: ", last, 80);
                ReadText("Specialty: ", specialty, 80);
                Doctor doctor(id, first, last, specialty);
                if (choice == 4) {
                    ShowResult(hospital.AddDoctor(&doctor));
                } else {
                    ShowResult(hospital.UpdateDoctor(id, &doctor));
                }
            } else if (choice == 7 || choice == 8) {
                int id = ReadNumber("Nurse ID: ");
                char first[80], last[80], department[80];
                ReadText("First name: ", first, 80);
                ReadText("Last name: ", last, 80);
                ReadText("Department: ", department, 80);
                Nurse nurse(id, first, last, department);
                if (choice == 7) {
                    ShowResult(hospital.AddNurse(&nurse));
                } else {
                    ShowResult(hospital.UpdateNurse(id, &nurse));
                }
            } else if (choice == 10 || choice == 11) {
                int id = ReadNumber("Room ID: ");
                char type[80];
                ReadText("Room type: ", type, 80);
                int capacity = ReadNumber("Capacity (1-100): ");
                Room room(id, type, capacity);
                if (choice == 10) {
                    ShowResult(hospital.AddRoom(&room));
                } else {
                    ShowResult(hospital.UpdateRoom(id, &room));
                }
            } else if (choice == 13 || choice == 14) {
                int id = ReadNumber("Appointment ID: ");
                int patientId = ReadNumber("Patient ID: ");
                int doctorId = ReadNumber("Doctor ID: ");
                char date[11], time[6];
                ReadText("Date (YYYY-MM-DD): ", date, 11);
                ReadText("Time (HH:MM): ", time, 6);
                int duration = ReadNumber("Duration in minutes: ");
                Appointment appointment(id, patientId, doctorId, date, time, duration);
                bool success;
                if (choice == 13) {
                    success = hospital.AddAppointment(&appointment);
                } else {
                    success = hospital.UpdateAppointment(id, &appointment);
                }
                if (!success) {
                    cout << "Check the date/time, duration, patient/doctor IDs and overlapping "
                            "appointments.\n";
                }
                ShowResult(success);
            } else if (choice == 3) {
                ShowResult(hospital.DeletePatient(ReadNumber("Patient ID: ")));
            } else if (choice == 6) {
                ShowResult(hospital.DeleteDoctor(ReadNumber("Doctor ID: ")));
            } else if (choice == 9) {
                ShowResult(hospital.DeleteNurse(ReadNumber("Nurse ID: ")));
            } else if (choice == 12) {
                ShowResult(hospital.DeleteRoom(ReadNumber("Room ID: ")));
            } else if (choice == 15) {
                ShowResult(hospital.DeleteAppointment(ReadNumber("Appointment ID: ")));
            } else if (choice == 16) {
                int patientId = ReadNumber("Patient ID: ");
                int roomId = ReadNumber("Room ID: ");
                ShowResult(hospital.AssignRoom(patientId, roomId));
            } else if (choice == 17) {
                ShowResult(hospital.ReleaseRoom(ReadNumber("Patient ID: ")));
            } else if (choice == 18) {
                hospital.Display();
            } else if (choice == 19 || choice == 20) {
                int type =
                    ReadNumber("Type (1 patient, 2 doctor, 3 nurse, 4 room, 5 appointment): ");
                if (type < 1 || type > 5) {
                    cout << "Invalid record type.\n";
                } else if (choice == 20) {
                    hospital.DisplayCategory(type);
                } else {
                    int id = ReadNumber("Record ID: ");
                    if (!hospital.Search(type, id)) {
                        cout << "Record not found.\n";
                    }
                }
            } else if (choice == 21 || choice == 22) {
                int id = ReadNumber("Doctor/patient ID: ");
                if (!hospital.DisplaySchedule(id, choice == 21)) {
                    cout << "Doctor/patient not found.\n";
                }
            } else if (choice == 23) {
                hospital.DisplayAvailableRooms();
            } else if (choice == 24) {
                hospital.DisplaySummary();
            } else if (choice != 0) {
                cout << "Choose a number from 0 to 24.\n";
            }
        } while (choice != 0);
    } catch (const char*) {
        cout << "\nInput closed.\n";
    }
    cout << "Goodbye.\n";
    return 0;
}
