#ifndef HHUOS_BOOTCOMPONENT_H
#define HHUOS_BOOTCOMPONENT_H

#include <kernel/threads/Thread.h>
#include <lib/util/Array.h>

class BootComponent : public Thread {

public:

    BootComponent(const String &name, Util::Array<BootComponent*> dependencies, void (*function)());

    BootComponent(const BootComponent &copy) = delete;

    BootComponent& operator=(const BootComponent &other) = delete;

    ~BootComponent() override = default;

    void run() override;

    bool isWaiting();

    bool hasFinished();

private:

    bool waiting;

    bool finished;

    Util::Array<BootComponent*> dependencies;

    void (*function)();
};

#endif