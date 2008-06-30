#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TStreamerInfo.h"

//----- GLAST includes

#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"

//----- local includes

#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/CGCUtil.h"

using namespace std;
using namespace CalUtil;
using namespace calibGenCAL;

int main(const int argc, const char **argv)
{
  if(argc!= 4)
    {
      cout << "Usage: splitDigi.exe <input filename> <base output filename> <number of even parts>" << endl;
      cout << "       for 2nd argument type something like 'inl_calibElement'. Part number and" << endl;
      cout << "       file's extension will be added" << endl;
      exit(0);
    }

  const string inpFname= argv[1];
  const string outFname= argv[2];
  const unsigned  nParts= atoi(argv[3]);



  long N_totl, N_part;
  unsigned  i, j, j0, j1;

  char buffer[3];
  vector<string> element(nParts);
  for(i= 0; i < nParts; i++)
    {
      sprintf(buffer, "%02d", i+1);
      element[i]= buffer;
    }

  //----- open input digi ROOT file and get "Digi" tree

  TChain inp0("Digi");
  if (inp0.AddFile(inpFname.c_str()) == 0) {
    cout << __FILE__ ": Error opening file: " << inpFname << endl;
    return -1;
  }

  //-- check DigiEvent version in code matches data --//
  const     int   codeDigiEventVer =
    DigiEvent::Class()->GetClassVersion();
  const int fileDigiEventVer = ((TStreamerInfo*)inp0.GetFile()->GetStreamerInfoList()->FindObject("DigiEvent"))->GetClassVersion();
  if (fileDigiEventVer != codeDigiEventVer) {
    cout  << "WARNING: digFile=" << inpFname << " "
           << fileDigiEventVer << " code is linked to DigiEvent version"
           << codeDigiEventVer << endl;
  }
      
  inp0.SetBranchStatus("*", 1);      // enable all branches in original tree
  N_totl= inp0.GetEntries();         // total number of events in original file
  N_part= (int)(N_totl/nParts);

  if(N_totl%nParts!= 0) { cout << "given number of parts cannot split the file evenly..\a" << endl; exit(0); }

  j0= 0; j1= N_part;

  // generate element output filenames
  vector<string> elementFilenames(nParts);
  for(i= 0; i < nParts; i++)
    elementFilenames[i] = outFname+"."+element[i]+".root";

  for(i= 0; i < nParts; i++)
    {
      cout << "creating file " << elementFilenames[i] << endl;

      TFile out0(elementFilenames[i].c_str(), "RECREATE");

      TTree* t1= inp0.CloneTree(0);  // clone original tree, but
      // do not copy any events yet
      for(j= j0; j < j1; j++)
        {
          if (j % 1000 == 0)
            cout << "Processing event: " << j << endl;
          inp0.GetEntry(j);
          t1->Fill();
        }

      t1->Write();
      out0.Close();

      j0+= N_part;
      j1+= N_part;
    }

  //----- check events ranges

  vector<string> rootFileList;

  cout << "-------- check events ranges --------" << endl;
  for(i= 0; i < nParts; i++)
    {
      rootFileList.clear();
      rootFileList.push_back(elementFilenames[i].c_str());
      RootFileAnalysis rootFile(0, &rootFileList, 0);

      //----- enable only needed branches

      rootFile.getDigiChain()->SetBranchStatus("*", 0);
      rootFile.getDigiChain()->SetBranchStatus("m_eventId");

      unsigned eventFirst=0, eventLast=0;
      unsigned eventn, nEvents= rootFile.getDigiChain()->GetEntries();

      for(j= 0; j < nEvents; j++)
        {
          if(!rootFile.getEvent(j)) { cout << "!cannot! read event:     " << j << endl; continue; }

          const DigiEvent *digiEvent= rootFile.getDigiEvent();
          if(!digiEvent)            { cout << "!cannot! read DigiEvent: " << j  << endl; continue; }

          eventn= digiEvent->getEventId();
          if(j== 0)         eventFirst= eventn;
          if(j== nEvents-1) eventLast=  eventn;
        }
      cout << elementFilenames[i] << " " << nEvents<< " events, " << eventFirst << " - " << eventLast << endl;
    }
}
