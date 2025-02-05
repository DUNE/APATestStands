#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "TTree.h"
#include "TFile.h"

void CRBackFileReader(int startSerial, int endSerial) {
    std::string BoardType;
    int SerialNumber;
    int MeasurementId;
    std::string MeasurementName;
    double MeasurementValue;
    double NominalValue;
    double UpperTolerance;
    double LowerTolerance;
    std::string tempString;
    std::stringstream inputString;

    TFile outFile("newCRBackTreeFile.root", "RECREATE");
    TTree *tree = new TTree("tree", "tree");

    tree->Branch("BoardType",        &BoardType);
    tree->Branch("SerialNumber",     &SerialNumber);
    tree->Branch("MeasurementId",    &MeasurementId);
    tree->Branch("MeasurementName",  &MeasurementName);
    tree->Branch("MeasurementValue", &MeasurementValue);
    tree->Branch("NominalValue",     &NominalValue);
    tree->Branch("UpperTolerance",   &UpperTolerance);
    tree->Branch("LowerTolerance",   &LowerTolerance);

    BoardType = "CRBK";

    std::ofstream errorLog("fileOpenErrors.txt");

    for (int i = startSerial; i <= endSerial; i++) {
        std::string filename = "/home/tkdkrishna/BoardData/CRBack/";

        if (i < 10) {
            filename += BoardType + "Data000" + std::to_string(i) + ".txt";
        } else if (i < 100) {
            filename += BoardType + "Data00" + std::to_string(i) + ".txt";
        } else if (i < 1000) {
            filename += BoardType + "Data0" + std::to_string(i) + ".txt";
        } else {
            filename += BoardType + "Data" + std::to_string(i) + ".txt";
        }

        SerialNumber = i;

        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cout << "could not open " << filename << std::endl;
            errorLog << "could not open " << filename << std::endl;
            continue;
        }

        std::string line;
        while (getline(inputFile, line)) {
            std::stringstream inputString(line);

            getline(inputString, tempString, ',');
            MeasurementId = std::atoi(tempString.c_str());
            tempString = "";

            getline(inputString, MeasurementName, ',');

            getline(inputString, tempString, ',');
            MeasurementValue = std::atof(tempString.c_str());
            tempString = "";

            getline(inputString, tempString, ',');
            NominalValue = std::atof(tempString.c_str());
            tempString = "";

            getline(inputString, tempString, ',');
            UpperTolerance = std::atof(tempString.c_str());
            tempString = "";

            getline(inputString, tempString, ',');
            LowerTolerance = std::atof(tempString.c_str());
            tempString = "";

            line = "";
            tree->Fill();
        }
    }

    tree->Write();
    outFile.Close();
    errorLog.close();
}