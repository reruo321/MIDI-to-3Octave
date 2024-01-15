#include "MidiFile.h"
#include "Options.h"
#include "Midito3Oct.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <math.h>
#include <chrono>
#include <thread>
#include <map>

#define DEFAULT_OCT_AVG 4
#define DECIMAL_POINTS 3

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
        bool_adj_outsiders = false,
        bool_not_high_note = false;
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

            bool_not_high_note = isNotHighestNote();

            for (int event = 0; event < midifile[track - 1].size(); ++event) {
                if (midifile[track - 1][event].isNoteOn()) {
                    note = (int)midifile[track - 1][event][1];
                    tick = midifile[track - 1][event].tick;

                    if (bool_not_high_note) {
                        // + octave of a note
                        // Example: C3 => ((48 / 12) - 1) = 3
                        sum += ((note / ONE_OCTAVE_DIFF) - 1);
                    }
                    else {
                        if (highestNoteAtTick.find(tick) == highestNoteAtTick.end())
                            highestNoteAtTick.insert(make_pair(tick, note));
                        else if (note > highestNoteAtTick.at(tick))
                            highestNoteAtTick.at(tick) = note;
                    }
                    ++valid_note_num;
                }
            }
            if (valid_note_num <= 0) {
                std::cout << "The track is empty..." << endl;
                continue;
            }

            if (!bool_not_high_note) {
                for (const auto& h : highestNoteAtTick) {
                    int note = h.second;
                    sum += ((note / ONE_OCTAVE_DIFF) - 1);
                }
            }

            avg_oct_of_song = static_cast<int>(sum / valid_note_num);
            tellOctaveAverage(tracks > 1, track, avg_oct_of_song);

            if ((avg_oct_of_song != DEFAULT_OCT_AVG) && (bool_adj_octave = isOctAdj()))
                adj_oct_val = avg_oct_of_song;

            bool_adj_outsiders = isNoteAdj();

            if (bool_not_high_note) {
                for (int event = 0; event < midifile[track - 1].size(); ++event) {
                    if (midifile[track - 1][event].isNoteOn()) {
                        int tick = midifile[track - 1][event].tick;
                        int note = midifile[track - 1][event][1];

                        double absoluteTime = midifile.getTimeInSeconds(tick);
                        std::stringstream ss;
                        ss << std::fixed << std::setprecision(DECIMAL_POINTS) << absoluteTime;
                        std::string timeStr = ss.str();

                        str += ("[" + timeStr + ",");
                        write3Oct(str, note, adj_oct_val, bool_adj_outsiders);
                    }
                }
            }
            else {
                for (const auto& h : highestNoteAtTick) {
                    int tick = h.first;
                    int note = h.second;

                    double absoluteTime = midifile.getTimeInSeconds(tick);
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(DECIMAL_POINTS) << absoluteTime;
                    std::string timeStr = ss.str();

                    str += ("[" + timeStr + ",");
                    write3Oct(str, note, adj_oct_val, bool_adj_outsiders);
                }
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

    std::cout << "The MIDI file is converted successfully!" << endl;
    std::cout << "Press enter to exit...";
    std::cin.get();
    exit(EXIT_SUCCESS);

    return 0;
}