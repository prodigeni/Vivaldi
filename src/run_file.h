#ifndef VV_RUN_FILE_H
#define VV_RUN_FILE_H

#include "vm/call_frame.h"

#include <string>

namespace vv {

class run_file_result {
public:
  enum class result {
    success,
    failure,
    file_not_found
  } res;
  /// true on success, excepted result on exception or file_not_found
  value::base* val;
  /// frame containing everything defined in file
  std::unordered_map<symbol, value::base*> frame;
};

run_file_result run_file(const std::string& filename);

}

#endif
