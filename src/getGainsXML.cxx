#include <iostream>
#include <string>

#include "muonCalib.h"

int main(int argc, char** argv)
{
  muonCalib mc;
  
  mc.ReadMuPeaks((argc >=2) ? argv[1] : "../output/mupeak.txt");
  mc.WriteMuPeaksXML((argc >= 3) ? argv[2] : "../xml/gains.xml");

  return 0;

}
