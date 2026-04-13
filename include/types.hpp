#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct FieldSpec
{
    std::string name;
    int         min_width  = 0;
    bool        expandable = false;
};

struct InstructionGroup
{
    std::vector<std::string> insns;
    std::vector<std::string> operands;
    std::string              format;
    std::string              comment;

    int  format_id         = -1;
    int  local_opcode_bits = 0;
    bool uses_opcode       = false;
};

struct InputSpec
{
    int                           length = 0;
    std::vector<FieldSpec>        fields;
    std::vector<InstructionGroup> instructions;

    std::unordered_map<std::string, FieldSpec> field_map;
};

struct Interval
{
    int msb = -1;
    int lsb = -1;

    int width() const
    {
        return (msb >= lsb && lsb >= 0) ? (msb - lsb + 1) : 0;
    }

    bool overlaps(const Interval& other) const
    {
        return !(msb < other.lsb || other.msb < lsb);
    }
};

struct FormatLayout
{
    std::string              format;
    std::vector<std::string> insns;
    std::vector<std::string> operands;
    int                      format_id   = -1;
    bool                     uses_opcode = false;
};

struct ReserveField
{
    std::string name;
    Interval    interval;
};

struct LayoutResult
{
    int instruction_length = 0;
    int format_bits        = 0;
    int opcode_bits        = 0;

    std::unordered_map<std::string, Interval>                  global_fields;
    std::vector<FormatLayout>                                  formats;
    std::unordered_map<std::string, std::vector<ReserveField>> reserves_by_format;
};