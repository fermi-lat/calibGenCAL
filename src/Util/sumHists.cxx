// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitLACHists.cxx,v 1.2 2008/04/22 19:02:47 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Sum all histograms with same name across multiple files into a new file.
    All histograms that only appear in at least one file will be copied to new file
    Side effect: All sub-directories from all files will be present in output file.
    Note: only histograms with same name and relative path within different fo;e will be summed.
*/


// EXTLIB INCLUDES
#include "TFile.h"
#include "TIterator.h"
#include "TClass.h"
#include "TH1.h"
#include "TROOT.h"

// STD INCLUDES
#include <string>
#include <vector>
#include <iostream>


using namespace std;

///   Recursively add all histograms and directories in inputDir to outputDir.
//    Any histograms with same name & path will be summed within outputDir
void sumDir(TDirectory &outputDir,
            TDirectory &inputDir) {

  /// don't know if inputDir is TFile or TDirectory, simplest is to
  /// treat all as if TDirectory, if I want to do this, I need to read
  /// all objs into memory (otherwise I would have to use
  /// GetListOfKeys() instead of GetList())
  ///
  /// if you know a better way, don't be shy!
  inputDir.ReadAll();

  TIterator *it = inputDir.GetList()->MakeIterator();
  TObject *obj;
  while ((obj = (TObject*)it->Next()) != 0) {
    TClass *cls = gROOT->GetClass(obj->ClassName());

    /// CASE 1: Obj is histogram
    if (cls->InheritsFrom("TH1")) {
      TH1 *hnew = (TH1*)obj;
      TH1 *hout = 0;
      const string hname(obj->GetName());
      /// CASE 1A: histogram already exist
      if ((hout = (TH1*)outputDir.Get(hname.c_str())) != 0) {
        cout << "Summing to existing histogram: " << inputDir.GetPathStatic() << "/" << hname << endl;
        hout->Add(hnew);
        outputDir.cd();
        hout->Write(0,TObject::kOverwrite);
      }
        
      /// CASE 1B: histogram does not already exit
      else {
        cout << "Adding new histogram: " << inputDir.GetPathStatic() << "/" << hname << endl;
        outputDir.Add(hnew);
        outputDir.cd();
        hnew->Write();
      }
    }

    /// CASE 2: Obj is directory
    else if (cls->InheritsFrom("TDirectory")) {
      TDirectory *const dnew = (TDirectory*)obj;
      char const*const dname = dnew->GetName();

      /// make subdir if it doesn't already exit
      TDirectory * dout = outputDir.GetDirectory(dname);
      if (dout == 0)
        dout = outputDir.mkdir(dname);

      /// recursively process subdirs
      sumDir(*dout, *dnew);
    }
  }
}

const string usage_str = 
  "sumHists.cxx outputPath.root [inputPath.root]+\n"
  "where: \n"
  " outputPath.root = output ROOT file\n"
  " inputPath.root  = 1 or more input ROOT files";

int main(const int argc, char const*const*const argv) {
  /// check commandline
  if (argc < 3) {
    cout << "Not enough paramters: " << endl;
    cout << usage_str << endl;
    return -1;
  }

  /// retrieve commandline args
  const string outputPath(argv[1]);
  
  
  const vector<string> inputPaths(&argv[2], &argv[argc]);
  cout << "Opening output file: " << outputPath << endl;
  TFile outputFile(outputPath.c_str(), "RECREATE");

  /// loop through each input
  for (unsigned nFile = 0; nFile < inputPaths.size(); nFile++) {
    const string inputPath = inputPaths[nFile];
    cout << "Opening input file: " << inputPath << endl;
    TFile fin(inputPath.c_str() ,"READ");

    sumDir(outputFile, fin);
  }
  
  outputFile.Close();
}
