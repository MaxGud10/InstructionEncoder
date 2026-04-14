#include <cassert>
#include <iostream>
#include <stdexcept>

#include "parser.hpp"
#include "validator.hpp"
#include "allocator.hpp"
#include "generator.hpp"

int main()
{
    try
    {
        const std::string input_path = "examples/input_example.json";

        InputSpec spec = parse_input_file(input_path);
        validate_input_spec(spec);

        LayoutResult layout   = allocate_layout(spec);
        nlohmann::json output = generate_output_json(spec, layout);

        assert(output.is_array());
        assert(output.size() == 23); // 12 + 10 + 1

        bool found_add         = false;
        bool found_sub         = false;
        bool found_branch_cond = false;

        for (const auto& item : output)
        {
            assert(item.contains("insn"));
            assert(item.contains("fields"));

            const std::string insn = item.at("insn").get<std::string>();
            const auto& fields     = item.at("fields");

            assert(fields.is_array());
            assert(!fields.empty());

            if (insn == "add")
                found_add = true;
            if (insn == "sub")
                found_sub = true;
            if (insn == "branch.cond")
                found_branch_cond = true;

            for (const auto& field_entry : fields)
            {
                assert(field_entry.is_object());
                assert(field_entry.size() == 1);

                      auto  it         = field_entry.begin();
                const auto& field_body = it.value();

                assert(field_body.contains("msb"));
                assert(field_body.contains("lsb"));
                assert(field_body.contains("value"));

                const int msb           = field_body.at("msb")  .get<int>();
                const int lsb           = field_body.at("lsb")  .get<int>();
                const std::string value = field_body.at("value").get<std::string>();

                assert(msb >= lsb);
                assert(lsb >= 0);
                assert(msb < spec.length);
                assert(!value.empty());
            }
        }

        assert(found_add);
        assert(found_sub);
        assert(found_branch_cond);

        std::cout << "test_integration passed\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "test_integration failed: " << ex.what() << "\n";
        return 1;
    }
}