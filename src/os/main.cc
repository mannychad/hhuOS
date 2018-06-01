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

/**
 * Main function - is called from assembler code. 
 * 
 * @author Michael Schoettner, Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski
 * @date HHU, 2018
 */

#include <lib/file/tar/Archive.h>

#include <kernel/threads/IdleThread.h>

#include "apps/Application.h"
#include "devices/block/storage/AhciDevice.h"
#include <kernel/services/ModuleLoader.h>
#include <devices/graphics/lfb/VesaGraphics.h>
#include <devices/graphics/lfb/CgaGraphics.h>
#include <kernel/services/InputService.h>
#include <kernel/services/StdStreamService.h>
#include <kernel/services/SoundService.h>
#include <devices/graphics/text/VesaText.h>
#include <devices/Pit.h>
#include <kernel/threads/Scheduler.h>
#include <lib/multiboot/Structure.h>
#include <kernel/Logger.h>

#include "bootlogo.h"

extern char *gitversion;

auto versionString = String("hhuOS ") + String(gitversion);

uint16_t xres = 800;
uint16_t yres = 600;
uint8_t bpp = 32;

IdleThread *idleThread = nullptr;
EventBus *eventBus = nullptr;
LinearFrameBuffer *lfb = nullptr;
TextDriver *text = nullptr;

void updateBootScreen(uint8_t percentage, const char *currentActivity) {
    auto normalizedPercentage = static_cast<uint8_t>((percentage * 60) / 100);

    lfb->fillRect(0, 0, lfb->getResX(), lfb->getResY(), Colors::HHU_DARK_BLUE);

    lfb->placeString(sun_font_8x16, 50, 10, static_cast<char *>(versionString), Colors::HHU_GRAY, Colors::INVISIBLE);

    if(lfb->getResY() < 350) {
        lfb->placeSprite(50, 45, static_cast<uint16_t>(bootlogo_75x75.width),
                         static_cast<uint16_t>(bootlogo_75x75.height), (int32_t *) (&bootlogo_75x75.pixel_data[0]));
    } else {
        lfb->placeSprite(50, 45, static_cast<uint16_t>(bootlogo_200x200.width),
                         static_cast<uint16_t>(bootlogo_200x200.height), (int32_t *) (&bootlogo_200x200.pixel_data[0]));
    }

    lfb->placeFilledRect(20, 80, 60, 2, Colors::HHU_BLUE_30);
    lfb->placeFilledCircle(20, 81, 1, Colors::HHU_BLUE_30);
    lfb->placeFilledCircle(80, 81, 1, Colors::HHU_BLUE_30);

    lfb->placeFilledRect(20, 80, normalizedPercentage, 2, Colors::HHU_BLUE);
    lfb->placeFilledCircle(20, 81, 1, Colors::HHU_BLUE);
    lfb->placeFilledCircle(static_cast<uint16_t>(20 + normalizedPercentage), 81, 1, Colors::HHU_BLUE);

    lfb->placeString(sun_font_8x16, 50, 90, currentActivity, Colors::HHU_GRAY, Colors::INVISIBLE);

    lfb->show();
}

void registerServices() {
    auto *graphicsService = new GraphicsService();
    graphicsService->setLinearFrameBuffer(lfb);
    graphicsService->setTextDriver(text);

    Kernel::registerService(GraphicsService::SERVICE_NAME, graphicsService);

    Kernel::registerService(EventBus::SERVICE_NAME, eventBus);

    Kernel::registerService(TimeService::SERVICE_NAME, new TimeService());
    Kernel::registerService(StorageService::SERVICE_NAME, new StorageService());
    Kernel::registerService(FileSystem::SERVICE_NAME, new FileSystem());
    Kernel::registerService(InputService::SERVICE_NAME, new InputService());
    Kernel::registerService(DebugService::SERVICE_NAME, new DebugService());
    Kernel::registerService(ModuleLoader::SERVICE_NAME, new ModuleLoader());
    Kernel::registerService(StdStreamService::SERVICE_NAME, new StdStreamService());
    Kernel::registerService(SoundService::SERVICE_NAME, new SoundService());

    Kernel::getService<StdStreamService>()->setStdout(text);
    Kernel::getService<StdStreamService>()->setStderr(text);
}

void initGraphics() {
    auto *vesa = new VesaGraphics();

    // Get desired resolution from GRUB
    Util::Array<String> res = Multiboot::Structure::getKernelOption("vbe").split("x");

    if(res.length() >= 3) {
        xres = static_cast<uint16_t>(strtoint((const char *) res[0]));
        yres = static_cast<uint16_t>(strtoint((const char *) res[1]));
        bpp = static_cast<uint8_t>(strtoint((const char *) res[2]));
    }

	// Detect video capability
	if(vesa->isAvailable()) {
		lfb = vesa;
        text = new VesaText();
	} else {
		delete vesa;
		auto *cga = new CgaGraphics();
		if(cga->isAvailable()) {
			lfb = cga;
            text = new CgaText();
		} else {
			//No VBE and no CGA? Your machine is waaaaay to old...
			delete cga;
			Cpu::halt();
		}
	}

    // Initialize drivers
    lfb->init(xres, yres, bpp);
    text->init(static_cast<uint16_t>(xres / 8), static_cast<uint16_t>(yres / 16), bpp);

    stdout = text;
    text->setpos(0, 0);
}

int32_t main() {
    Cpu::disableInterrupts();

    Logger::trace("Start Kernel Main");

    Pic::getInstance()->forbid(Pic::Interrupt::PIT);
    Pic::getInstance()->forbid(Pic::Interrupt::KEYBOARD);
    Pic::getInstance()->forbid(Pic::Interrupt::CASCADE);
    Pic::getInstance()->forbid(Pic::Interrupt::COM2);
    Pic::getInstance()->forbid(Pic::Interrupt::COM1);
    Pic::getInstance()->forbid(Pic::Interrupt::LPT2);
    Pic::getInstance()->forbid(Pic::Interrupt::FLOPPY);
    Pic::getInstance()->forbid(Pic::Interrupt::LPT1);
    Pic::getInstance()->forbid(Pic::Interrupt::RTC);
    Pic::getInstance()->forbid(Pic::Interrupt::FREE1);
    Pic::getInstance()->forbid(Pic::Interrupt::FREE2);
    Pic::getInstance()->forbid(Pic::Interrupt::FREE3);
    Pic::getInstance()->forbid(Pic::Interrupt::MOUSE);
    Pic::getInstance()->forbid(Pic::Interrupt::FPU);
    Pic::getInstance()->forbid(Pic::Interrupt::PRIMARY_ATA);
    Pic::getInstance()->forbid(Pic::Interrupt::SECONDARY_ATA);

    initGraphics();
    eventBus = new EventBus();
    registerServices();

    Pit::getInstance()->plugin();

    auto *rtc = Kernel::getService<TimeService>()->getRTC();
    rtc->plugin();

    Pic::getInstance()->allow(Pic::Interrupt::CASCADE);

    auto *inputService = Kernel::getService<InputService>();
    inputService->getKeyboard()->plugin();
    inputService->getMouse()->plugin();

    Cpu::enableInterrupts();

    if(Multiboot::Structure::getKernelOption("debug") == "true") {
        text->puts("Initializing PCI Devices\n", 25, Colors::HHU_RED);
        Pci::scan();

        text->puts("Initializing Filesystem\n", 24, Colors::HHU_RED);
        auto *fs = Kernel::getService<FileSystem>();
        fs->init();
        printfUpdateStdout();

        text->puts("Starting Threads\n", 17, Colors::HHU_RED);
        idleThread = new IdleThread();

        idleThread->start();
        eventBus->start();
        Application::getInstance()->start();

        text->puts("\n\nFinished Booting! Please press Enter!\n", 40, Colors::HHU_BLUE);

        while (!inputService->getKeyboard()->isKeyPressed(28));

        lfb->init(xres, yres, bpp);
    } else {
        lfb->init(xres, yres, bpp);
        lfb->enableDoubleBuffering();

        updateBootScreen(0, "Initializing PCI Devices");
        Pci::scan();

        updateBootScreen(33, "Initializing Filesystem");
        auto *fs = Kernel::getService<FileSystem>();
        fs->init();
        printfUpdateStdout();

        updateBootScreen(66, "Starting Threads");
        idleThread = new IdleThread();

        idleThread->start();
        eventBus->start();
        Application::getInstance()->start();

        updateBootScreen(100, "Finished Booting!");
        Kernel::getService<TimeService>()->msleep(1000);

        lfb->disableDoubleBuffering();
        lfb->clear();
    }

    Kernel::getService<DebugService>()->printPic();
    while(true);

    Scheduler::getInstance()->schedule();

    return 0;
}
