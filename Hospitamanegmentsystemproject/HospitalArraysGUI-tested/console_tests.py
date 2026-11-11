from pathlib import Path
import subprocess
import sys

exe = Path(sys.argv[1]).resolve()
checks = 0


def check(ok, message):
    global checks
    checks += 1
    if not ok:
        raise AssertionError(message)


def run(values):
    result = subprocess.run([str(exe)], input="\n".join(map(str, values)) + "\n", text=True,
                            capture_output=True, timeout=20)
    check(result.returncode == 0, result.stderr)
    return result.stdout


values = [
    "abc", 999,
    1, 1, "Ali", "Haddad", 30, "Checkup",
    1, 2, "Maya", "Nasser", 25, "",
    4, 20, "Sara", "Khalil", "General",
    4, 21, "Lina", "Salem", "Surgery",
    7, 30, "Noor", "Ali", "Ward",
    10, 10, "Ward", 1,
    10, 11, "Shared ward", 2,
    13, 40, 1, 20, "2026-11-12", "10:00", 60,
    13, 41, 2, 20, "2026-11-12", "10:30", 30,
    16, 1, 10, 16, 2, 10, 16, 1, 11, 16, 2, 11, 17, 1,
    18,
    19, 1, 1, 19, 2, 20, 19, 3, 30, 19, 4, 11, 19, 5, 40, 19, 1, 999,
    20, 1, 20, 2, 20, 3, 20, 4, 20, 5, 21, 20, 22, 1, 23, 24,
    2, 1, "Updated", "Patient", 31, "Recovered",
    5, 20, "Updated", "Doctor", "Surgery",
    8, 30, "Updated", "Nurse", "Emergency",
    11, 11, "Private room", 2,
    14, 40, 1, 20, "2026-11-13", "11:00", 45,
    18,
    15, 40, 3, 1, 6, 20, 9, 30, 12, 11, 17, 2, 3, 2, 12, 11, 12, 10, 6, 21,
    24, 0,
]
output = run(values)
for text in ["Please enter a whole number.", "Choose a number from 0 to 24.",
             "Operation refused", "Record not found.", "Recovered", "Private room",
             "2026-11-13", "duration: 45 minutes", "Goodbye."]:
    check(text in output, "Missing console behavior: " + text)
last_summary = output[output.rfind("Patients:"):]
for label in ["Patients", "Doctors", "Nurses", "Rooms", "Appointments"]:
    check(label + ": 0/100" in last_summary, "Records were not deleted: " + label)

eof_output = run([1, 1, "Incomplete"])
check("Input closed." in eof_output and "Goodbye." in eof_output, "EOF recovery failed")
long_output = run([1, 1, "X" * 80, "Valid", "Patient", "bad", 30, "", 24, 0])
check("Text too long." in long_output, "Long name recovery failed")
check("Please enter a whole number." in long_output, "Invalid age recovery failed")
check("Patients: 1/100" in long_output, "Recovered input did not add the record")
print(f"Console menu tests passed: {checks} checks across 3 scenarios.")
