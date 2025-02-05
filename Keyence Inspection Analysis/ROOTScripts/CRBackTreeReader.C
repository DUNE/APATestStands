#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <TH1F.h>
#include <TF1.h>
#include <TH1.h>
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "array"
#include <TEntryListFromFile.h>
#include <TDirectory.h>
#include <map>
#include <TLine.h>
#include <TROOT.h>
#include <TStyle.h>
#include <memory>
#include <TLegend.h>
#include <TPDF.h>
#include <TPad.h>
#include <TCanvas.h>
#include <set>
#include <TLatex.h>
#include <TObject.h>
#include <TColor.h>
#include <sstream>
#include <iomanip>

void CRBackTreeReader() {
    // Define variables for the data in the tree
    int SerialNumber;
    int MeasurementId;
    std::string* MeasurementName = nullptr;
    std::string* BoardType = nullptr; 
    double MeasurementValue;
    double NominalValue;
    double UpperTolerance;
    double LowerTolerance;

    // Open the input file and get the tree
    TFile *inputFile = TFile::Open("/home/tkdkrishna/newCRBackTreeFile.root"); //! YOUR DIRECTORY HERE
    TTree *tree = static_cast<TTree*>(inputFile->Get("tree"));
   
    // Check if the tree was successfully opened
    if (!tree) {
        std::cerr << "Error getting TTree from input file" << std::endl;
        inputFile->Close();
        return;
    }

    // Set the branch addresses for the data in the tree
    tree->SetBranchAddress("MeasurementName",  &MeasurementName);
    tree->SetBranchAddress("BoardType",        &BoardType);
    tree->SetBranchAddress("SerialNumber",     &SerialNumber);
    tree->SetBranchAddress("MeasurementId",    &MeasurementId);
    tree->SetBranchAddress("MeasurementValue", &MeasurementValue);
    tree->SetBranchAddress("NominalValue",     &NominalValue);
    tree->SetBranchAddress("UpperTolerance",   &UpperTolerance);
    tree->SetBranchAddress("LowerTolerance",   &LowerTolerance);

    // Define constants for the histogram settings
    const int binNumber = 25000; 

    // Get the number of entries in the tree
    const int numberOfEntries = tree->GetEntries();

    // Define vectors to store the histograms for each board type
    std::vector<TH1F*> vec1CRBack;

    // Create empty histograms for each board type
    for(size_t h = 0; h <= 16; ++h) {
        vec1CRBack.push_back(new TH1F(("CRBackID_" + std::to_string(h)).c_str(),
                                      ";Measurement Value (mm);Number of CR Boards",
                                      binNumber, 0, 250));
    }

    std::map<std::string, std::set<std::string>> uniqueBoardsWithOOTMeasurements;
    std::map<std::string, std::set<std::string>> distanceforOOT;
    std::map<std::string, std::pair<int, int>> toleranceCounts;

    // Loop over the entries in the tree
    for (int i = 0; i < numberOfEntries; ++i) {
        tree->GetEntry(i);
        if (MeasurementId > 0) {
            if (BoardType->compare("CRBK") == 0) {
                // Remove leading and trailing spaces from the measurement name
                std::string trimmedMeasurementName = MeasurementName->substr(MeasurementName->find_first_not_of(' '));
                // Check if the measurement value is outside the tolerance range
                if (MeasurementValue < NominalValue + LowerTolerance || MeasurementValue > NominalValue + UpperTolerance) {
                    toleranceCounts[trimmedMeasurementName].first++;
                    // Calculate the distance between the failed measurement and the nominal value
                    double distance = (MeasurementValue - NominalValue);
                    // Add the measurement name and distance to the uniqueBoardsWithOOTMeasurements map
                    uniqueBoardsWithOOTMeasurements[trimmedMeasurementName].insert(std::to_string(SerialNumber));
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(3) << distance;
                    distanceforOOT[(std::to_string(SerialNumber) + trimmedMeasurementName)].insert(ss.str());
                }

                vec1CRBack[MeasurementId]->SetNameTitle(("CRBack_" + *MeasurementName).c_str(),
                (";Measurement Value (mm);Number of CR Boards"));
                
                vec1CRBack[MeasurementId]->SetTitle(("CRBack_" + *MeasurementName).c_str());
                vec1CRBack[MeasurementId]->Fill(MeasurementValue);

                vec1CRBack[MeasurementId]->SetFillColor(17);

                // Nominal line
                TLine *NominalValueLine = new TLine(NominalValue, 0, NominalValue, 
                vec1CRBack[MeasurementId]->GetMaximum()*1.1);
                NominalValueLine->SetLineColor(kBlack);
                NominalValueLine->SetLineWidth(2);
                vec1CRBack[MeasurementId]->GetListOfFunctions()->Add(NominalValueLine);
                
                // + Tolerance line
                TLine *positiveToleranceLine = new TLine(NominalValue + UpperTolerance, 0, NominalValue + UpperTolerance, (vec1CRBack[MeasurementId]->GetMaximum())*.6);
                positiveToleranceLine->SetLineColor(kGreen);
                positiveToleranceLine->SetLineStyle(1);
                vec1CRBack[MeasurementId]->GetListOfFunctions()->Add(positiveToleranceLine);

                // - Tolerance line
                TLine *negativeToleranceLine = new TLine(NominalValue + LowerTolerance, 0, NominalValue + LowerTolerance, (vec1CRBack[MeasurementId]->GetMaximum())* .6);
                negativeToleranceLine->SetLineColor(kGreen);
                negativeToleranceLine->SetLineStyle(1);
                vec1CRBack[MeasurementId]->GetListOfFunctions()->Add(negativeToleranceLine);
                
                // Set Histogram Range
                vec1CRBack[MeasurementId]->GetXaxis()->SetRangeUser(NominalValue + (3* LowerTolerance), NominalValue + (3*UpperTolerance));

                // Gaussian Fit
                TF1 *fit = new TF1("fit", "gaus");
                fit->Clear();
                fit->SetRange((vec1CRBack[MeasurementId]->GetMean()) - (3 * (vec1CRBack[MeasurementId]->GetStdDev())), (vec1CRBack[MeasurementId]->GetMean()) + (3 * (vec1CRBack[MeasurementId]->GetStdDev())));
                vec1CRBack[MeasurementId]->Fit(fit, "QR");

                vec1CRBack[MeasurementId]->SetStats(0);

                // Check if the function is of type TLegend
                int functionIndex = 0;
                TObject *obj = nullptr;
                while ((obj = vec1CRBack[MeasurementId]->GetListOfFunctions()->At(functionIndex)) != nullptr) {
                    if (dynamic_cast<TLegend*>(obj)) {
                        // Delete the existing legend
                        vec1CRBack[MeasurementId]->GetListOfFunctions()->Remove(obj);
                        delete obj;
                        break;
                    }
                    functionIndex++;
                }

                TLegend *legend = new TLegend(0.63, 0.62, .895, 0.86); 

                legend->Clear();
                legend->SetBorderSize(0);
                legend->SetFillColor(0);
                legend->SetFillStyle(0);
                legend->SetTextSize(0.05);
                legend->SetTextFont(42);
                
                legend->AddEntry((TObject*)0, Form("QTY: %d", (int)vec1CRBack[MeasurementId]->GetEntries()), "");
                legend->AddEntry("", ("OOT: " + std::to_string(toleranceCounts[trimmedMeasurementName].first)).c_str(), "");
                legend->AddEntry(positiveToleranceLine, "+/-Tolerance", "l");
                legend->AddEntry(NominalValueLine, "Target Value", "l");
                legend->AddEntry(fit, "Gaussian", "l");
                vec1CRBack[MeasurementId]->GetListOfFunctions()->Add(legend);

            } else {
                continue;
            }
        } else {
            continue;
        }
    }

    for(size_t h = 0; h <= 16; ++h) {

        // Calculate the maximum bin content for the y-axis range
        double maxBinContent = (vec1CRBack[h]->GetMaximum()) * 1.2;

        // Set the y-axis range for the histogram
        vec1CRBack[h]->GetYaxis()->SetRangeUser(0, maxBinContent);
    }

    std::vector<TCanvas*> Canvases;
    for(size_t k = 0; k <= 4; ++k) {
        Canvases.push_back(new TCanvas(("c" + std::to_string(k)).c_str(), ("quadrants" +std::to_string(k)).c_str()));
    }
    Canvases[0]->Divide(2,2);
    Canvases[0]->Print("CRBackHist.pdf[");

    int canvasNum = 0;
    for(int j = 1; j <= 16; ++j) {
        if(j == 16){
            Canvases[canvasNum]->cd(4);
            vec1CRBack[j]->Draw();
            Canvases[canvasNum]->Print("CRBackHist.pdf");
        } else if(j % 4 != 0){
            Canvases[canvasNum]->cd(j % 4);
            vec1CRBack[j]->Draw();
            Canvases[canvasNum]->Update();
            gPad->Update();
        } else{
            Canvases[canvasNum]->cd(4);
            vec1CRBack[j]->Draw();
            Canvases[canvasNum]->Print("CRBackHist.pdf");
            canvasNum = canvasNum + 1;
            Canvases[canvasNum]->Divide(2,2);
        }
    }

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 950, 600);
    TLatex* text = new TLatex();
    text->SetTextAlign(12);
    text->SetTextSize(0.02);

    // Calculate the x and y coordinates for the text
    Double_t x = gPad->GetLeftMargin() - 0.04;
    Double_t y = 1 - gPad->GetTopMargin() - 0.01;

    int num = 0;
    // Loop through the unique boards with out of tolerance measurements
    for (const auto& MeasurementName : uniqueBoardsWithOOTMeasurements) {
        // Print the measurement name
        text->SetTextSize(0.02);
        text->SetTextColor(kBlack);
        text->DrawLatexNDC(x, y, Form("\tMeasurement Failed: %s", MeasurementName.first.c_str()));
        y -= 0.02; // Move the y coordinate down for the next line

        // Print the serial numbers of boards that failed the current measurement
        text->SetTextColor(1);
        text->DrawLatexNDC(x, y, "\tSerial Numbers of failed boards:");
        y -= 0.03;

        

        // Loop through the serial numbers and add them to the text
        std::stringstream ss;
        for (const auto& SerialNumber : MeasurementName.second) {
            // Get the distance value from the distanceforOOT map
            std::string distanceStr = *distanceforOOT[SerialNumber + MeasurementName.first].begin();

            // Add the serial number and distance value to the text
            ss << SerialNumber << " (" << distanceStr << "), ";
            num++;

            if (num == 5) {
                // Print the serial numbers on the same line
                text->DrawLatexNDC(x + 0.02, y, ss.str().c_str());
                ss.str(""); // Clear the stringstream
                y -= 0.03; // Move the y coordinate down for the next line
                num = 0; // Reset the counter
            }
        }


        // Remove the trailing comma and space
        std::string failedSerialNumbersStr = ss.str();
        if (!failedSerialNumbersStr.empty()) {
            failedSerialNumbersStr.pop_back();
            failedSerialNumbersStr.pop_back();
        }
        text->SetTextSize(.017);
        text->DrawLatexNDC(x + 0.02, y, failedSerialNumbersStr.c_str());
        y -= 0.04; // Move the y coordinate down for the next line
    }
    text->SetTextSize(.02);

    // Print the number of rejected boards and the yield percentage
    text->SetTextColor(1); 
    y -= 0.02; 

    y -= .05;
    text->DrawLatexNDC(x, y, Form("Batch Date: Received 8/24 (IMAGINEERING)"));

    y -= .05;
    text->DrawLatexNDC(x, y, Form("Yield Percentage: 90.59"));
    // Draw the canvas and save the PDF
    canvas1->Print("CRBackHist.pdf");

    // Create a new map to store serial numbers and their corresponding failed measurements
    std::map<std::string, std::set<std::string>> failedBoardMeasurements;

    // Populate the new map
    for (const auto& measurement : uniqueBoardsWithOOTMeasurements) {
        for (const auto& serialNumber : measurement.second) {
            failedBoardMeasurements[serialNumber].insert(measurement.first);
        }
    }

    // Define the initial canvas
    TCanvas *summaryCanvas = new TCanvas("summaryCanvas", "Summary of Failed Boards", 950, 600);
    summaryCanvas->cd();
    TLatex* summaryText = new TLatex();
    summaryText->SetTextAlign(12);
    summaryText->SetTextSize(0.02);

    // Initialize coordinates for text placement
    Double_t xHeader = gPad->GetLeftMargin();
    Double_t yHeader = 1 - gPad->GetTopMargin() - 0.03;

    // Print the header
    summaryText->SetTextColor(kBlack);
    summaryText->DrawLatexNDC(xHeader, yHeader, "Summary of Failed Boards:");
    yHeader -= 0.04; // Adjust for spacing

    // Loop through the map to print serial numbers and their failed measurements
    for (const auto& board : failedBoardMeasurements) {
        // Check if the y-coordinate is too low for new content
        if (yHeader < gPad->GetBottomMargin()) {
            // Save the current page and create a new canvas
            summaryCanvas->Print("CRBackHist.pdf");
            delete summaryCanvas; // Free memory
            summaryCanvas = new TCanvas("summaryCanvas", "Summary of Failed Boards", 950, 600);
            summaryCanvas->cd();
            yHeader = 1 - gPad->GetTopMargin() - 0.03; // Reset yHeader
        }

        // Print the serial number
        summaryText->SetTextColor(kBlack);
        summaryText->DrawLatexNDC(xHeader, yHeader, Form("Serial Number: %s", board.first.c_str()));
        yHeader -= 0.05;

        // Print the failed measurements for this serial number
        summaryText->SetTextColor(kRed);
        for (const auto& measurement : board.second) {
            // Check if the y-coordinate is too low for new content
            if (yHeader < gPad->GetBottomMargin()) {
                // Save the current page and create a new canvas
                summaryCanvas->Print("CRBackHist.pdf");
                delete summaryCanvas; // Free memory
                summaryCanvas = new TCanvas("summaryCanvas", "Summary of Failed Boards", 950, 600);
                summaryCanvas->cd();
                yHeader = 1 - gPad->GetTopMargin() - 0.03; // Reset yHeader
            }

            summaryText->DrawLatexNDC(xHeader + 0.02, yHeader, Form("Failed Measurement: %s", measurement.c_str()));
            yHeader -= 0.04;
        }
    }
    // Save the final page
    summaryCanvas->Print("CRBackHist.pdf");
    delete summaryCanvas;

    // Close the PDF file
    TCanvas* closingCanvas = new TCanvas("closingCanvas", "Closing Page", 950, 600);
    closingCanvas->Print("CRBackHist.pdf]");
    delete closingCanvas;

}