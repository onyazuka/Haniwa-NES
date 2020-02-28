#include "nes.hpp"

NES::NES(const std::string &romFname, Logger* _logger)
    : rom{romFname, _logger},
      stController1{},
      stController2{},
      mapper{makeMapper(rom.header()->mapper(), rom, _logger)},
      ppuMemory{*mapper, _logger},
      ppu{ppuMemory, eventQueue, _logger},
      memory{*mapper, ppu, stController1, stController2},
      cpu{memory, ppu, eventQueue, _logger},
      logger{_logger}
{
    //ppu.setDrawDebugGrid(true);
}

// Both save and load guarantee, that CPU's event queue is empty
void NES::save(const std::string& fname) {
    waitUntilEventQueueIsEmpty();

    std::ofstream ofs;
    ofs.open(fname, std::ios_base::binary);
    if(!ofs) {
        if(logger) logger->log(LogLevel::Error, "Couldn't open " + fname + " to save game!");
        throw InvalidFileException{};
    }
    std::string savedData;
    Serialization::Serializer::serializeAll(savedData, &ppu, &cpu);
    ofs.write(savedData.data(), savedData.size());
}

void NES::load(const std::string& fname) {
    waitUntilEventQueueIsEmpty();

    std::ifstream ifs;
    ifs.open(fname, std::ios_base::binary);
    if(!ifs) {
        if(logger) logger->log(LogLevel::Error, "Couldn't open " + fname + " to load game!");
        throw InvalidFileException{};
    }

    std::string data;
    ifs.seekg(0, std::ios_base::end);
    std::size_t fsize = ifs.tellg();
    ifs.seekg(0, std::ios_base::beg);
    data.resize(fsize);
    ifs.read(&(data[0]), fsize);
    Serialization::Deserializer::deserializeAll(data, 0, &ppu, &cpu);
}

void NES::waitUntilEventQueueIsEmpty() {
    while(!cpu.eventQueueEmpty()) cpu.exec();
}
