#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "Log.h"
#include "GameBoy.h"

int main(int argc, char* argv[])
{
    // Remap clog to file
    std::ofstream ofs("log.txt");
    std::clog.rdbuf(ofs.rdbuf());

    std::filesystem::path romPath;
    if (argc == 1) romPath = std::filesystem::current_path() / "tests" / "02-interrupts.gb";
    else romPath = argv[1];

    GameBoy gb = (romPath);
    if (!gb.IsValid()) return 1;

    while(gb.IsRunning())
    {
        gb.Update();
    }

    std::getchar(); // Prevent console from closing
}
