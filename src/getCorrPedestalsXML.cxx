#include <iostream>
#include <fstream>

#include "muonCalib.h"

int main(int argc, char** argv)
{
  muonCalib mc;
  
  mc.ReadCorrPed((argc >=2) ? argv[1] : "../output/mucorrped.txt");
  mc.WriteCorrPedXML((argc >= 3) ? argv[2] : "../xml/mucorrped.xml");

  return 0;
}
