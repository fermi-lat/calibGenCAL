#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{

  if(argc <= 1) {
    std::cout << "First argument should be an ascii file containing calibration constants for muon slope" << std::endl;
    exit(1);
  }
    
   std::ifstream muslopesin(argv[1]); 
    
    float muslopes[8][12];

    while(1){
    
         float av; int layer,col,side;
                muslopesin 
                       >> col
                       >> layer 
                       >> av;
                       
        if(!muslopesin.good()) break;
                std::cout <<" " << layer 
                       <<" " << col
                       <<" " << av
                       << endl;

        muslopes[layer][col] = av;
    }
            
    
    
    
    char posface[] ="POS";
    char negface[] ="NEG";
    char naface[] = "NA";
    char* face;
    std::ofstream muslopesout("../xml/muslopes.xml"); 
    char  q= '"';

    muslopesout << "<?xml version=\"1.0\" ?>" << endl;

    muslopesout << "<!DOCTYPE calCalib SYSTEM \"calCalib_v2.dtd\" [] >" << endl;

    muslopesout << "<calCalib>" << endl;

    muslopesout << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_MuSlope\" fmtVersion=\"v3r3p2\">" << endl;

  muslopesout << "</generic>" << endl;

  muslopesout << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"1\" />" << endl;

  muslopesout << "<tower iRow=\"0\" iCol=\"0\">" << endl;

    for (int layer=0;layer <8;layer++){
      muslopesout << "    <layer iLayer=" << q << layer << q << ">" << endl;
        for(int col=0;col<12;col++){
            muslopesout << "      <xtal iXtal=" << q << col << q <<">" << endl;
                face = naface;
                muslopesout << "        <face end=" << q << face << q <<">" << endl;
                float av = -10*muslopes[layer][col];
                char* lex8 = "LEX8";
                char* lex1 = "LEX1";
                char* hex8 = "HEX8";
                char* hex1 = "HEX1";
                char* r[] = {lex8,lex1,hex8,hex1};
                for(int range=0; range <4; range++){
                    muslopesout <<"            <muSlope slope=" << q << av
                       <<q  <<" range=" << q << r[range] << q <<" />"
                       << endl;
                }
                muslopesout << "        </face>" << endl;
            
           muslopesout << "       </xtal>" << endl;

        }
      muslopesout << "     </layer>" << endl;

   }

  muslopesout << "</tower>" << endl << "</calCalib>" << endl;

}
