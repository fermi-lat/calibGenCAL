#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{

  if(argc <= 2) {
	 std::cout << "Usage: getCorrPedestalsXML: infile xmloutfile\n";
    std::cout << "First argument should be an ascii file containing calibration constants for muon slope" << std::endl;
    exit(1);
  }
    std::ifstream mupeaksin(argv[1]); 
	 std::ofstream gainout(argv[2]);
    
    float mupeaks[8][12][2], mupeaksig[8][12][2];

    while(1){
    
         float av,rms; int layer,col,side;
                mupeaksin >> side
                       >> col
                       >> layer 
                       >> av
                       >> rms;
                       
        if(!mupeaksin.good()) break;
	std::cout <<" " << layer 
		  <<" " << col
		  <<" " << side
		  <<" " << av
		  <<" " << rms
		  << std::endl;

        mupeaks[layer][col][side] = av;
        mupeaksig[layer][col][side] = rms;
    }
            
    
    
    
    char posface[] ="POS";
    char negface[] ="NEG";
    char* face;
    char  q= '"';

  gainout << "<?xml version=\"1.0\" ?>" << std::endl;

  gainout << "<!DOCTYPE calCalib SYSTEM \"calCalib_v2.dtd\" [] >" << std::endl;

  gainout << "<calCalib>" << std::endl;

  gainout << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_Ped\" fmtVersion=\"v3r3p2\">" << std::endl;

  gainout << "</generic>" << std::endl;

  gainout << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"2\" />" << std::endl;

  gainout << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

    for (int layer=0;layer <8;layer++){
      gainout << "    <layer iLayer=" << q << layer << q << ">" << std::endl;
        for(int col=0;col<12;col++){
            gainout << "      <xtal iXtal=" << q << col << q <<">" << std::endl;
            for(int side = 0;side<2;side++){
                face = (side==1) ? posface : negface;
                gainout << "        <face end=" << q << face << q <<">" << std::endl;

		// change muon peak from 12.3 MeV to 11.2 MeV, 11.2 MeV is obtained by fitting MC energy spectrum deposited in a single crystal with a landau function, 11.2 MeV is the fit peak position while 12.3 MeV is the average value obtained from PDG.

                float av = 11.2/mupeaks[layer][col][side];
                float rms = mupeaksig[layer][col][side];
                char* lex8 = "LEX8";
                char* lex1 = "LEX1";
                char* hex8 = "HEX8";
                char* hex1 = "HEX1";
                char* r[] = {lex8,lex1,hex8,hex1};
                for(int range=0; range <4; range++){
                    gainout <<"            <calGain avg=" << q << av
			    <<q << " sig=" << q << rms << q <<" range=" << q << r[range] << q <<" />"
                       << std::endl;
                    av *= 8;
                }
                gainout << "        </face>" << std::endl;
            }
           gainout << "       </xtal>" << std::endl;

        }
      gainout << "     </layer>" << std::endl;

   }

  gainout << "</tower>" << std::endl << "</calCalib>" << std::endl;

}
