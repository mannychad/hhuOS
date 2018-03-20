#ifndef __GraphicsVendorNameNode_include__
#define __GraphicsVendorNameNode_include__


#include <kernel/filesystem/RamFs/VirtualNode.h>
#include <kernel/services/GraphicsService.h>

class GraphicsVendorNameNode : public VirtualNode {

private:
    GraphicsService *graphicsService = nullptr;

    uint8_t mode;

public:

    /**
     * Possible graphics modes.
     */
    enum MODES {
        TEXT = 0x00,
        LINEAR_FRAME_BUFFER = 0x01
    };

    /**
     * Constructor.
     */
    explicit GraphicsVendorNameNode(uint8_t mode);

    /**
     * Copy-Constructor.
     */
    GraphicsVendorNameNode(const GraphicsVendorNameNode &copy) = delete;

    /**
     * Destructor.
     */
    ~GraphicsVendorNameNode() override = default;

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
