#ifndef CGCUtil_H
#define CGCUtil_H 1

#include <string>
#include <vector>

using namespace std;

namespace CGCUtil {

  /// Template function fills any STL type container with zero values
  template <class T> static void fill_zero(T &container) {
    fill(container.begin(), container.end(), 0);
  }

  void tokenize_str(const string& str,
                    vector<string>& tokens,
                    const string& delimiters = " ");

  string &path_remove_dir(string &path);

  string &path_remove_ext(string &path);

};

#endif // CGCUtil_H
