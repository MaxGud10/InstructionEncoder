#include "parser.hpp"

#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace
{

int parse_int_like(const nlohmann::json& j)
{
    if (j.is_number_integer())
        return j.get<int>();
    if (j.is_string())
        return std::stoi(j.get<std::string>());

    throw std::runtime_error("Expected integer or stringified integer");
}

FieldSpec parse_field_object(const nlohmann::json& obj)
{
    if (!obj.is_object() || obj.size() != 1)
        throw std::runtime_error("Each field entry must be an object with exactly one key");

    auto it = obj.begin();

    FieldSpec spec;
    spec.name = it.key();

    std::string raw;
    if (it.value().is_string())
        raw = it.value().get<std::string>();
    else if (it.value().is_number_integer())
        raw = std::to_string(it.value().get<int>());
    else
        throw std::runtime_error("Field size must be a string or integer");

    if (raw.rfind(">=", 0) == 0)
    {
        spec.expandable = true;
        spec.min_width  = std::stoi(raw.substr(2));
    }
    else
    {
        spec.expandable = false;
        spec.min_width  = std::stoi(raw);
    }

    if (spec.min_width <= 0)
        throw std::runtime_error("Field width must be positive");

    return spec;
}

InputSpec parse_json_object(const nlohmann::json& j)
{
    InputSpec spec;
    spec.length = parse_int_like(j.at("length"));

    for (const auto& field_entry : j.at("fields"))
    {
        spec.fields.push_back(parse_field_object(field_entry));
    }

    for (const auto& group_j : j.at("instructions"))
    {
        InstructionGroup g;
        g.insns    = group_j.at   ("insns"   ).get<std::vector<std::string>>();
        g.operands = group_j.at   ("operands").get<std::vector<std::string>>();
        g.format   = group_j.at   ("format"  ).get<std::string>();
        g.comment  = group_j.value("comment", "");
        spec.instructions.push_back(std::move(g));
    }

    return spec;
}

} // namespace

InputSpec parse_input_stream(std::istream& in)
{
    if (!in)
        throw std::runtime_error("Input stream is not readable");

    nlohmann::json j;
    in >> j;
    return parse_json_object(j);
}

InputSpec parse_input_file(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("Cannot open input file: " + path);

    return parse_input_stream(in);
}