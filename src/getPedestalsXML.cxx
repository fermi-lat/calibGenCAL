#include <iostream>
#include <fstream>

#include "muonCalib.h"

int main(int argc, char** argv)
{
  muonCalib mc;
  
  mc.ReadCalPed((argc >=2) ? argv[1] : "../output/muped.txt");
  mc.WritePedXML((argc >= 3) ? argv[2] : "../xml/pedestals.xml");

  return 0;
}
