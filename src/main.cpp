#include <iostream>
#include <stdexcept>
#include <string>

#include "parser.hpp"
#include "validator.hpp"
#include "allocator.hpp"
#include "generator.hpp"

int main(int argc, char** argv)
{
    try
    {
        if (argc == 3)
        {
            const std::string input_path  = argv[1];
            const std::string output_path = argv[2];

            InputSpec spec = parse_input_file(input_path);
            validate_input_spec(spec);

            LayoutResult layout = allocate_layout(spec);
            auto output_json    = generate_output_json(spec, layout);
            write_output_json(output_path, output_json);

            std::cout << "Output written to: " << output_path << "\n";
            return 0;
        }

        if (argc == 1)
        {
            InputSpec spec = parse_input_stream(std::cin);
            validate_input_spec(spec);

            LayoutResult layout = allocate_layout(spec);
            auto output_json    = generate_output_json(spec, layout);

            std::cout << output_json.dump(2) << "\n";
            return 0;
        }

        std::cerr << "Usage:\n";
        std::cerr << "  instruction_encoder <input.json> <output.json>\n";
        std::cerr << "  cat <input.json> | instruction_encoder\n";
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
}