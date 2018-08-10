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

#include "ElfConstants.h"

using namespace ElfConstants;

bool FileHeader::isValid() {
    if (magic[0] != 0x7F ||
        magic[1] != 'E'  ||
        magic[2] != 'L'  ||
        magic[3] != 'F') {
        return false;
    }

    if (architecture != Architecture::BIT_32||
        byteOrder != ByteOrder::LITTLE_ENDIAN     ||
        machine != MachineType::X86) {
        return false;
    }

    return !(type != ElfType::RELOCATABLE && type != ElfType::EXECUTABLE);
}

bool FileHeader::hasProgramEntries() {
    return programHeaderEntries != 0;
}

uint32_t RelocationEntry::getSymbolIndex() {
    return (uint32_t) (info >> 8U);
}

RelocationType RelocationEntry::getType() {
    return RelocationType(info & 0xFFU);
}

SymbolBinding SymbolEntry::getSymbolBinding() {
    return SymbolBinding(info >> 4U);
}

SymbolType SymbolEntry::getSymbolType() {
    return SymbolType(info & 0x0FU);
}
