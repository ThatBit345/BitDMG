#include <memory>
#include <filesystem>

#include "Log.h"
#include "Memory.h"
#include "CPU.h"
#include "Cartridge.h"

int main(int argc, char* argv[])
{
    std::shared_ptr<Memory> memory = std::make_shared<Memory>();
    CPU cpu(memory);

    std::filesystem::path romPath = std::filesystem::current_path() / "01-special.gb";
    Cartridge cartridge(romPath.string().c_str());

    if(!cartridge.IsValid())
    {
        return 1;
    }

    if(!memory->LoadCartridge(cartridge))
    {
        Log::LogError("Error loading cartridge!");
        return 1;
    }

    Log::LogInfo("Emulator started succesfully!");

    bool quit = false;

    while(!quit)
    {
        quit = !cpu.Cycle();
    }
}

