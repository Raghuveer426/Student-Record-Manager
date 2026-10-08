/*
 * Student Record Manager
 * A C++ Console Application for Managing Student Academic Records
 * Object-Oriented Programming through C++ (2501CS08) - Cornerstone Project
 * Modules: Registration, Display, Search, Update, Delete, Marks Analysis
 * Storage: "students.txt" via fstream. Format: rollNo|name|m1|m2|m3|m4|m5
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace std;

const int NUM_SUBJECTS = 5;
const string DATA_FILE = "students.txt";

/* ---------- small helper functions (no C++11 needed) ---------- */
string intToString(int n) {
    ostringstream oss;
    oss << n;
    return oss.str();
}

string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++)
        s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

/* ============================================================
 *  Class: Student
 * ============================================================ */
class Student {
private:
    int    rollNo;
    string name;
    float  marks[NUM_SUBJECTS];

public:
    Student() : rollNo(0), name("") {
        for (int i = 0; i < NUM_SUBJECTS; i++) marks[i] = 0;
    }

    Student(int roll, const string& n, const float m[]) : rollNo(roll), name(n) {
        for (int i = 0; i < NUM_SUBJECTS; i++) marks[i] = m[i];
    }

    int           getRollNo() const { return rollNo; }
    const string& getName()   const { return name; }

    void setName(const string& n) { name = n; }
    void setMarks(const float m[]) {
        for (int i = 0; i < NUM_SUBJECTS; i++) marks[i] = m[i];
    }

    float getTotal() const {
        float t = 0;
        for (int i = 0; i < NUM_SUBJECTS; i++) t += marks[i];
        return t;
    }

    float getAverage() const { return getTotal() / NUM_SUBJECTS; }

    string getGrade() const {
        float avg = getAverage();
        if (avg >= 90) return "A+";
        if (avg >= 80) return "A";
        if (avg >= 70) return "B";
        if (avg >= 60) return "C";
        if (avg >= 50) return "D";
        if (avg >= 40) return "E";
        return "F";
    }

    static void printHeader() {
        cout << left << setw(8) << "Roll" << setw(20) << "Name";
        for (int i = 1; i <= NUM_SUBJECTS; i++)
            cout << setw(7) << ("S" + intToString(i));
        cout << setw(9) << "Total" << setw(9) << "Average" << "Grade" << endl;
        cout << string(8 + 20 + 7 * NUM_SUBJECTS + 9 + 9 + 5, '-') << endl;
    }

    void printRow() const {
        cout << left << setw(8) << rollNo << setw(20) << name.substr(0, 19);
        cout << fixed << setprecision(1);
        for (int i = 0; i < NUM_SUBJECTS; i++) cout << setw(7) << marks[i];
        cout << setw(9) << getTotal();
        cout << setw(9) << setprecision(2) << getAverage();
        cout << getGrade() << endl;
    }

    string toFileLine() const {
        ostringstream oss;
        oss << rollNo << "|" << name;
        for (int i = 0; i < NUM_SUBJECTS; i++) oss << "|" << marks[i];
        return oss.str();
    }

    // Returns false if the line is not a valid record.
    static bool fromFileLine(const string& line, Student& s) {
        vector<string> parts;
        string token;
        istringstream ss(line);
        while (getline(ss, token, '|')) parts.push_back(token);
        if (parts.size() != (size_t)(2 + NUM_SUBJECTS)) return false;

        istringstream rollStream(parts[0]);
        int roll;
        if (!(rollStream >> roll)) return false;

        float m[NUM_SUBJECTS];
        for (int i = 0; i < NUM_SUBJECTS; i++) {
            istringstream ms(parts[2 + i]);
            if (!(ms >> m[i])) return false;
        }

        s.rollNo = roll;
        s.name = parts[1];
        for (int i = 0; i < NUM_SUBJECTS; i++) s.marks[i] = m[i];
        return true;
    }
};

/* Used for sorting records by roll number */
bool compareByRoll(const Student& a, const Student& b) {
    return a.getRollNo() < b.getRollNo();
}

/* ============================================================
 *  Input helpers (validation)
 * ============================================================ */
void clearInput() {
    if (cin.eof()) {   // input stream closed: avoid an endless loop
        cout << "\nInput ended. Exiting program.\n";
        exit(0);
    }
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            clearInput();
            return value;
        }
        cout << "  [Error] Please enter a whole number between "
             << minVal << " and " << maxVal << "." << endl;
        clearInput();
    }
}

float readMark(const string& prompt) {
    float value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0 && value <= 100) {
            clearInput();
            return value;
        }
        cout << "  [Error] Marks must be a number between 0 and 100." << endl;
        clearInput();
    }
}

string readName(const string& prompt) {
    string name;
    while (true) {
        cout << prompt;
        if (!getline(cin, name)) clearInput();

        size_t start = name.find_first_not_of(" \t");
        size_t end   = name.find_last_not_of(" \t");
        if (start == string::npos) name = "";
        else name = name.substr(start, end - start + 1);

        if (name.empty())
            cout << "  [Error] Name cannot be empty." << endl;
        else if (name.find('|') != string::npos)
            cout << "  [Error] Name cannot contain the '|' character." << endl;
        else
            return name;
    }
}

void waitForEnter() {
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

/* ============================================================
 *  Class: StudentManager
 * ============================================================ */
class StudentManager {
private:
    vector<Student> students;

    int findIndex(int roll) const {
        for (size_t i = 0; i < students.size(); i++)
            if (students[i].getRollNo() == roll) return (int)i;
        return -1;
    }

    void readAllMarks(float m[]) const {
        for (int i = 0; i < NUM_SUBJECTS; i++)
            m[i] = readMark("  Marks in Subject " + intToString(i + 1) + " (0-100): ");
    }

public:
    void loadFromFile() {
        ifstream fin(DATA_FILE.c_str());
        if (!fin) return;   // first run: no file yet

        students.clear();
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            Student s;
            if (Student::fromFileLine(line, s)) students.push_back(s);
        }
        fin.close();
        cout << "Loaded " << students.size() << " record(s) from "
             << DATA_FILE << "." << endl;
    }

    bool saveToFile() const {
        ofstream fout(DATA_FILE.c_str());
        if (!fout) {
            cout << "  [Error] Could not write to " << DATA_FILE << "." << endl;
            return false;
        }
        for (size_t i = 0; i < students.size(); i++)
            fout << students[i].toFileLine() << "\n";
        fout.close();
        return true;
    }

    // 1. Registration
    void addStudent() {
        cout << "\n=== Add New Student ===" << endl;
        int roll = readInt("Roll Number: ", 1, 999999999);

        if (findIndex(roll) != -1) {
            cout << "  [Error] Roll Number " << roll
                 << " already exists. Record not added." << endl;
            return;
        }

        string name = readName("Student Name: ");
        float m[NUM_SUBJECTS];
        readAllMarks(m);

        students.push_back(Student(roll, name, m));
        if (saveToFile()) cout << "\n  New record saved successfully." << endl;
    }

    // 2. Display
    void displayAll() const {
        cout << "\n=== All Student Records ===" << endl;
        if (students.empty()) {
            cout << "  No records found." << endl;
            return;
        }
        vector<Student> sorted = students;
        sort(sorted.begin(), sorted.end(), compareByRoll);

        Student::printHeader();
        for (size_t i = 0; i < sorted.size(); i++) sorted[i].printRow();
        cout << "\nTotal records: " << students.size() << endl;
    }

    // 3. Search
    void searchStudent() const {
        cout << "\n=== Search Student ===" << endl;
        cout << "  1. Search by Roll Number\n  2. Search by Name" << endl;
        int choice = readInt("Choice: ", 1, 2);

        if (choice == 1) {
            int roll = readInt("Enter Roll Number: ", 1, 999999999);
            int idx = findIndex(roll);
            if (idx == -1) {
                cout << "  Student not found." << endl;
            } else {
                Student::printHeader();
                students[idx].printRow();
            }
        } else {
            string key = toLower(readName("Enter Name (or part of it): "));
            bool found = false;
            for (size_t i = 0; i < students.size(); i++) {
                if (toLower(students[i].getName()).find(key) != string::npos) {
                    if (!found) Student::printHeader();
                    students[i].printRow();
                    found = true;
                }
            }
            if (!found) cout << "  Student not found." << endl;
        }
    }

    // 4. Update
    void updateStudent() {
        cout << "\n=== Update Student Record ===" << endl;
        int roll = readInt("Enter Roll Number to update: ", 1, 999999999);
        int idx = findIndex(roll);
        if (idx == -1) {
            cout << "  Student not found." << endl;
            return;
        }

        cout << "\nCurrent record:" << endl;
        Student::printHeader();
        students[idx].printRow();

        cout << "\nEnter new details (Roll Number cannot be changed):" << endl;
        string name = readName("New Name: ");
        float m[NUM_SUBJECTS];
        readAllMarks(m);

        students[idx].setName(name);
        students[idx].setMarks(m);
        if (saveToFile()) cout << "\n  Record updated successfully." << endl;
    }

    // 5. Delete
    void deleteStudent() {
        cout << "\n=== Delete Student Record ===" << endl;
        int roll = readInt("Enter Roll Number to delete: ", 1, 999999999);
        int idx = findIndex(roll);
        if (idx == -1) {
            cout << "  Student not found." << endl;
            return;
        }

        Student::printHeader();
        students[idx].printRow();

        char confirm = 'n';
        while (true) {
            cout << "\nAre you sure you want to delete this record? (y/n): ";
            if (cin >> confirm) {
                clearInput();
                confirm = (char)tolower((unsigned char)confirm);
                if (confirm == 'y' || confirm == 'n') break;
            } else {
                clearInput();
            }
            cout << "  [Error] Please enter 'y' or 'n'." << endl;
        }

        if (confirm == 'y') {
            students.erase(students.begin() + idx);
            if (saveToFile()) cout << "  Record deleted successfully." << endl;
        } else {
            cout << "  Deletion cancelled." << endl;
        }
    }

    // 6. Marks Analysis
    void analyzeMarks() const {
        cout << "\n=== Marks Analysis ===" << endl;
        if (students.empty()) {
            cout << "  No records available for analysis." << endl;
            return;
        }

        size_t hi = 0, lo = 0;
        float sumAvg = 0;
        for (size_t i = 0; i < students.size(); i++) {
            if (students[i].getTotal() > students[hi].getTotal()) hi = i;
            if (students[i].getTotal() < students[lo].getTotal()) lo = i;
            sumAvg += students[i].getAverage();
        }

        cout << "\nTotal, Average & Grade of every student:" << endl;
        Student::printHeader();
        for (size_t i = 0; i < students.size(); i++) students[i].printRow();

        cout << fixed << setprecision(2);
        cout << "\nHighest Scorer : " << students[hi].getName()
             << " (Roll " << students[hi].getRollNo() << ") - Total "
             << students[hi].getTotal() << ", Grade " << students[hi].getGrade() << endl;
        cout << "Lowest Scorer  : " << students[lo].getName()
             << " (Roll " << students[lo].getRollNo() << ") - Total "
             << students[lo].getTotal() << ", Grade " << students[lo].getGrade() << endl;
        cout << "Class Average  : " << sumAvg / students.size() << endl;
    }
};

/* ============================================================
 *  Main Menu
 * ============================================================ */
void showMenu() {
    cout << "\n=========================================" << endl;
    cout << "        STUDENT RECORD MANAGER" << endl;
    cout << "=========================================" << endl;
    cout << "  1. Add Student" << endl;
    cout << "  2. Display All Records" << endl;
    cout << "  3. Search Student" << endl;
    cout << "  4. Update Student" << endl;
    cout << "  5. Delete Student" << endl;
    cout << "  6. Marks Analysis" << endl;
    cout << "  7. Exit" << endl;
    cout << "-----------------------------------------" << endl;
}

int main() {
    StudentManager manager;
    manager.loadFromFile();

    int choice;
    do {
        showMenu();
        choice = readInt("Enter your choice (1-7): ", 1, 7);

        switch (choice) {
            case 1: manager.addStudent();    break;
            case 2: manager.displayAll();    break;
            case 3: manager.searchStudent(); break;
            case 4: manager.updateStudent(); break;
            case 5: manager.deleteStudent(); break;
            case 6: manager.analyzeMarks();  break;
            case 7:
                manager.saveToFile();
                cout << "\nRecords saved. Thank you for using Student Record Manager!" << endl;
                break;
        }
        if (choice != 7) waitForEnter();
    } while (choice != 7);

    return 0;
}
