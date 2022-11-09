#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <math.h>

using namespace std;
using namespace smf;

double round_double(double num) {
    double d = pow(10, 1);
    return round(num * d) / d;
}

void write3Oct(string& output, int note) {
    int quotient, dividend;
    double remained_duration = 0;

    while (note < 48)
        note += 12;
    while (note > 83)
        note -= 12;

    quotient = note / 12;
    dividend = note % 12;

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
    }

    output += ",";
}

int main(int argc, char** argv) {
    Options options;
    options.process(argc, argv);
    MidiFile midifile;
    string filename, output_name;
    size_t p;

    if (options.getArgCount() == 0) {
        cout << "Input the file name to convert: ";
        getline(cin, filename);
        midifile.read(filename);
    }
    else midifile.read(options.getArg(1));
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    int tracks = midifile.getTrackCount();
    cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
    if (tracks > 1) cout << "TRACKS: " << tracks << endl;
    for (int track = 0; track < tracks; track++) {
        if (tracks > 1) cout << "\nTrack " << track << endl;
        cout << "Tick\tSeconds\tDur\tMessage" << endl;
        for (int event = 0; event < midifile[track].size(); event++) {
            cout << dec << midifile[track][event].tick;
            cout << '\t' << dec << midifile[track][event].seconds;
            cout << '\t';
            if (midifile[track][event].isNoteOn())
                cout << midifile[track][event].getDurationInSeconds();
            cout << '\t';
            for (int i = 0; i < midifile[track][event].size(); i++)
                cout << (int)midifile[track][event][i] << ' ';
            cout << endl;
        }
    }

    p = filename.find_last_of(".");
    filename = filename.substr(0, p);
    std::filesystem::create_directory("./output");

    for (int track = 0; track < tracks; ++track) {
        string str;

        for (int event = 0; event < midifile[track].size(); ++event) {
            if (midifile[track][event].isNoteOn()) {
                write3Oct(str, (int)midifile[track][event][1]);
                str += std::format("{:.1f}", round_double(midifile[track][event].getDurationInSeconds() * 100)) + "] ";
            }
        }
        if (!str.empty()) {
            str.pop_back();
            output_name = filename + "_Track_" + to_string(track + 1) + ".txt";
            ofstream output("./output/" + output_name);
            if (output.is_open()) {
                output << str;
                output.close();
            }
            else {
                cout << "Error while writing the output..." << endl;
                return -1;
            }
        }
    }

    return 0;
}