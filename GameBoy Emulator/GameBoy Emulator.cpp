#include <memory>
#include <filesystem>

#include "Log.h"
#include "Memory.h"
#include "CPU.h"

int main(int argc, char* argv[])
{
    std::shared_ptr<Memory> memory = std::make_shared<Memory>();
    CPU cpu(memory);

    std::filesystem::path romPath = std::filesystem::current_path() / "01-special.gb";

    if(!memory->LoadRom(romPath.string().c_str()))
    {
        return 1;
    }

    Log::LogInfo("Emulator started succesfully!");

    while(true)
    {
        cpu.Cycle();
    }
}

