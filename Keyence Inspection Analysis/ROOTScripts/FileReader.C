#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "TTree.h"
#include "TFile.h"

void FileReader(const char* boardName, int firstSerial, int lastSerial) {
    // Variables for the TTree
    std::string BoardType = boardName;
    int SerialNumber;
    int MeasurementId;
    std::string MeasurementName;
    double MeasurementValue;
    double NominalValue;
    double UpperTolerance;
    double LowerTolerance;
    int Batch;
    std::string dataname;

    // Map to track seen measurement names and assign unique IDs
    std::map<std::string, int> measurementNameToId;
    int currentId = 1;

    // Create output file with dynamic name
    TFile outFile(Form("new%sTreeFile.root", boardName), "RECREATE");
    TTree *tree = new TTree("tree", "tree");

    // Set up branches
    tree->Branch("BoardType", &BoardType);
    tree->Branch("SerialNumber", &SerialNumber);
    tree->Branch("MeasurementId", &MeasurementId);
    tree->Branch("MeasurementName", &MeasurementName);
    tree->Branch("MeasurementValue", &MeasurementValue);
    tree->Branch("NominalValue", &NominalValue);
    tree->Branch("UpperTolerance", &UpperTolerance);
    tree->Branch("LowerTolerance", &LowerTolerance);
    tree->Branch("Batch", &Batch);

    dataname = "";


    // Loop over serial numbers
    for (int i = firstSerial; i <= lastSerial; i++) {
        BoardType = boardName;

        // Batch determination logic (modify if needed for other boards)
        if (i <= 72) {
            Batch = 1;
        } else if (i > 72 && i < 1073) {
            Batch = 2;
        } else {
            Batch = 3;
        }

        if (std::string(boardName).compare("AdapterFront") == 0) {
            dataname = "Adapt";
        }

        else if (std::string(boardName).compare("AdapterBack") == 0) {
            dataname = "AdaptBK";
        }

        else if (std::string(boardName).compare("CRFront") == 0) {
            dataname = "CR";
        }
        else if (std::string(boardName).compare("CRBack") == 0) {
            dataname = "CRBK";
        }
        else if (std::string(boardName).compare("GBias") == 0) {
            dataname = "GBias";
        }
        else {
            std::cerr << "Invalid Board Name" << std::endl;
        }
        
        // Construct filename with zero-padding
        std::string filename = Form("/home/tkdkrishna/BoardData/%s/%sData%04d.txt", 
                                  boardName, dataname.c_str(), i);
        SerialNumber = i;

        // Open the data file
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cout << "Could not open " << filename << std::endl;
            continue;
        }

        // Process each line
        std::string line;
        while (getline(inputFile, line)) {
            std::stringstream inputString(line);
            std::string tempString;

            // Skip original MeasurementId
            getline(inputString, tempString, ',');

            // Extract and clean MeasurementName
            getline(inputString, MeasurementName, ',');
            MeasurementName.erase(0, MeasurementName.find_first_not_of(" \t\n\r"));
            MeasurementName.erase(MeasurementName.find_last_not_of(" \t\n\r") + 1);

            // Assign MeasurementId
            auto it = measurementNameToId.find(MeasurementName);
            if (it != measurementNameToId.end()) {
                MeasurementId = it->second;
            } else {
                MeasurementId = currentId++;
                measurementNameToId[MeasurementName] = MeasurementId;
            }

            // Extract numerical values
            getline(inputString, tempString, ',');
            MeasurementValue = std::atof(tempString.c_str());

            getline(inputString, tempString, ',');
            NominalValue = std::atof(tempString.c_str());

            getline(inputString, tempString, ',');
            UpperTolerance = std::atof(tempString.c_str());

            getline(inputString, tempString, ',');
            LowerTolerance = std::atof(tempString.c_str());

            tree->Fill();
        }
    }

    // Close
    tree->Write();
    outFile.Close();

    std::cout << "Processing complete. Output saved to new" 
              << boardName << "TreeFile.root" << std::endl;
}