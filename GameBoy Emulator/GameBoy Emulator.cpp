#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "Log.h"
#include "Memory.h"
#include "CPU.h"
#include "Cartridge.h"

int main(int argc, char* argv[])
{
    // Remap clog to file
    std::ofstream ofs("log.txt");
    std::clog.rdbuf(ofs.rdbuf());

    std::filesystem::path romPath;
    if (argc == 1) romPath = std::filesystem::current_path() / "01-special.gb";
    else romPath = argv[1];

    if (romPath.empty())
    {
        Log::LogError("File not found!");
        return 1;
    }

    std::shared_ptr<Cartridge> cartridge = std::make_shared<Cartridge>(romPath.string().c_str());

    if (!cartridge->IsValid())
    {
        Log::LogError("Error loading ROM file!");
        return 1;
    }

    std::shared_ptr<Memory> memory = std::make_shared<Memory>(cartridge);
    CPU cpu(memory);

    Log::LogInfo("Emulator started succesfully!");

    bool quit = false;

    while(!quit)
    {
        quit = cpu.Cycle();
    }

    std::getchar(); // Prevent console from closing
}

