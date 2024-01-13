#include "MidiFile.h"
#include "Options.h"
#include "Midito3Oct.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <math.h>
#include <chrono>
#include <thread>
#include <map>

#define DEFAULT_OCT_AVG 4

using namespace smf;

int main(int argc, char** argv) {
    Options options;
    options.process(argc, argv);
    MidiFile midifile;
    string filename, output_name, adj_opt;
    size_t p;
    int adj_oct_val = DEFAULT_OCT_AVG,
        avg_oct_of_song = 4,
        valid_note_num = 0;
    bool bool_go_next,
        bool_adj_octave = false,
        bool_adj_by_avg = false,
        bool_adj_outsiders = false;
    ofstream output;

    filesystem::path outputDirectory = filesystem::path("./output");

    if (!filesystem::exists(outputDirectory))
        std::filesystem::create_directory(outputDirectory);

    while (true) {

        bool_go_next = false;

        std::cout << "Input the file name to convert: ";
        getline(cin, filename);
        filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());

        filesystem::path absolutePath = filesystem::absolute(filename);
        filename = absolutePath.string();

        std::ifstream fileStream(filename, ios::binary | ios::in);

        if (!fileStream.is_open()) {
            cerr << "Error: Unable to open the file.\nCurrent working directory: " << filesystem::current_path().string() << endl;

            cerr << "Press enter to exit...";
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        if (!midifile.read(fileStream)) {
            cerr << "Error: Failed to read MIDI file." << endl;

            cerr << "Press enter to exit...";
            std::cin.get();
            exit(EXIT_FAILURE);
        }
        midifile.doTimeAnalysis();
        midifile.linkNotePairs();

        int tracks = midifile.getTrackCount();

        if (tracks < 1) {
            std::cout << "The MIDI file, " << filename << " seems to have no tracks or invalid..." << endl << endl;
            continue;
        }

        std::cout << "The MIDI file, " << filename << " has " << tracks << " tracks." << endl << endl;

        for (int track = 1; track <= tracks; ++track) {
            std::map<int, int> highestNoteAtTick;
            string str;
            int note;
            int tick;
            long long sum = 0;
            valid_note_num = 0;

            std::cout << "========== Track " << track << " ==========" << endl;

//            str += ("Tempo: " + to_string(midifi
            str += ("Tempo: " + to_string(midifile.getTicksPerQuarterNote()) + "\n");

            for (int event = 0; event < midifile[track - 1].size(); ++event) {
                if (midifile[track - 1][event].isNoteOn()) {
                    note = (int)midifile[track - 1][event][1];
                    tick = midifile[track - 1][event].tick;

                    if (highestNoteAtTick.find(tick) == highestNoteAtTick.end() || note > highestNoteAtTick.at(tick))
                        highestNoteAtTick.at(tick) = note;

                    ++valid_note_num;
                }
            }
            if (valid_note_num <= 0) {
                std::cout << "The track is empty..." << endl;
                continue;
            }

            for (const auto& h : highestNoteAtTick) {
                int note = h.second;

                // + octave of a note
                // Example: C3 => ((48 / 12) - 1) = 3
                sum += ((note / ONE_OCTAVE_DIFF) - 1);
            }

            avg_oct_of_song = static_cast<int>(sum / valid_note_num);
            tellOctaveAverage(tracks > 1, track, avg_oct_of_song);

            if ((avg_oct_of_song != DEFAULT_OCT_AVG) && (bool_adj_octave = isOctAdj()))
                adj_oct_val = avg_oct_of_song;

            bool_adj_outsiders = isNoteAdj();

            for (const auto& h : highestNoteAtTick) {
                int tick = h.first;
                int note = h.second;

                write3Oct(str, note, adj_oct_val, bool_adj_outsiders);
                str += (to_string(tick) + "] ");
            }

            if (!str.empty()) {
                str.pop_back();
                filesystem::path filenamePath(filename);
                output_name = filenamePath.stem().string() + "_Track_" + to_string(track) + ".txt";

                filesystem::path outputPath = outputDirectory / output_name;
                ofstream trackOutput(outputPath.string(), ios::app);

                if (trackOutput.is_open()) {
                    trackOutput << str;
                    trackOutput.close();
                }
                else {
                    cerr << "Error while opening file: " << filesystem::absolute("./output/" + output_name) << endl;
                    return -1;
                }
            }
        }
        fileStream.close();
        break;
    }
    output.close();

    std::cout << "Successfully Converted!" << endl;
    std::cout << "Press enter to finish the program...";
    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');

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