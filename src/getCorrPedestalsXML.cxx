#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  if(argc <= 2) {
	 std::cout << "Usage: getCorrPedestalsXML infile xmloutfile\n";
    std::cout << "First argument should be an ascii file containing calibration constants for pedestal" << std::endl;
    exit(1);
  }

  std::ifstream mupedIn(argv[1]); 
  std::ofstream pedout(argv[2]);
    
  float muped[8][12][2][4];
  float mupedRms[8][12][2][4];
  float mupedCos[8][12][2][2];

  while(1){
    
    float av, rms, cosAngle; 
    int layer,col,side, rng;
    mupedIn >> layer >> col >> side >> rng >> av >> rms >> cosAngle;
                       
    if(!mupedIn.good()) break;
    std::cout <<" " << layer <<" " << col <<" " << av << " " << rms 
      <<" "<< cosAngle << endl;

    muped[layer][col][side][rng] = av;
    mupedRms[layer][col][side][rng] = rms;
    mupedCos[layer][col][side][rng/2] = cosAngle;

  }
   
  char posface[] ="POS";
  char negface[] ="NEG";
  char* face;
  char  q= '"';

  pedout << "<?xml version=\"1.0\" ?>" << endl;

  pedout << "<!DOCTYPE calCalib SYSTEM \"calCalib_v2.dtd\" [] >" << endl;

  pedout << "<calCalib>" << endl;

  pedout << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_Ped\" fmtVersion=\"v3r3p2\">" << endl;

  pedout << "</generic>" << endl;

  pedout << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"2\" />" << endl;

  pedout << "<tower iRow=\"0\" iCol=\"0\">" << endl;

  for (int layer=0;layer <8;layer++){
    pedout << "    <layer iLayer=" << q << layer << q << ">" << endl;
    for(int col=0;col<12;col++){
      pedout << "      <xtal iXtal=" << q << col << q <<">" << endl;
      for(int side = 0;side<2;side++){
	face = (side==1) ? posface : negface;
	pedout << "        <face end=" << q << face << q <<">" << endl;

	// only use first range for now
	int rng = 0;

	float av = muped[layer][col][side][rng];
	float rms = mupedRms[layer][col][side][rng];
	float cosAngle = mupedCos[layer][col][side][rng/2];
	char* lex8 = "LEX8";
	char* lex1 = "LEX1";
	char* hex8 = "HEX8";
	char* hex1 = "HEX1";
	char* r[] = {lex8,lex1,hex8,hex1};
	for(int range=0; range <4; range++)
	  pedout <<"            <calPed avg=" << q << av
		 <<q <<" sig=" << q << rms
		 <<q <<" cos=" << q << (range%2)?-1000.:cosAngle;
		 << q <<" range=" << q << r[range] << q <<" />"
		 << endl;
	pedout << "        </face>" << endl;

      }
      pedout << "       </xtal>" << endl;

    }
    pedout << "     </layer>" << endl;

  }

  pedout << "</tower>" << endl << "</calCalib>" << endl;

}
