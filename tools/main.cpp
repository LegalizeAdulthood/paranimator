#include <ParFile/ParFile.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

std::vector<std::string_view> arguments(int argc, char *argv[])
{
    std::vector<std::string_view> result;
    result.resize(argc);
    for (int i = 0; i < argc; ++i)
    {
        result.emplace_back(argv[i]);
    }
    return result;
}

int usage(std::string_view program)
{
    std::cerr << "Usage:\n" //
              << program << " <begin-par> <end-par> <num-frames>\n";
    return 1;
}

ParFile::ParFilePtr get_par(std::string_view file)
{
    std::ifstream in{std::string{file}};
    return ParFile::create(in);
}

ParFile::ParSet interpolate(const ParFile::ParFilePtr &begin, const ParFile::ParFilePtr &end, int frame, int num_frames)
{
    ParFile::ParSet par_set{*begin->cbegin()};
    par_set.name = "Frame_" + std::to_string(frame + 1);
    return par_set;
}

void print(const ParFile::ParSet &par)
{
    std::cout << par.name << " {\n";
    for (const ParFile::Parameter &param : par.params)
    {
        std::cout << "    " << param.name;
        if (!param.value.empty())
        {
            std::cout << param.value;
        }
        std::cout << '\n';
    }
    std::cout << "}\n";
}

int main(const std::vector<std::string_view> &args)
{
    if (args.size() != 4)
    {
        return usage(args[0]);
    }
    const std::string_view &begin{args[1]};
    if (!std::filesystem::exists(begin))
    {
        std::cerr << begin << " does not exist.\n";
        return 2;
    }
    const std::string_view &end{args[2]};
    if (!std::filesystem::exists(end))
    {
        std::cerr << end << " does not exist.\n";
        return 2;
    }
    const int num_frames{std::stoi(std::string{args[3]})};
    if (num_frames < 1)
    {
        std::cerr << num_frames << " must be positive.\n";
        return 3;
    }

    ParFile::ParFilePtr begin_par{get_par(begin)};
    ParFile::ParFilePtr end_par{get_par(end)};
    for (int i = 0; i < num_frames; ++i)
    {
        ParFile::ParSet frame{interpolate(begin_par, end_par, i, num_frames)};
        print(frame);
    }
    
    return 0;
}

}

int main(int argc, char *argv[])
{
    return main(arguments(argc, argv));    
}
