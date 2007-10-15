#ifndef ROOTUtil_h
#define ROOTUtil_h

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/ROOTUtil.h,v 1.3 2007/10/05 11:39:39 fewtrell Exp $

/** @file
    @author  Zachary Fewtrell
    
    @brief collection of utilities for working w/ CERN ROOT library objects
*/

// EXTLIB INCLUDES
#include "TKey.h"
#include "TDirectory.h"
#include "TClass.h"
#include "TF1.h"
#include "TROOT.h"

// LOCAL INCLUDES
#include "stl_util.h"

// STD INCLUDES
#include <cassert>
#include <string>
#include <vector>

namespace calibGenCAL {
  /// use this method to retrieve a histogram of given
  /// type and name out of a root file
  /// \return ptr to hist obj if successful, NULL ptr otherwise
  template <class T>
  T *retrieveROOTObj(const TDirectory &rootDir,
                     const std::string &objname) {
    TKey *const key(rootDir.FindKey(objname.c_str()));
   
    // skip missing hist
    if (!key)
      return NULL;
              
    const TClass *const cls(gROOT->GetClass(key->GetClassName()));
    if (!cls)
      return NULL;
                           
    T  *const hist_ptr(dynamic_cast<T *>(key->ReadObj()));
    if (!hist_ptr) return NULL;
                                              
    // skip hist if it's the wrong type
    if (!hist_ptr->InheritsFrom(T::Class()))
      return NULL;
                                                                                        
    return hist_ptr;
  }

  /// return collection of all ROOT objects below given dir which match given type & string prefix.
  template <class T>
  std::vector<T*> harvestROOTObjs(const TDirectory &rootDir,
                                  const std::string &prefix="") {

    /// retrieve list of objects in dir
    TList *const keyList = rootDir.GetListOfKeys();
    /// critical error
    assert(keyList != 0);

    /// construct (initially empty) list to return
    std::vector<T*> retVal;

    /// iterate through list.
    TIterator *keyIter = keyList->MakeIterator();
    while (TKey *const key = dynamic_cast<TKey *const>(keyIter->Next())) {
      const TClass *const cls(gROOT->GetClass(key->GetClassName()));
      assert(cls != 0);

      /// if key is sub dir, recursively scan it
      if (cls->InheritsFrom(TDirectory::Class())) {
        TDirectory *const subdir = dynamic_cast<TDirectory *const>(key->ReadObj());
        append(retVal, harvestROOTObjs<T>(*subdir, prefix));          
      }

      /// if key is correct type, check name match & append it to list
      else if (cls->InheritsFrom(T::Class())) {
        /// check name prefix
        if (!prefix.empty()) {
          const std::string name(key->GetName());
            
          /// skip if name does not match
          if (name.find(prefix) != 0)
            continue;

          /// finally add obj to list
          retVal.push_back(dynamic_cast<T*>(key->ReadObj()));
        }
      }
    }

    return retVal;
  }
                     
  /// create new 1D histogram w/ residuals from fitted 1D histogram
  /// and 1st TF1 on histogram list-of-fuctions
  template<class T>
  T *createResidHist(const T &fittedHist) {
    // retrieve previous histogram info
    const std::string name(fittedHist.GetName());
   
    const std::string title(fittedHist.GetTitle());
     
    const int    nBins = fittedHist.GetNbinsX();
    const double xlo   = fittedHist.GetXaxis()->GetXmin();
    const double xhi   = fittedHist.GetXaxis()->GetXmax();
           
    name  += "_resid";
    title += "_resid";
               
    T *    resid = new T(name.c_str(),
                         title.c_str(),
                         nBins,
                         xlo,
                         xhi);
                 
    resid->Add(&fittedHist);
    resid->Add(dynamic_cast<TF1*>(fittedHist.GetListOfFunctions()->At(0)),
               -1);
                     
    return resid;
  }
                     
  /// create new sub-directory inside of parent if it doesn't exist
  /// \return ref to new subdir regardless of whether I had to make one or not.
  /// \note leave pwd unchanged
  TDirectory &root_safe_mkdir(TDirectory &parent,
                              const std::string &dirName);


  /// return ROOT TDirectory *, create subfolders as needed
  TDirectory *deliverROOTDir(TDirectory *const parent,
                             const std::string &childPath);

  /// reset histogram limits to remove outliers using TH1::SetAxisRange()
  /// \note algorithm works by iteratively clipping @ mean +/- 3*RMS
  template <class HistType>
  void removeOutliers(HistType &h) {
    static const unsigned char N_ITER = 3;
    
    // trim outliers
    float av = h.GetMean(); 
    float err = h.GetRMS();
    for ( unsigned short iter = 0; iter < N_ITER; iter++ ) {
      h.SetAxisRange(av-3*err, av+3*err);
      av = h.GetMean(); err = h.GetRMS();
    }
  }


};

#endif
