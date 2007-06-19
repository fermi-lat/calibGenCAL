/** @file
    @author Zachary Fewtrell
    @brief implementation of ROOTUtil.h
 */



// STD INCLUDES
#include <string>
#include <queue>
#include <stdexcept>

// EXTLIB INCLUDES
#include "TDirectory.h"

// GLAST INCLUDES

// LOCAL INCLUDES
#include "string_util.h"
#include "stl_util.h"
#include "ROOTUtil.h"


using namespace std;

namespace calibGenCAL {
    TDirectory &root_safe_mkdir(TDirectory &parent,
                                const string &dirName) {
      TDirectory &cwd = *gDirectory;

      // only create new dir if it doesn't already exist
      if (!parent.cd(dirName.c_str())) {
        cwd.cd();
        return *(parent.mkdir(dirName.c_str()));
      }

      // else return existing directory
      cwd.cd();
      return *(parent.GetDirectory(dirName.c_str()));
    }

  /// mkdir op creates subfolders as needed
  static TDirectory *recursiveROOTMkDir(TDirectory &parent,
                                        const string &childPath) {
    /// split up subdirs by path delim
    vector<string> dirlist(tokenize_str(childPath, "/"));
      
    queue<string> dirq;
    copy(dirlist.begin(),
         dirlist.end(),
         push_insert_iterator<queue<string> >(dirq));

    // initial value
    string fullpath(parent.GetPath());
    
    // for each subdir, check if it exists & create if needed.
    TDirectory *child = 0;
    TDirectory *currentDir = &parent;
    while (!dirq.empty()) {
      string childName(dirq.front());
      dirq.pop();

        
      // check if childDir exists
      child = currentDir->GetDirectory(childName.c_str());
        
      // else create new child dir
      if (child == 0) {
        child = currentDir->mkdir(childName.c_str());
        if (child == 0)
          throw runtime_error("Unable to create ROOT dir: " + fullpath);
      }

      // prepare for next iteration of loop
      currentDir = child;
      fullpath += string("/") + childName;
    }
    
    return child;
  }

  TDirectory *deliverROOTDir(TDirectory *const parent,
                             const string &childPath) {
    const string fullpath(string(parent->GetPath()) + childPath);
      
    TDirectory *retVal;
    if ((retVal = parent->GetDirectory(fullpath.c_str())))
      return retVal;
    
    
    return recursiveROOTMkDir(*parent, childPath);
  }
};
