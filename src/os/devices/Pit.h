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

#ifndef __PIT_include__
#define __PIT_include__

#include "kernel/interrupts/InterruptHandler.h"
#include "kernel/IOport.h"
#include "kernel/Kernel.h"
#include "kernel/services/GraphicsService.h"
#include "kernel/services/TimeService.h"
#include "devices/graphics/text/TextDriver.h"

/**
 * Driver for the programmable interval timer.
 *
 * @author Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * @date HHU, 2017
 */
class Pit : public InterruptHandler {

public:
    /**
     * Copy-constructor.
     */
    Pit(const Pit &copy) = delete;

    /**
     * Copy-operator.
     */
    Pit &operator=(const Pit &copy) = delete;

    /**
     * Destructor.
     */
    ~Pit() override = default;

    /**
     * Returns an instance of the PIT.
     *
     * @return An instance of the PIT
     */
    static Pit *getInstance();

    /**
     * Returns the interval at which the PIT fires it's interrupts.
     *
     * @return The PIT's interval in microseconds
     */
    uint32_t getInterval();

    /**
     * Sets the interval at which the PIT fires it's interrupts.
     *
     * @param us The PIT's interval in microseconds
     */
    void setInterval(uint32_t us);

    /**
     * Enable interrupts for the PIT.
     */
    void plugin ();

    /**
     * Overriding function from InterruptHandler.
     */
    void trigger () override;

private:
    /**
     * Constructor.
     *
     * @param us The interval, with which the PIT shall trigger interrupts.
     */
    explicit Pit (uint32_t us);

    static Pit *instance;

    TimeService *timeService;

    GraphicsService *graphicsService;

    uint32_t timerInterval;

    static const uint32_t TIME_BASE = 838;

    static const uint32_t DEFAULT_INTERVAL = 10000;
};

#endif
