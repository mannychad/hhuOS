#include "ApplicationMainThread.h"

Kernel::ApplicationMainThread::ApplicationMainThread(int (*main)(int, char **), int argc, char **argv) : main(main), argc(argc), argv(argv) {

}

void Kernel::ApplicationMainThread::run() {
    main(argc, argv);
}