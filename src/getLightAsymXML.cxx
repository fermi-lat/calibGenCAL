#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{

  if(argc <= 1) {
    std::cout << "First argument should be an ascii file containing calibration constants for muon slope" << std::endl;
    exit(1);
  }
    
   std::ifstream asymin(argv[1]); 
  float asym[12][8][10];  
    while(1){
    
          int layer,col,side;
                asymin 
                       >> layer 
                        >> col;
               for (int i=0;i<10;i++) asymin >> asym[col][layer][i];

                       
        if(!asymin.good()) break;
                std::cout <<" " << layer 
                       <<" " << col
                       << std::endl;

    }
            
    
    
    
    char posface[] ="POS";
    char negface[] ="NEG";
    char naface[] = "NA";
    char* face;
    std::ofstream asymout("../xml/light_asym.xml"); 
    char  q= '"';

    asymout << "<?xml version=\"1.0\" ?>" << std::endl;

    asymout << "<!DOCTYPE calCalib SYSTEM \"calCalib_v2.dtd\" [] >" << std::endl;

    asymout << "<calCalib>" << std::endl;

    asymout << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_MuSlope\" fmtVersion=\"v3r3p2\">" << std::endl;

  asymout << "</generic>" << std::endl;

  asymout << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"1\" nRange=\"2\" />" << std::endl;

  asymout << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

    for (int layer=0;layer <8;layer++){
      asymout << "    <layer iLayer=" << q << layer << q << ">" << std::endl;
        for(int col=0;col<12;col++){
            asymout << "      <xtal iXtal=" << q << col << q <<">" << std::endl;
                face = naface;
                asymout << "        <face end=" << q << face << q <<">" << std::endl;
                char* le = "LE";
                char* he = "HE";
                char* r[] = {le,he};
                for(int range=0; range <2; range++){
                    asymout <<"            <lightAsym diode=" << q << r[range]
                       <<q  <<" error=" << q << "0.03" << q << std::endl;
                       
					asymout << "              values=" << q;  
					for (int i=0;i<10;i++) asymout << " " << asym[col][layer][i];
					   asymout << q << " />" << std::endl;
                }
                asymout << "        </face>" << std::endl;
            
           asymout << "       </xtal>" << std::endl;

        }
      asymout << "     </layer>" << std::endl;

   }

  asymout << "</tower>" << std::endl << "</calCalib>" << std::endl;

}
