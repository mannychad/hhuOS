/*
 * Copyright (C) 2018 Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * Heinrich-Heine University
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __IntDispatcher_include__
#define __IntDispatcher_include__

#include "kernel/interrupts/InterruptHandler.h"
#include "kernel/threads/ThreadState.h"
#include "kernel/KernelService.h"

#include "lib/util/ArrayList.h"
#include "lib/util/HashMap.h"

#include <cstdint>
#include <lib/system/SystemCall.h>

typedef void (*debugFunction)();

/**
 * IntDispatcher - responsible for registering and dispatching interrupts to the
 * corresponding handlers.
 *
 * @author Michael Schoettner, Filip Krakowski, Fabian Ruhland, Burak Akguel, Christian Gesse
 * @date HHU, 2018
 */
class IntDispatcher : public KernelService {

public:
	// enum of important interrupt numbers
    enum {
        PAGEFAULT = 14,
        PIT = 32,
        KEYBOARD = 33,
        COM2 = 35,
        COM1 = 36,
        LPT2 = 37,
        FLOPPY = 38,
        LPT1 = 39,
        RTC = 40,
        FREE1 = 41,
        FREE2 = 42,
        FREE3 = 43,
        MOUSE = 44,
        FPU = 45,
        PRIMARY_ATA = 46,
        SECONDARY_ATA = 47,
    };
    
    // no constructor needed
    IntDispatcher();

    IntDispatcher(const IntDispatcher &other) = delete;
    /**
     * Register an interrupt handler to an interrupt number.
     *
     * @param slot Interrupt number for this handler
     * @param gate Pointer to the handler itself
     */
    void assign(uint8_t slot, InterruptHandler &gate);

    /**
     * Register a debug handler to an interrupt number.
     *
     * @param slot Interrupt number for this handler
     * @param gate Pointer to the handler itself
     */
    void assignDebug(uint8_t slot, void (*debugHandler)());
    
    /**
     * Get the interrupt handlers that are registered for a specific interrupt.
     *
     * @param slot Interrupt number
     * @return Pointer to a list of all registered handlers or <em>nullptr</em> if no handlers are registered
     */
    Util::List<InterruptHandler*>* report(uint8_t slot);

    /**
     * Get the debug handlers that are registered for a specific interrupt.
     *
     * @param slot Interrupt number
     * @return Pointer to a list of all registered handlers or <em>nullptr</em> if no handlers are registered
     */
    debugFunction reportDebug(uint8_t slot);

    /**
     * Dispatched the interrupt to all registered interrupt handlers.
     *
     * @param frame The interrupt frame
     */
    void dispatch(InterruptFrame *frame);

    static IntDispatcher &getInstance() noexcept;

private:

    SystemCall systemCall;

    Util::HashMap<uint8_t, debugFunction> debugHandlers;

    Util::HashMap<uint8_t, Util::ArrayList<InterruptHandler*>*> handler;

    void sendEoi(uint32_t slot);
};

#endif
