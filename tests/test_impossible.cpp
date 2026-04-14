#include <iostream>
#include <stdexcept>

#include "parser.hpp"
#include "validator.hpp"
#include "allocator.hpp"

int main()
{
    try
    {
        const std::string input_path = "examples/impossible_case.json";

        InputSpec    spec   = parse_input_file(input_path);
        validate_input_spec(spec);
        LayoutResult layout = allocate_layout(spec);

        std::cerr << "test_impossible failed: expected an exception, but layout was built\n";
        (void)layout;
        return 1;
    }
    catch (const std::exception&)
    {
        std::cout << "test_impossible passed\n";
        return 0;
    }
}