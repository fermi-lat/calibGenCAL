#include <iostream>
#include <fstream>

#include "muonCalib.h"

int main(int argc, char** argv)
{

  muonCalib mc;
  
  mc.ReadMuSlopes((argc >=2) ? argv[1] : "../output/muslope.txt");
  mc.WriteMuSlopesXML((argc >= 3) ? argv[2] : "../xml/muslopes.xml");

  return 0;
}
