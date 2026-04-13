#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "types.hpp"

nlohmann::json generate_output_json(const InputSpec&   spec, const LayoutResult&   layout);
void           write_output_json   (const std::string& path, const nlohmann::json& j);