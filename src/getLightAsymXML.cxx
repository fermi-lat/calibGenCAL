#include <iostream>
#include <fstream>

#include "muonCalib.h"

int main(int argc, char** argv)
{
  muonCalib mc;
  
  mc.ReadAsymTable((argc >=2) ? argv[1] : "../output/asym_table.txt");
  mc.WriteAsymXML((argc >= 3) ? argv[2] : "../xml/light_asym.xml");

  return 0;
}
