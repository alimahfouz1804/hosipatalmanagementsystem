# HospitalArraysGUI

A C++17 hospital management project with a Windows GUI, classes, pointers,
fixed arrays and file saving.

The arrays use `a` and `nbele`, sequential searching, sorted insertion by shifting,
deletion by shifting left, and removal followed by insertion for updates. The
source uses ordinary loops and `if/else` blocks, with no code comments.

## Run

Double-click `HospitalArraysGUI.exe` or `RunHospitalGUI.bat`.

The GUI loads `hospital_data.txt` from the folder beside the executable. It starts
empty if the file does not exist. Every successful Add, Update, Delete,
Assign/transfer and Release saves the complete hospital automatically.

- **Save records** saves all current records immediately.
- **Reload records** asks before replacing the hospital with its saved data.
- Closing keeps saved records for the next launch. If changes could not be saved,
  closing retries saving and keeps the window open if the retry fails.

The header displays the data-file location. When running an executable built in
`out\build\x64-Debug` or another build folder, its data file lives beside that
executable. To choose a different dataset, run:

```bat
HospitalArraysGUI.exe "C:\path\to\hospital_data.txt"
```

The destination folder must exist. Back up the data file to keep a copy of your
records. No test patients are added to your real project.

## Saved information

The single readable, versioned text file contains all fields for patients,
doctors, nurses, rooms and appointments, including patient room assignments and
appointment duration. Each category stays sorted by its record ID. Spaces, quotes,
backslashes, empty diagnoses and UTF-8 names are preserved.

Saving writes and closes a temporary file before replacing the previous save.
Loading validates the complete snapshot and its relationships before replacing
the current hospital. Damaged or incomplete files, duplicate IDs, invalid data,
overfilled rooms and appointment conflicts are rejected. A bad file at startup
stops the program without overwriting the file; restore a valid backup or use a
different data-file path.

## Build and test

Open this folder as a CMake project in Visual Studio, or run these commands in a
terminal with CMake and a C++17 compiler:

```bat
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The Visual Studio build normally creates `build\Release\HospitalArraysGUI.exe`.
`hospitalfile.cpp` implements saving and loading and is linked by CMake. The
existing console target remains available; this update wires saving into the GUI.

Four C++ test suites cover validation, randomized array operations, file
persistence and GUI handlers. The GUI tests use real controls in a hidden test
window, with predictable answers to confirmation/error dialogs. If Python 3 is
available when CMake configures the project, a fifth suite exercises the console
menu. Python is optional for building and running the application and C++ suites.
See `TEST_RESULTS.txt` for verified results and the limits of the GUI checks.
