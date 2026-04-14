#pragma once

#include <istream>
#include <string>
#include "types.hpp"

InputSpec parse_input_file  (const std::string& path);
InputSpec parse_input_stream(std::istream&      in);