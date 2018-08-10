/*
 * Copyright (C) 2018  Filip Krakowski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lib/elf/ElfLoader.h>
#include "ElfModule.h"

uint32_t ElfModule::getSymbol(const String &name) {

    if (localSymbols.containsKey(name)) {

        return localSymbols.get(name);
    }

    return 0;
}

ElfModule::~ElfModule() {

    delete buffer;
}

bool ElfModule::isValid() {

    if ( !fileHeader->isValid() ) {

        return false;
    }

    return fileHeader->type == ElfType::RELOCATABLE;
}

void ElfModule::loadSectionNames() {

    SectionHeader *sectionHeader = nullptr;

    sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + fileHeader->sectionHeaderStringIndex * fileHeader->sectionHeaderEntrySize];

    sectionNames = &buffer[sectionHeader->offset];
}

void ElfModule::loadSections() {

    SectionHeader *sectionHeader = nullptr;

    char *sectionName = nullptr;

    for (elf32_word i = 0; i < fileHeader->sectionHeaderEntries; i++) {

        sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + i * fileHeader->sectionHeaderEntrySize];

        if (sectionHeader->type == SectionHeaderType::NONE) {

            continue;
        }

        sectionHeader->virtualAddress = base + sectionHeader->offset;

        sectionName = &sectionNames[sectionHeader->nameOffset];

        sections.put(sectionName, sectionHeader);

        localSymbols.put(sectionName, sectionHeader->virtualAddress);

        if ( strcmp(sectionName, ".symtab") == 0 ) {

            symbolTable = (SymbolEntry*) sectionHeader->virtualAddress;

            symbolTableSize = sectionHeader->size / sectionHeader->entrySize;

        } else if ( strcmp(sectionName, ".strtab") == 0 ) {

            stringTable = (char*) sectionHeader->virtualAddress;

            stringTableSize = sectionHeader->size;
        }
    }
}

void ElfModule::parseSymbolTable() {

    SectionHeader *sectionHeader = nullptr;

    SymbolEntry *symbolEntry = nullptr;

    SymbolBinding symbolBinding;

    char *symbolName = nullptr;

    for (uint32_t i = 0; i < symbolTableSize; i++) {

        symbolEntry = &symbolTable[i];

        if (symbolEntry->section == 0) {

            continue;
        }

        if (symbolEntry->nameOffset == 0) {

            continue;
        }

        symbolBinding = symbolEntry->getSymbolBinding();

        if (symbolBinding != SymbolBinding::GLOBAL && symbolBinding != SymbolBinding::WEAK && symbolBinding != SymbolBinding::LOCAL) {

            continue;
        }

        symbolName = &stringTable[symbolEntry->nameOffset];

        bool isPresent = localSymbols.containsKey(symbolName);

        if (isPresent && symbolBinding == SymbolBinding::WEAK) {

            continue;
        }

        sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + symbolEntry->section * fileHeader->sectionHeaderEntrySize];

        localSymbols.put(symbolName, (sectionHeader->virtualAddress + symbolEntry->value));
    }
}

void ElfModule::relocate() {

    SectionHeader *sectionHeader = nullptr;

    RelocationEntry *relocationTable = nullptr;

    RelocationEntry *relocationEntry = nullptr;

    SymbolEntry *symbol = nullptr;

    char *symbolName = nullptr;

    uint32_t relocationTableSize = 0;

    uint32_t addend = 0;

    uint32_t *location = nullptr;

    for (uint32_t i = 0; i < fileHeader->sectionHeaderEntries; i++) {

        sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + i * fileHeader->sectionHeaderEntrySize];

        if (sectionHeader->type != SectionHeaderType::REL) {

            continue;
        }

        relocationTable = (RelocationEntry*) sectionHeader->virtualAddress;

        relocationTableSize = sectionHeader->size / sectionHeader->entrySize;

        sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + sectionHeader->info * fileHeader->sectionHeaderEntrySize];

        for (uint32_t j = 0; j < relocationTableSize; j++) {

            relocationEntry = &relocationTable[j];

            symbol = &symbolTable[relocationEntry->getSymbolIndex()];

            if (relocationEntry->getType() == RelocationType::R_386_NONE) {

                continue;
            }

            if (symbol->getSymbolType() == SymbolType::FILE) {

                continue;
            }

            if (symbol->getSymbolType() == SymbolType::SECTION) {

                symbolName = getSectionName(symbol->section);

            } else {

                symbolName = &stringTable[symbol->nameOffset];
            }

            uint32_t address = getSymbol(symbolName);

            if (address == 0) {

                address = KernelSymbols::get(symbolName);
            }

            if (address == 0) {
                Cpu::throwException(Cpu::Exception::KEY_NOT_FOUND, (char*) String::format("The requested symbol %s is not defined", symbolName));
            }

            location = (uint32_t*) (sectionHeader->virtualAddress + relocationEntry->offset);

            addend = *location;

            switch (relocationEntry->getType()) {
                case RelocationType::R_386_32 :
                    *location = addend + address;
                    break;
                case RelocationType::R_386_PC32 :
                    *location = addend + address - (uint32_t) location;
                    break;
                default:
                    break;
            }
        }
    }
}

char *ElfModule::getSectionName(uint16_t sectionIndex) {

    SectionHeader *sectionHeader = (SectionHeader*) &buffer[fileHeader->sectionHeader + sectionIndex * fileHeader->sectionHeaderEntrySize];

    return &sectionNames[sectionHeader->nameOffset];
}

Module *ElfModule::getInstance() {

    if (instance == nullptr) {

        instance = provider();
    }

    return instance;
}
