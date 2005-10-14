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

CfData::CfData(CfCfg &cfg,
               const string &histFilename) :
  splineFunc("spline_fitter","pol2",0,4095),
  m_adcMean(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_splineADC(tRngIdx::N_VALS),
  m_splineDAC(tRngIdx::N_VALS),
  m_cfg(cfg),
  m_testDAC(0),
  m_histFilename(histFilename)
{
  // need a c-style array of floats for DAC values.
  m_testDAC = new float[m_cfg.nDACs];
  copy(m_cfg.dacVals.begin(), m_cfg.dacVals.end(), m_testDAC);

  if (m_cfg.genHistfile) {
    openHistfile();
    createHists();
  }
}

CfData::~CfData() {
  closeHistfile();

  delete m_testDAC;
}

void CfData::createHists() {
  // profiles owned by current ROOT directory/m_histFile.
  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    ostringstream tmp;
    tmp << "ciRaw_" << rngIdx;
    m_ciRawProfs.push_back(new TProfile(tmp.str().c_str(), 
                                        tmp.str().c_str(),
                                        m_cfg.nDACs,0, m_cfg.nDACs));

    ostringstream mgName;
    mgName << "ciOverlay_" << rngIdx;
    TMultiGraph *mg = new TMultiGraph();
    mg->SetNameTitle(mgName.str().c_str(),
                     mgName.str().c_str());
    

    m_ciGraphs.push_back(mg);

    // add graph to file so it will be written out.
    m_histFile->Add(mg);
  }
}

/** Generate spline points for each channel from raw data.
    END POINT
    - In all circumstances, the

    RAW MODE
    - save w/out alteration all points in raw data
    SMOOTHING MODE
    - save w/out alteration skipLow points from beginning of raw data
    - save w/out alteration skipHigh points from end of raw data
    - group middle points in sets of groupWidth
    - fit a quadratic to each group & save the midpoint.
    
*/
void CfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.
  float *tmpADC(new float[m_cfg.nDACs]);

  // Loop through all 4 energy ranges
  for (RngNum rng; rng.isValid(); rng++) {
    // following vals only change w/ rng, so i get them outside the other loops.
    int grpWid  = m_cfg.splineGroupWidth[rng];
    int skpLo   = m_cfg.splineSkipLow[rng];
    int skpHi   = m_cfg.splineSkipHigh[rng];

    // loop through each xtal face.
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
      
      // set up new graph object for fitting.
      copy(curADC.begin(),
           curADC.end(),
           tmpADC);

      TGraph myGraph(last_idx+1,
                     m_testDAC,
                     tmpADC);

      if (!m_cfg.splineEnableGrp[rng]) {
        ////////////////////////
        // GROUPING DISABLED  //
        ////////////////////////
        for (int i = 0; i <= last_idx; i++) {
          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list
          m_splineDAC[rngIdx].push_back(m_testDAC[i]);
        }
      } else {

        //////////////////////
        // GROUPING ENABLED //
        //////////////////////

        // PART I: NO SMOOTHING/GROUPING FOR SKIPLO PTS ////////////////////////
        // copy SKPLO points directly from beginning of array.

        for (int i = 0; i < skpLo; i++) {
          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list
          m_splineDAC[rngIdx].push_back(m_testDAC[i]);
        }

        // PART II: GROUP & SMOOTH MIDDLE RANGE  ///////////////////////////////
        // start one grp above skiplo & go as hi as you can w/out entering skpHi
        for (int ctrIdx = skpLo + grpWid - 1; 
             ctrIdx <= last_idx - max(skpHi,grpWid-1); // allow last group to overlap skpHi section.
             ctrIdx += grpWid) {

          int lp = ctrIdx - grpWid; // 1st point in group
          int hp  = ctrIdx + grpWid; // last point in group

          // fit curve to grouped points
          myGraph.Fit(&splineFunc,"QN","",m_testDAC[lp],m_testDAC[hp]);
          float myPar1 = splineFunc.GetParameter(0);
          float myPar2 = splineFunc.GetParameter(1);
          float myPar3 = splineFunc.GetParameter(2);

          // use DAC value from center point
          float fitDAC = m_testDAC[ctrIdx];
          // eval smoothed ADC value
          float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

          // put new ADC val on list
          m_splineADC[rngIdx].push_back(fitADC);

          // put new DAC val on global output list
          m_splineDAC[rngIdx].push_back(fitDAC);
        }

        // PART III: NO SMOOTHING/GROUPING FOR LAST SKPHI PTS //////////////////
        // copy SKPHI points directly from face of array.
        for (int i = last_idx+1 - skpHi;
             i <= last_idx;
             i++) {
          // put new ADC val on list
          m_splineADC[rngIdx].push_back(curADC[i]); 

          // put new DAC val on global output list
          m_splineDAC[rngIdx].push_back(m_testDAC[i]);
        }

      } // grouping enabled

      //-- EXTRAPOLATE FINAL POINT --//
      int nPts    = m_splineADC[rngIdx].size();
      float dac1  = m_splineDAC[rngIdx][nPts-2];
      float dac2  = m_splineDAC[rngIdx][nPts-1];
      float adc1  = m_splineADC[rngIdx][nPts-2];
      float adc2  = m_splineADC[rngIdx][nPts-1];
      float slope = (dac2 - dac1)/(adc2 - adc1);
      float dac_max = dac2 + (adc_max - adc2)*slope;

      // put final point on the list.
      m_splineADC[rngIdx].push_back(adc_max);
      m_splineDAC[rngIdx].push_back(dac_max);
    } // xtalFace lop
  } // range loop

  delete tmpADC;
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
              << m_splineDAC[rngIdx][n] << " "
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
    m_splineDAC[rngIdx].push_back(tmpDAC);
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
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CfData.cxx,v 1.3 2005/10/11 23:25:02 fewtrell Exp $  -->" << endl;
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
  xmlFile << "           nDacCol=\"" << 0 << "\" />" << endl;

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

            // DAC VALS //
            xmlFile << "             sdacs=\"";
            for (unsigned i = 0; i < m_splineDAC[rngIdx].size(); i++)
              xmlFile << fixed << m_splineDAC[rngIdx][i] << " ";
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


void CfData::makeGraphs() {
  // reuse to create each graphs, set to max possible size
  float *tmpADC = new float[m_cfg.nDACs];
  float *tmpDAC = new float[m_cfg.nDACs];

  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {

    //-- FIRST GRAPH: ALL INPUT DATA POINTS, POST OUTLIER REJECTION --//
    // generate plot name
    ostringstream cleanName;
    cleanName << "ciCleanInput_" << rngIdx;

    // copy ADC points into temp array
    copy(m_adcMean[rngIdx].begin(),
         m_adcMean[rngIdx].end(),
         tmpADC);
    
    // no need to retain graph pointer as ROOT handles my memory for me
    TGraph *tmpGrClean = new TGraph(m_adcMean[rngIdx].size(),
                                    m_testDAC, tmpADC);

    tmpGrClean->SetNameTitle(cleanName.str().c_str(),
                             cleanName.str().c_str());

    //-- SECOND GRAPH: OUTPUT SPLINE POINTS --//
    // generate plot name
    ostringstream splineName;
    splineName << "ciSpline_" << rngIdx;

    // copy ADC & DAC points into temp arrays
    copy(m_splineADC[rngIdx].begin(), 
         m_splineADC[rngIdx].end(), 
         tmpADC);
    copy(m_splineDAC[rngIdx].begin(), 
         m_splineDAC[rngIdx].end(), 
         tmpDAC);

    // no need to retain graph pointer as ROOT handles my memory for me
    TGraph *tmpGrSpline = new TGraph(getNSplineADC(rngIdx),
                                     tmpDAC,
                                     tmpADC);
             
    tmpGrSpline->SetNameTitle(splineName.str().c_str(),
                              splineName.str().c_str());

             

    m_ciGraphs[rngIdx]->Add(tmpGrClean);
    m_ciGraphs[rngIdx]->Add(tmpGrSpline);
  }

  // clean up memory
  delete tmpADC;
  delete tmpDAC;
}

void CfData::closeHistfile() {
  // write current histograms to file & close if we have an open file.
  if (m_histFile.get()) {
    m_histFile->Write();
    m_histFile->Close(); // all histograms deleted.
    delete m_histFile.release();
  }

}

void CfData::openHistfile() {
  m_histFile.reset(new TFile(m_histFilename.c_str(), 
                             "RECREATE", 
                             "IntNonlinHists",
                             9));
}
