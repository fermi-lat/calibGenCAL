// LOCAL INCLUDES
#include "CfData.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TGraph.h"
#include "TSpline.h"

// STD INCLUDES
#include <sstream>

// CONSTANTS //
const double mu2ci_thr_rat = 1.7;

CfData::CfData(CfCfg &cfg) :
  splineFunc("spline_fitter","pol2",0,4095),
  m_adcSum(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcN(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcMean(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_splineADC(tRngIdx::N_VALS),
  m_splineDAC(RngNum::N_VALS),
  m_cfg(cfg),
  m_dacArr(0)
{

  // need a c-style array of floats for DAC values.
  m_dacArr = new float[m_cfg.nDACs];
  copy(m_cfg.dacVals.begin(), m_cfg.dacVals.begin()+m_cfg.nDACs, m_dacArr);
}

// smooth test lines & print output.
void CfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.
  float *tmpADC(new float[m_cfg.nDACs]);

  for (RngNum rng; rng.isValid(); rng++) {
    // following vals only change w/ rng, so i get them outside the other loops.
    int grpWid  = m_cfg.splineGroupWidth[rng];
    //int splLen  = grpWid*2 + 1;
    int skpLo   = m_cfg.splineSkipLow[rng];
    int skpHi   = m_cfg.splineSkipHigh[rng];
    int nPtsMin = m_cfg.splineNPtsMin[rng];

    for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      tRngIdx rngIdx(faceIdx,rng);

      // point to current adc vector
      vector<float> &curADC = m_adcMean[rngIdx];
      // get pedestal
      float ped     = curADC[0];

      //calculate ped-subtracted means.
      for (int dac = 0; dac < m_cfg.nDACs; dac++)
        curADC[dac] -= ped;

      // get upper adc boundary
      float adc_max = curADC[m_cfg.nDACs-1];

      // last idx will be last index that is <= 0.99*adc_max
      // it is the last point we intend on using.
      int last_idx = 0; 
      while (curADC[last_idx] <= 0.99*adc_max) {
        last_idx++;
      }
      last_idx--;
      
      adc_max = curADC[last_idx]; 

      // set up new graph object for fitting.
      for (unsigned i = 0; i < curADC.size(); i++) tmpADC[i] = curADC[i];
      TGraph myGraph(last_idx+1,
                     m_dacArr,
                     tmpADC);

      if (!m_cfg.splineEnableGrp[rng]) {
        ////////////////////////
        // GROUPING DISNABLED //
        ////////////////////////
        for (int i = 0; i <= last_idx; i++) {
          // quit past the max_value
          if (curADC[i] > adc_max) break; 
          
          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list if needed
          if (m_splineADC[rngIdx].size() >  m_splineDAC[rng].size())
            m_splineDAC[rng].push_back((int)m_dacArr[i]);
        }
      } else {

        //////////////////////
        // GROUPING ENABLED //
        //////////////////////

        // PART I: NO SMOOTHING/GROUPING FOR SKIPLO PTS ////////////////////////
        // copy SKPLO points directly from beginning of array.

        int spl_idx = 0;
        for (int i = 0; i < skpLo; i++,spl_idx++) {
          // quit past the max_value- unlikely
          if (curADC[i] > adc_max) break; 
          
          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list if needed
          if (m_splineADC[rngIdx].size() >  m_splineDAC[rng].size())
            m_splineDAC[rng].push_back((int)m_dacArr[i]);
        }

        // PART II: GROUP & SMOOTH MIDDLE RANGE  ///////////////////////////////
        // start one grp above skiplo & go as hi as you can w/out entering skpHi
        for (int cp = skpLo + grpWid - 1; // cp = 'center point'
             cp < (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
             cp += grpWid, spl_idx++) {
          int lp = cp - grpWid; // 1st point in group
          int hp  = cp + grpWid; // last point in group

          // fit curve to grouped points
          myGraph.Fit(&splineFunc,"QN","",m_dacArr[lp],m_dacArr[hp]);
          float myPar1 = splineFunc.GetParameter(0);
          float myPar2 = splineFunc.GetParameter(1);
          float myPar3 = splineFunc.GetParameter(2);

          // use DAC value from center point
          int   fitDAC = (int)m_dacArr[cp];
          // eval smoothed ADC value
          float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

          // quit if we have past the max_adc_value
          if (fitADC > adc_max) break;

          // put new ADC val on list
          m_splineADC[rngIdx].push_back(fitADC);

          // put new DAC val on global output list if needed
          if (m_splineADC[rngIdx].size() > m_splineDAC[rng].size())
            m_splineDAC[rng].push_back(fitDAC);
        }

        // PART III: NO SMOOTHING/GROUPING FOR LAST SKPHI PTS //////////////////
        // copy SKPHI points directly from face of array.
        for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
             i <= last_idx;
             i++,spl_idx++) {
          // quit if past the max_adc_value
          if (curADC[i] > adc_max) break;

          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]); 

          // put new DAC val on global output list if needed
          if (m_splineADC[rngIdx].size() >  m_splineDAC[rng].size())
            m_splineDAC[rng].push_back((int)m_dacArr[i]);
        }
      }
    }
  }
  delete tmpADC;
}

void CfData::corr_FLE_Thresh(CfData& data_high_thresh){
  RngNum rng;  // we correct LEX8 rng only

  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    tRngIdx rngIdx(faceIdx,rng);

    int nSpline = m_splineADC[rngIdx].size();
    double* arrADC = new double(nSpline);
    double* arrDAC = new double(nSpline);
    for (int i = 0; i<nSpline; i++){
      arrADC[i] = m_splineADC[rngIdx][i];
      arrDAC[i] = m_splineDAC[rng][i];
    }
    int nSpline_hi_thr = data_high_thresh.getNSplineADC(rngIdx);
    double* arrADC_hi_thr = new double(nSpline_hi_thr);
    double* arrDAC_hi_thr = new double(nSpline_hi_thr);
    const vector<float>& splineADC = data_high_thresh.getSplineADC(rngIdx);
    const vector<int>& splineDAC = data_high_thresh.getSplineDAC(rng);


    for (int i = 0; i<nSpline_hi_thr; i++){
      arrADC_hi_thr[i] = splineADC[i];
      arrDAC_hi_thr[i] = splineDAC[i];
    }

    TSpline3* spl = new TSpline3("spl",arrDAC,arrADC,nSpline); 
    TSpline3* spl_hi_thr = new TSpline3("spl_hi_thr",arrDAC_hi_thr,
                                        arrADC_hi_thr,nSpline_hi_thr); 

    for (int i = 0; i<nSpline;i++){
      float dac_corr = arrDAC[i]/mu2ci_thr_rat;
      m_splineADC[rngIdx][i] = arrADC_hi_thr[i] + 
        spl->Eval(dac_corr)-spl_hi_thr->Eval(dac_corr);
    }

    delete spl;
    delete spl_hi_thr;
    delete arrADC;
    delete arrDAC;
    delete arrADC_hi_thr;
    delete arrDAC_hi_thr;

  }
}

void CfData::WriteSplinesTXT(const string &filename) {
  ofstream outFile(filename.c_str());
  if (!outFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw tmp.str();
  }
  outFile.precision(2);
  outFile.setf(ios_base::fixed);

  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    for (unsigned n = 0; n < m_splineADC[rngIdx].size(); n++) {
      RngNum rng = rngIdx.getRng();
      outFile << rngIdx.getLyr() << " "
              << rngIdx.getCol()  << " "
              << rngIdx.getFace() << " "
              << rng  << " "
              << m_splineDAC[rng][n] << " "
              << m_splineADC[rngIdx][n]
              << endl;
    }
}

void CfData::ReadSplinesTXT (const string &filename) {
  ifstream inFile(filename.c_str());
  if (!inFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw tmp.str();
  }

  //short twr = m_cfg.twrBay;
  short lyr;
  short col;
  short face;
  short rng;
  int tmpDAC;
  float tmpADC;
  while (inFile.good()) {
    // load in one spline val w/ coords
    inFile >> lyr 
           >> col
           >> face
           >> rng
           >> tmpDAC
           >> tmpADC;
    
    tRngIdx rngIdx(lyr,col,face,rng);

    m_splineADC[rngIdx].push_back(tmpADC);
    if (m_splineADC[rngIdx].size() > m_splineDAC[rng].size()) 
      m_splineDAC[rng].push_back(tmpDAC);
  }
}

void CfData::WriteSplinesXML(const string &filename, const string &dtdPath) {
  // setup output file
  ofstream xmlFile(filename.c_str());
  if (!xmlFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open xmlFile='" << filename << "'";
    throw tmp.str();
  }

  // input file
  ifstream dtdFile(dtdPath.c_str());
  if (!dtdFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open dtdPath='" << dtdPath << "'";
    throw tmp.str();
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/runCIFit.cxx,v 1.17 2005/07/07 23:43:49 fewtrell Exp $  -->" << endl;
  xmlFile << "<!-- Made-up  intNonlin XML file for EM, according to calCalib_v2r1.dtd -->" << endl;
  xmlFile << endl;
  xmlFile << "<!DOCTYPE calCalib [" << endl;
  string tmpStr;
  while (dtdFile.good()) {
    getline(dtdFile, tmpStr);
    if (dtdFile.fail()) continue; // bat get()
    xmlFile << tmpStr << endl;;
  }
  xmlFile << "] >" << endl;
  xmlFile << endl;
  xmlFile << "<calCalib>" << endl;
  xmlFile << "  <generic instrument=\"" << m_cfg.instrument
          << "\" timestamp=\"" << m_cfg.timestamp << "\"" << endl;
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"v2r2\" >" << endl;

  xmlFile << endl;
  xmlFile << "  </generic>" << endl;
  xmlFile << endl;
  xmlFile << "<!-- EM instrument: 8 layers, 12 columns -->" << endl;
  xmlFile << endl;
  xmlFile << "<!-- number of collections of dac settings should normally be" << endl;
  xmlFile << "     0 (the default), if dacs aren't used to acquire data, or " << endl;
  xmlFile << "     equal to nRange -->" << endl;
  xmlFile << " <dimension nRow=\"" << TwrNum::N_ROWS
          << "\" nCol=\""          << TwrNum::N_COLS
          << "\" nLayer=\""        << LyrNum::N_VALS 
          << "\" nXtal=\""         << ColNum::N_VALS 
          << "\" nFace=\""         << FaceNum::N_VALS 
          << "\" nRange=\""        << RngNum::N_VALS << "\"" << endl;
  xmlFile << "           nDacCol=\"" << RngNum::N_VALS << "\" />" << endl;

  //
  // DAC values for rest of file.
  //
  xmlFile << endl;
  for (RngNum rng; rng.isValid(); rng++) {
    xmlFile << " <dac range=\"" << RngNum::MNEM[rng] << "\"" << endl;
    xmlFile << "     values=\"";
    for (unsigned i = 0; i < m_splineDAC[rng].size(); i++) 
      xmlFile << m_splineDAC[rng][i] << " ";
    xmlFile << "\"" << endl;
    xmlFile << "     error=\"" << 0.1 << "\" />" << endl;
  }
  
  //
  // main data loop
  //
  
  xmlFile.setf(ios_base::fixed);
  xmlFile.precision(2);
  // TOWER // currently only using 1 tower.
  xmlFile << endl;
  // only using 1 tower right now
  for (TwrNum twr = m_cfg.twrBay; twr == m_cfg.twrBay; twr++) {
    xmlFile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;
    // LAYER //
    for (LyrNum lyr; lyr.isValid(); lyr++) {
      xmlFile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      // COL //
      for (ColNum col; col.isValid(); col++) {
        xmlFile << "   <xtal iXtal=\"" << col << "\">" << endl;
        // FACE //
        for (FaceNum face; face.isValid(); face++) {
          const string facestr = (face == NEG_FACE) ? "NEG" : "POS";
          xmlFile << "    <face end=\"" << facestr << "\">" << endl;
          // RNG //
          for (RngNum rng; rng.isValid(); rng++) {
            tRngIdx rngIdx(lyr,col,face,rng);

            xmlFile << "     <intNonlin range=\"" << RngNum::MNEM[rng] << "\"" << endl;
            // ADC VALS //
            xmlFile << "             values=\"";
            for (unsigned i = 0; i < m_splineADC[rngIdx].size(); i++)
              xmlFile << fixed << m_splineADC[rngIdx][i] << " ";

            xmlFile << "\"" << endl;
            xmlFile << "             error=\"" << 0.1 << "\" />" << endl;
          }
          xmlFile << "    </face>" << endl;
        }
        xmlFile << "   </xtal>" << endl;
      }
      xmlFile << "  </layer>" << endl;
    }
    xmlFile << " </tower>" << endl;
  }
  xmlFile << "</calCalib>" << endl;
}
