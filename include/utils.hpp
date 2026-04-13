#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "types.hpp"

int         ceil_log2_int   (int value);
std::string to_binary_string(int value, int width);
std::string to_upper        (std::string s);

std::vector<std::string> collect_all_operand_names(const InputSpec& spec);

std::unordered_map<std::string, std::vector<std::string>>
build_field_usage     (const InputSpec& spec);

std::unordered_map<std::string, std::vector<std::string>>
build_format_to_fields(const InputSpec& spec, int format_bits, int opcode_bits);

std::unordered_map<std::string, std::vector<std::string>>
build_conflict_graph  (const InputSpec& spec, int format_bits, int opcode_bits);