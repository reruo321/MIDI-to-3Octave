#pragma once
#include <iostream>

using namespace std;

#define B5 83
#define C3 48
#define DEFAULT_OCT_AVG 4
#define ONE_OCTAVE_DIFF 12

enum { OCT_ORIGINAL, OCT_ADJUST, OCT_AVG };

double round_double(double num) {
    double d = pow(10, 1);
    return round(num * d) / d;
}

void writeText(int quotient, int dividend, string& output) {
    output += "[" + to_string(dividend + 1);

    switch (quotient) {
    case 4:
        output += "L";
        break;
    case 5:
        output += "M";
        break;
    case 6:
        output += "H";
        break;
    default:
        output += "M";
    }

    output += ",";
}

void write3Oct(string& output, int note, int adj_oct_val, bool adj_outsiders) {
    int quotient, dividend;

    note += ((DEFAULT_OCT_AVG - adj_oct_val) * ONE_OCTAVE_DIFF);

    // adjust the octaves of outsider notes
    if (adj_outsiders) {
        while (note < C3)
            note += ONE_OCTAVE_DIFF;
        while (note > B5)
            note -= ONE_OCTAVE_DIFF;

        quotient = note / ONE_OCTAVE_DIFF;
        dividend = note % ONE_OCTAVE_DIFF;
    }
    // discard outsider notes
    else {
        if (note < C3 || note > B5) {
            dividend = -1;
            quotient = 5;
        }
        else {
            quotient = note / ONE_OCTAVE_DIFF;
            dividend = note % ONE_OCTAVE_DIFF;
        }
    }

    writeText(quotient, dividend, output);
}

void tellOctaveAverage(bool isMultipleTrack, int track, int avg) {
    if (isMultipleTrack)
        cout << "### The Average Octave of Track " << track << " is " << avg << " ###" << endl;
    else
        cout << "### The Average Octave of Your Song is " << avg << " ###" << endl;
}

bool isOctAdj() {
    string adj_opt;
    bool bool_adj_octave;

    cout << "б▐ Adjust the Range of Octaves to [3, 5]?" << endl
        << "1. Yes" << endl
        << "2. No" << endl
        << "[Please type 1 or 2]: ";

    while (1) {
        cin >> adj_opt;

        switch (adj_opt.at(0)) {
        case '1':
        case 'Y':
        case 'y':
            return true;
        case '2':
        case 'N':
        case 'n':
            return false;
        default:
            cout << "Try again. [Please type 1 or 2]: ";
            break;
        }
    }
}

bool isNoteAdj() {

    string adj_opt;
    bool bool_note_adj;

    cout << endl << "бс Process Notes Outside of Supporting Octaves: Adjust or Discard?" << endl
        << "1. Adjust" << endl
        << "2. Discard" << endl
        << "[Please type 1 or 2]: ";

    while (1) {
        cin >> adj_opt;
        bool esc = false;

        switch (adj_opt.at(0)) {
        case '1':
            esc = true;
            bool_note_adj = true;
            break;
        case '2':
            esc = true;
            bool_note_adj = false;
            break;
        default:
            cout << "Try again. [Please type 1 or 2]: ";
            break;
        }
        if (esc)
            break;
    }
    return bool_note_adj;
}