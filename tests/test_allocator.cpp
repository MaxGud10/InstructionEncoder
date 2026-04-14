#include <cassert>
#include <iostream>
#include <stdexcept>

#include "parser.hpp"
#include "validator.hpp"
#include "allocator.hpp"

int main()
{
    try
    {
        const std::string input_path = "examples/input_example.json";

        InputSpec spec = parse_input_file(input_path);
        validate_input_spec(spec);

        LayoutResult layout = allocate_layout(spec);

        assert(layout.instruction_length == 25);
        assert(layout.format_bits        == 2);
        assert(layout.opcode_bits        == 4);

        assert(layout.global_fields.find("F")      != layout.global_fields.end());
        assert(layout.global_fields.find("OPCODE") != layout.global_fields.end());
        assert(layout.global_fields.find("R0")     != layout.global_fields.end());
        assert(layout.global_fields.find("R1")     != layout.global_fields.end());
        assert(layout.global_fields.find("R2")     != layout.global_fields.end());
        assert(layout.global_fields.find("imm")    != layout.global_fields.end());
        assert(layout.global_fields.find("disp")   != layout.global_fields.end());
        assert(layout.global_fields.find("code")   != layout.global_fields.end());

        const auto r0   = layout.global_fields.at("R0");
        const auto r1   = layout.global_fields.at("R1");
        const auto r2   = layout.global_fields.at("R2");
        const auto imm  = layout.global_fields.at("imm");
        const auto disp = layout.global_fields.at("disp");
        const auto code = layout.global_fields.at("code");

        assert(r0.width()   >= 5);
        assert(r1.width()   >= 5);
        assert(r2.width()   >= 5);
        assert(imm.width()  >= 8);
        assert(disp.width() >= 12);
        assert(code.width() >= 3);

        std::cout << "test_allocator passed\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "test_allocator failed: " << ex.what() << "\n";
        return 1;
    }
}