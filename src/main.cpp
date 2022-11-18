#include "MidiFile.h"
#include "Options.h"
#include "Midito3Oct.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <math.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace smf;

int main(int argc, char** argv) {
    Options options;
    options.process(argc, argv);
    MidiFile midifile;
    string filename, output_name, adj_opt;
    size_t p;
    int adj_oct_val = 0,
        avg_oct_of_song = 4,
        valid_note_num = 0;
    bool bool_go_next,
        bool_adj_octave = false,
        bool_adj_by_avg = false,
        bool_adj_outsiders = false;

    while (1) {

        bool_go_next = false;

        cout << "Input the file name to convert: ";
        getline(cin, filename);

        midifile.read(filename);
        midifile.doTimeAnalysis();
        midifile.linkNotePairs();

        int tracks = midifile.getTrackCount();

        if (tracks < 1) {
            cout << "The MIDI file, " << filename << " seems to have no tracks or invalid..." << endl << endl;
            continue;
        }

        for (int track = 1; track <= tracks; ++track) {
            string str;

            int note;
            long long sum = 0;
            valid_note_num = 0;

            cout << "========== Track " << track << " ==========" << endl;

            // Calculate the average octave of a track
            for (int event = 0; event < midifile[track].size(); ++event) {
                if (midifile[track][event].isNoteOn()) {
                    note = (int)midifile[track][event][1];

                    // + octave of a note
                    // Example: C3 => ((48 / 12) - 1) = 3
                    sum += ((note / ONE_OCTAVE_DIFF) - 1);
                    ++valid_note_num;
                }
            }
            if (valid_note_num <= 0) {
                cout << "The track is empty..." << endl;
                continue;
            }

            else
                avg_oct_of_song = static_cast<int>(sum / valid_note_num);

            tellOctaveAverage(tracks > 1, track, avg_oct_of_song);

            for (int event = 0; event < midifile[track].size(); ++event) {
                if (midifile[track][event].isNoteOn()) {
                    write3Oct(str, (int)midifile[track][event][1], adj_oct_val, adj_outsiders);
                    str += std::format("{:.1f}", round_double(midifile[track][event].getDurationInSeconds() * 100)) + "] ";
                }
            }

            if (!str.empty()) {
                str.pop_back();
                output_name = filename + "_Track_" + to_string(track) + ".txt";
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
        break;
    }




        if (bool_adj_octave = isOctAdj())
            bool_adj_by_avg = isOctAdjByAvg();





        p = filename.find_last_of(".");
        filename = filename.substr(0, p);
        std::filesystem::create_directory("./output");

    }

    return 0;
}

/*
* For Debug:
* 
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
*/