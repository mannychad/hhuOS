#ifndef __RandomNode_include__
#define __RandomNode_include__

#include "kernel/filesystem/RamFs/VirtualNode.h"
#include <cstdint>

class RandomNode : public VirtualNode {

public:
    /**
     * Constructor.
     */
    RandomNode();

    /**
     * Copy-constructor.
     */
    RandomNode(const RandomNode &copy) = delete;

    /**
     * Destructor.
     */
    ~RandomNode() override = default;

    /**
     * Overriding function from VirtualNode.
     */
    uint64_t getLength() override;

    /**
     * Overriding function from VirtualNode.
     */
    bool readData(char *buf, uint64_t pos, uint64_t numBytes) override;

    /**
     * Overriding function from VirtualNode.
     */
    bool writeData(char *buf, uint64_t pos, uint64_t numBytes) override;
};

#endif