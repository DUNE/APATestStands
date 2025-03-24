#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <ctime>
#include <TH1F.h>
#include <TF1.h>
#include <TH1.h>
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include <TLine.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TObject.h>
#include <TColor.h>
#include <iomanip>
#include <set>

void TreeReader(const char* boardName) {
    // Get current date for PDF naming
    time_t now = time(nullptr);
    struct tm *tstruct = localtime(&now);
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", tstruct);
    TString pdfName = Form("%s Hist %s.pdf", boardName, dateStr);

    // Define variables for the data in the tree
    int SerialNumber;
    int MeasurementId;
    std::string* MeasurementName = nullptr;
    std::string* BoardType = nullptr; 
    double MeasurementValue;
    double NominalValue;
    double UpperTolerance;
    double LowerTolerance;
    int Batch = 0;

    // Open the input file and get the tree
    TString inputFileName = Form("/home/tkdkrishna/root-6.30.06/macros/new%sTreeFile.root", boardName);
    TFile *inputFile = TFile::Open(inputFileName);
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening input file: " << inputFileName << std::endl;
        return;
    }

    //Can't get TTree
    TTree *tree = static_cast<TTree*>(inputFile->Get("tree"));
    if (!tree) {
        std::cerr << "Error getting TTree from input file" << std::endl;
        inputFile->Close();
        return;
    }

    // Set branch addresses
    tree->SetBranchAddress("MeasurementName",  &MeasurementName);
    tree->SetBranchAddress("BoardType",        &BoardType);
    tree->SetBranchAddress("SerialNumber",     &SerialNumber);
    tree->SetBranchAddress("MeasurementId",    &MeasurementId);
    tree->SetBranchAddress("MeasurementValue", &MeasurementValue);
    tree->SetBranchAddress("NominalValue",     &NominalValue);
    tree->SetBranchAddress("UpperTolerance",   &UpperTolerance);
    tree->SetBranchAddress("LowerTolerance",   &LowerTolerance);

    // Check if the Batch branch exists
    if (tree->GetBranch("Batch")) {
        tree->SetBranchAddress("Batch", &Batch);
    } else {
        std::cerr << "Warning: Batch branch not found. Proceeding without Batch information." << std::endl;
    }

    // Binning
    const int binNumber = 25000;

    // Map to store histograms depending on their ID
    std::map<int, TH1F*> measurementHistograms;

    // Maps for OOT tracking
    std::map<std::string, std::set<std::string>> uniqueBoardsWithOOTMeasurements;
    std::map<std::string, std::set<std::string>> distanceforOOT;
    std::map<std::string, std::pair<int, int>> toleranceCounts;
    std::map<int, std::pair<int, int>> batchSerialRanges;

    // Loop over entries
    const int numberOfEntries = tree->GetEntries();
    for (int i = 0; i < numberOfEntries; ++i) {
        tree->GetEntry(i);
        if (MeasurementId <= 0 || BoardType->compare(boardName) != 0) continue;

        // Track serial number ranges for each batch for summary data
        if (!batchSerialRanges.count(Batch)) {
            batchSerialRanges[Batch] = {SerialNumber, SerialNumber};
        } else {
            batchSerialRanges[Batch].first = std::min(batchSerialRanges[Batch].first, SerialNumber);
            batchSerialRanges[Batch].second = std::max(batchSerialRanges[Batch].second, SerialNumber);
        }

        if(std::string(boardName) == "AdapterBack"){
        LowerTolerance = -.2;
        UpperTolerance = .2;
        }

        // Trim measurement name to avoid errors
        std::string trimmedName = MeasurementName->substr(MeasurementName->find_first_not_of(' '));

        // Handle OOT measurements
        if (MeasurementValue < NominalValue + LowerTolerance || MeasurementValue > NominalValue + UpperTolerance) {
            toleranceCounts[trimmedName].first++;
            double distance = MeasurementValue - NominalValue;
            uniqueBoardsWithOOTMeasurements[trimmedName].insert(std::to_string(SerialNumber));
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << distance;
            distanceforOOT[std::to_string(SerialNumber) + trimmedName].insert(ss.str());
        }

        // Create a new histogram if encountered measurement name is new
        if (!measurementHistograms.count(MeasurementId)) {
            TString histName = Form("%s_%d", boardName, MeasurementId);
            TH1F* hist = new TH1F(
                histName.Data(),
                (std::string(MeasurementName->c_str()) + ";Measurement Value (mm);Number of Boards").c_str(), // Empty title (added later with TLatex)
                binNumber, 0, 250
            );
            hist->SetFillColor(17);
            measurementHistograms[MeasurementId] = hist;
        }

        TH1F* currentHist = measurementHistograms[MeasurementId];
        currentHist->Fill(MeasurementValue);

        // Configure histogram settings
        double xMin = NominalValue + 1.2 * LowerTolerance;
        double xMax = NominalValue + 1.2 * UpperTolerance;
        currentHist->GetXaxis()->SetRangeUser(xMin, xMax);

        // Add tolerance lines
        TLine *NominalLine = new TLine(NominalValue, 0, NominalValue, currentHist->GetMaximum() * 1.1);
        NominalLine->SetLineColor(kBlack);
        currentHist->GetListOfFunctions()->Add(NominalLine);

        TLine *UpperLine = new TLine(NominalValue + UpperTolerance, 0, 
                                   NominalValue + UpperTolerance, currentHist->GetMaximum() * 0.6);
        UpperLine->SetLineColor(kGreen);
        currentHist->GetListOfFunctions()->Add(UpperLine);

        TLine *LowerLine = new TLine(NominalValue + LowerTolerance, 0, 
                                   NominalValue + LowerTolerance, currentHist->GetMaximum() * 0.6);
        LowerLine->SetLineColor(kGreen);
        currentHist->GetListOfFunctions()->Add(LowerLine);

        // Gaussian fit
        if (currentHist->GetEntries() > 10) {
            TF1 *fit = new TF1("fit", "gaus");
            fit->SetRange(
                currentHist->GetMean() - 3 * currentHist->GetStdDev(),
                currentHist->GetMean() + 3 * currentHist->GetStdDev()
            );
            currentHist->Fit(fit, "QR");
        }

        // Update legend
        TCanvas *tempCanvas = new TCanvas("tempCanvas", "tempCanvas", 800, 600);
        TLegend *legend = new TLegend(0.63, 0.62, 0.895, 0.86);
        legend->SetBorderSize(0);
        legend->AddEntry((TObject*)0, Form("QTY: %d", (int)currentHist->GetEntries()), "");
        legend->AddEntry("", Form("OOT: %d", toleranceCounts[trimmedName].first), "");
        legend->AddEntry(UpperLine, "+/- Tolerance", "l");
        legend->AddEntry(NominalLine, "Target", "l");
        currentHist->GetListOfFunctions()->Add(legend);
        delete tempCanvas;
    }

    // Set Y-axis ranges
    for (auto& [id, hist] : measurementHistograms) {
        hist->GetYaxis()->SetRangeUser(0, hist->GetMaximum() * 1.2);
    }

    // PDF Generation
    TCanvas *canvas = new TCanvas("canvas", "canvas", 950, 600);
    canvas->Divide(2, 2);
    canvas->Print(pdfName + "["); // Start PDF

    int pageCounter = 0;
    TLatex latex;

    for (const auto& [id, hist] : measurementHistograms) {
        canvas->cd(pageCounter % 4 + 1);
        hist->SetStats(0);

        // Add title to the histogram
        std::string title = hist->GetTitle();
        if (hist->GetEntries() < 10) {
            title += " *NO LONGER COLLECTED*"; // Adds note to the title if entries are low
        }
        hist->SetTitle(title.c_str());

        hist->Draw();
        pageCounter++;

        if (pageCounter % 4 == 0) {
            canvas->Update();
            canvas->Print(pdfName);
            canvas->Clear();
            canvas->Divide(2, 2);
        }
    }

    // Draw remaining histograms
    if (pageCounter % 4 != 0) {
        canvas->Update();
        canvas->Print(pdfName);
    }

    // Add OOT summary pages
    TCanvas *summaryCanvas = new TCanvas("summaryCanvas", "Summary", 950, 600);
    TLatex summaryText;
    summaryText.SetTextAlign(12);
    double y = 0.90;
    double bottomMargin = 0.05;
    int pageNumber = 1;

    auto printNewPage = [&]() {
        summaryCanvas->Print(pdfName);
        summaryCanvas->Clear();
        y = 0.90;
        pageNumber++;
        summaryText.DrawLatex(0.1, y, Form("Summary of Failed Boards (Page %d)", pageNumber));
        y -= 0.1;
    };

    // Batch info
    summaryText.SetTextSize(0.04);
    summaryText.DrawLatex(0.1, y, "Serial Number Ranges by Batch:");
    y -= 0.05;
    for (const auto& [batch, range] : batchSerialRanges) {
        summaryText.DrawLatex(0.1, y, Form("Batch %d: #%04d - #%04d", batch, range.first, range.second));
        y -= 0.05;
    }
    printNewPage();

    // OOT details
    for (const auto& [name, serials] : uniqueBoardsWithOOTMeasurements) {
        if (y < bottomMargin) printNewPage();
        summaryText.SetTextSize(0.035);
        summaryText.DrawLatex(0.1, y, Form("Failed Measurement: %s", name.c_str()));
        y -= 0.05;

        if (y < bottomMargin) printNewPage();
        summaryText.SetTextSize(0.03);
        summaryText.DrawLatex(0.1, y, "Affected Boards:");
        y -= 0.05;

        for (const auto& serial : serials) {
            if (y < bottomMargin) printNewPage();
            summaryText.DrawLatex(0.15, y, Form("%s (%s mm)", 
                serial.c_str(), distanceforOOT[serial + name].begin()->c_str()));
            y -= 0.05;
        }
        y -= 0.05;
    }

    // Final cleanup
    summaryCanvas->Print(pdfName);
    canvas->Print(pdfName + "]");
    inputFile->Close();
}