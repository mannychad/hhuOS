#ifndef __StorageDevice_include__
#define __StorageDevice_include__

#include "lib/String.h"
#include "lib/util/ArrayList.h"

#include <cstdint>
#include <kernel/lock/Spinlock.h>

/**
 * Interface for storage devices, that are accessed sector-wise.
 */
class StorageDevice {

public:
    /**
     * Possible partition types.
     */
    enum PARTITION_TYPE {
        /**
         * Standard primary partition.
         */
        PRIMARY = 0x01,
        /**
         * Extended partition.
         * An extended partition is a primary partition, that contains a linked list of logical partitions.
         */
        EXTENDED = 0x02,
        /**
         * Logical partition.
         * A logical partition is located inside an extended partition.
         */
        LOGICAL = 0x03
    };

    /**
     * Basic information about a partition.
     *
     * @var type The partition type (PRIMARY, EXTENDED, or LOGICAL)
     * @var active True, if the partition is bootable
     * @var systemId The system id
     * @var startSector The number of the partition's first sector, relative to the device's beginning
     * @var sectorCount The amount of sectors, that the partition consists of
     */
    struct PartitionInfo {
        uint8_t type;
        bool active;
        uint8_t systemId;
        uint32_t startSector;
        uint32_t sectorCount;

        bool operator==(const PartitionInfo &other) const {
            return type == other.type && active == other.active &&
                    systemId == other.systemId && startSector == other.startSector &&
                    sectorCount == other.sectorCount;
        }

        bool operator!=(const PartitionInfo &other) const {
            return type != other.type || active != other.active ||
                   systemId != other.systemId || startSector != other.startSector ||
                   sectorCount != other.sectorCount;
        }
    };

protected:
    Util::ArrayList<StorageDevice::PartitionInfo> partitionList;

private:
    /**
     * Representation of an entry in a partition table.
     * See https://wiki.osdev.org/Partition_Table for further reference.
     *
     * @var active_flag 0x80 --> bootable partition, 0x0 --> non-bootable partition
     * @var start_head Starting head of the partition
     * @var start_sector_cylinder Bits 0-6: Starting sector of the partition, Bits 7-15: Starting cylinder of the partition
     * @var system_id Partition type identifier
     * @var end_head Ending head of the partition
     * @var end_sector_cylinder Bits 0-6: Ending sector of the partition, Bits 7-15: Ending cylinder of the partition
     * @var relative_sector Relative sector to start of partition
     * @var sector_count Amount of sectors in partition
     */
    struct PartitionTableEntry {
        uint8_t  active_flag;
        uint8_t  start_head;
        uint16_t start_sector_cylinder;
        uint8_t  system_id;
        uint8_t  end_head;
        uint16_t end_sector_cylinder;
        uint32_t relative_sector;
        uint32_t sector_count;
    } __attribute__((packed));

    String name;

    Spinlock partLock;

    static const constexpr uint32_t PARTITON_TABLE_START = 0x1be;

public:
/**
 * Possible return codes for functions of this class.
 */
enum RETURN_CODES {
    SUCCESS = 0x00,
    READ_SECTOR_FAILED = 0x01,
    WRITE_SECTOR_FAILED = 0x02,
    INVALID_MBR_SIGNATURE = 0x03,
    EXTENDED_PARTITION_NOT_FOUND = 0x04,
    UNUSED_PARTITION = 0x05,
    NON_EXISTENT_PARITION = 0x06,
    DEVICE_NOT_PARTITIONABLE = 0x07,
};

/**
 * Possible system ids that a partition can have.
 * Based on: https://www.win.tue.nl/~aeb/partitions/partition_types-1.html
 */
enum SYSTEM_ID {
    EMPTY = 0x00,
    FAT12 = 0x01,
    XENIX_ROOT = 0x02,
    XENIX_USR = 0x03,
    FAT16 = 0x04,
    EXTENDED_PARTITION = 0x05,
    FAT16B = 0x06,
    IFS = 0x07,
    AIX_BOOT = 0x08,
    AIX_DATA = 0x09,
    OS2_BOOT = 0x0a,
    FAT32 = 0x0b,
    FAT32_LBA = 0x0c,
    SILICON_SAFE = 0x0d,
    FAT16B_LBA = 0x0e,
    EXTENDED_PARTITION_LBA = 0x0f,
    OPUS = 0x10,
    FAT12_HIDDEN = 0x11,
    COMPAQ_DIAGNOSTICS = 0x12,
    FAT16_HIDDEN = 0x14,
    FAT16B_HIDDEN = 0x16,
    IFS_HIDDEN = 0x17,
    AST_WINDOWS_SWAP = 0x18,
    FAT32_HIDDEN = 0x1b,
    FAT32_LBA_HIDDEN = 0x1c,
    FAT16_LBA_HIDDEN = 0x1e,
    OXYGEN_FS = 0x21,
    OXYGEN_EXTENDED_PARTITION = 0x22,
    RESERVED_01 = 0x23,
    NEC_DOS_3 = 0x24,
    RESERVED_02 = 0x26,
    WINDOWS_RESCUE_HIDDEN = 0x27,
    ATHEOS_FS = 0x2a,
    SYLLABLE_SECURE = 0x2b,
    RESERVED_03 = 0x31,
    NOS = 0x32,
    RESERVED_04 = 0x33,
    RESERVED_05 = 0x34,
    JFS = 0x35,
    RESERVED_06 = 0x36,
    THEOS_3_2_2GB = 0x38,
    THEOS_4_0_SPANNED = 0x39,
    THEOS_4_0_4GB = 0x3a,
    THEOS_4_0_EXTENDED = 0x3b,
    PARTITION_MAGIC = 0x3c,
    NETWARE_HIDDEN = 0x3d,
    VENIX_80286 = 0x40,
    LINUX__MINIX__DRDOS = 0x41,
    LINUX_SWAP__SECURE_FS = 0x42,
    LINUX_OLD = 0x43,
    GO_BACK = 0x44,
    PRIUM__EUMEL__ELAN = 0x45,
    EUMEL__ELAN_01 = 0x46,
    EUMEL__ELAN_02 = 0x47,
    EUMEL__ELAN_03 = 0x48,
    ALFS = 0x4a,
    AOS = 0x4c,
    QNX_PRIMARY = 0x4d,
    QNX_SECONDARY = 0x4e,
    QNX_TERTIARY = 0x4f,
    ONTRACK_READ_ONLY = 0x50,
    ONTRACK_READ_WRITE = 0x51,
    CPM_80 = 0x52,
    ONTRACK_AUXILIARY = 0x53,
    ONTRACK_DDO = 0x54,
    EZ_DRIVE = 0x55,
    GOLDEN_BOW_VFEATURE = 0x56,
    DRIVE_PRO = 0x57,
    HHU_OS_ROOT_FAT = 0x58,
    HHU_OS_RESERVED_01 = 0x59,
    HHU_OS_RESERVED_02 = 0x5a,
    HHU_OS_RESERVED_03 = 0x5b,
    PRIAM_EDISK = 0x5c,
    SPEEDSTOR = 0x61,
    UNIX_SYSTEM_V__MACH__GNU_HURD = 0x63,
    NOVELL_NETWARE_286 = 0x64,
    NOVELL_NETWARE_386 = 0x65,
    NOVELL_NETWARE_SMS = 0x66,
    NOVELL_NETWARE__WOLF_MOUNTAIN = 0x67,
    NOVELL_NETWARE = 0x68,
    NOVELL_NETWARE_5 = 0x69,
    DISK_SECURE_MULTI_BOOT = 0x70,
    RESERVED_07 = 0x71,
    UNIX_V7_X86 = 0x72,
    RESERVED_08 = 0x73,
    SCRAMDISK = 0x74,
    IBM_PC_IX = 0x75,
    RESERVED_09 = 0x76,
    M2FS_M2CS = 0x77,
    XOSL_BOOT = 0x78,
    F_I_X = 0x7e,
    ALTERNATIVE_OS_DEVELOPMENT = 0x7f,
    MINIX_1_1 = 0x80,
    MINIX_1_4b = 0x81,
    LINUX_SWAP = 0x82,
    LINUX = 0x83,
    OS2_FAT16_HIDDEN = 0x84,
    LINUX_EXTENDED = 0x85,
    FAT16_RAID = 0x86,
    HPFS_RAID__NTFS_RAID = 0x87,
    LINUX_PARTITION_TABLE = 0x88,
    LINUX_KERNEL = 0x8a,
    FAT32_RAID = 0x8b,
    FAT32_RAID_LBA = 0x8c,
    FREEDOS_FAT12_HIDDEN = 0x8d,
    LINUX_LVM = 0x8e,
    FREEDOS_FAT16_HIDDEN = 0x90,
    FREEDOS_EXTENDED_HIDDEN = 0x91,
    FREEDOS_FAT16B_HIDDEN = 0x92,
    AMOEBA = 0x93,
    AMOEBA_BAD_BLOCK_TABLE = 0x94,
    EXOPC = 0x95,
    ISO_9660 = 0x96,
    FREEDOS_FAT32_HIDDEN = 0x97,
    FREEDOS_FAT32_HIDDEN_LBA = 0x98,
    DCE376_LOGICAL = 0x99,
    FREEDOS_FAT16_HIDDEN_LBA = 0x9a,
    FREEODS_EXTENDED_HIDDEN_LBA = 0x9b,
    FORTH_OS = 0x9e,
    BSD_OS = 0x9f,
    LAPTOP_HIBERNATION_01 = 0xa0,
    LAPTOP_HIBERNATION_02 = 0xa1,
    HP_VOLUME_EXPANSION_01 = 0xa3,
    HP_VOLUME_EXPANSION_02 = 0xa4,
    FREE_BSD = 0xa5,
    OPEN_BSD = 0xa6,
    NEXT_STEP = 0xa7,
    MAC_OS_X = 0xa8,
    NET_BSD = 0xa9,
    OLIVETTI_DOS_FAT12 = 0xaa,
    MAC_OS_X_BOOT = 0xab,
    RISC_OS_ADFS = 0xad,
    SHAG_OS_FS = 0xae,
    SHAG_OS_SWAP__APPLE_HFS = 0xaf,
    BOOT_STAR_DUMMY = 0xb0,
    QNX_POWER_SAFE_FS_01 = 0xb1,
    QNX_POWER_SAFE_FS_02 = 0xb2,
    QNX_POWER_SAFE_FS_03 = 0xb3,
    HP_VOLUME_EXPANSION_03 = 0xb4,
    HP_VOLUME_EXPANSION_04 = 0xb6,
    BSDI_FS = 0xb7,
    BSDI_SWAP = 0xb8,
    PTS_BOOT_WIZARD = 0xbb,
    ACRONIS_BACKUP = 0xbc,
    BONNY_DOS = 0xbd,
    SOLARIS_BOOT = 0xbe,
    SOLARIS_X86 = 0xbf,
    CTOS__DR_DOS__NOVELL_DOS = 0xc0,
    DR_DOS_SECURED_FAT12 = 0xc1,
    LINUX_HIDDEN = 0xc2,
    LINUX_SWAP_HIDDEN = 0xc3,
    DR_DOS_SECURED_FAT16 = 0xc4,
    DR_DOS_SECURED_EXTENDED_PARTITION = 0xc5,
    DR_DOS_SECURED_FAT16B = 0xc6,
    SYRINX_BOOT = 0xc7,
    RESERVED_10 = 0xc8,
    RESERVED_11 = 0xc9,
    RESERVED_12 = 0xca,
    DR_DOS_SECURED_FAT32_CHS = 0xcb,
    DR_DOS_SECURED_FAT32_LBA = 0xcc,
    CTOS_MEMORY_DUMP = 0xcd,
    DR_DOS_SECURED_FAT16B_LBA = 0xce,
    DR_DOS_SECURED_EXTENDED_PARTITION_LBA = 0xcf,
    MULTI_USER_DOS_SECURED = 0xd0,
    MULTI_USER_DOS_SECURED_FAT12 = 0xd1,
    MULTI_USER_DOS_SECURED_FAT16 = 0xd4,
    MULTI_USER_DOS_SECURED_EXTENDED = 0xd5,
    MULTI_USER_DOS_SECURED_FAT16B = 0xd6,
    CPM_86 = 0xd8,
    POWERCOPY_BACKUP = 0xda,
    CONCURRENT_CPM__CONCURRENT_DOS__CTOS = 0xdb,
    CTOS_MEMORY_DUMP_HIDDEN = 0xdd,
    DELL_DIAGNOSTICS_FAT16 = 0xde,
    BOOTIT_EMBRM = 0xdf,
    ST_AVFS = 0xe0,
    SPEEDSTOR_FAT12_EXTENDED = 0xe1,
    SPEEDSTOR_READ_ONLY = 0xe3,
    SPEEDSTOR_FAT16_EXTENDED = 0xe4,
    TANDY_DOS_LOGICAL_FAT = 0xe5,
    SPEEDSTOR_STORAGE_DIMENSIONS_01 = 0xe6,
    LINUX_LUKS = 0xe7,
    RUFUS_EXTRA = 0xea,
    BEOS_FS = 0xeb,
    SKY_FS = 0xec,
    GPT_PROTECTIVE_MBR = 0xee,
    EFI_SYSTEM_PARTITION = 0xef,
    LINUX_PA_RISC_BOOT = 0xf0,
    SPEEDSTOR_STORAGE_DIMENSIONS_02 = 0xf1,
    DOS_SECONDARY = 0xf2,
    SPEEDSTOR_STORAGE_DIMENSIONS_03  = 0xf3,
    SPEEDSTOR_LARGE_PARTITION = 0xf4,
    PROLOGUE = 0xf5,
    SPEEDSTOR_STORAGE_DIMENSIONS_04 = 0xf6,
    DDRDRIVE_SSFS = 0xf7,
    LINUX_PCACHE = 0xf9,
    BOCHS = 0xfa,
    VMWARE_FS = 0xfb,
    VMWARE_SWAP = 0xfc,
    LINUX_RAID = 0xfd,
    SPEEDSTOR_STORAGE_DIMENSIONS_05 = 0xfe,
    XENIX_BAD_BLOCK_TABLE = 0x0ff
};

    /**
     * Constructor.
     *
     * @param name The device's name (e.g. 'hdd0', 'usb1', ...)
     */
    explicit StorageDevice(const String &name);

    /**
     * Copy-constructor.
     */
    StorageDevice(StorageDevice &copy) = delete;

    /**
     * Destructor.
     */
    virtual ~StorageDevice() = default;

    /**
     * Get the device's name.
     */
    String getName();

    /**
     * Read the partition table.
     * Currently, only MBR partition tables are supported.
     *
     * @return A list, containing information about all partitions.
     */
    virtual Util::ArrayList<StorageDevice::PartitionInfo>& readPartitionTable();

    /**
     * Write a partition to the partition table.
     * Currently, only MBR partition tables are supported.
     *
     * @param partNumber The partition number (1-4 --> Primary, > 4 --> Logical)
     * @param active True, if the partition is bootable
     * @param systemId The system id
     * @param startSector The number of the partition's first sector, relative to the device's beginning
     * @param sectorCount The amount of sectors, that the partition consists of
     * @return Return code
     */
    virtual uint32_t writePartition(uint8_t partNumber, bool active, uint8_t systemId, uint32_t startSector, uint32_t sectorCount);

    /**
     * Delete a partition from the partition table.
     * Currently, only MBR partition tables are supported.
     *
     * @param partNumber The partition number (1-4 --> Primary, > 4 --> Logical)
     * @return Return code
     */
    virtual uint32_t deletePartition(uint8_t partNumber);

    /**
     * Create a new empty partition table.
     * Currently, only MBR partition tables are supported.
     *
     * @return Return code
     */
    virtual uint32_t createPartitionTable();

    /**
     * Get the system id. This is almost only relevant for partitions,
     * but other devices may also return a valid system id.
     */
    virtual uint8_t getSystemId();

    /**
     * Get the size of a single sector in bytes.
     */
    virtual uint32_t getSectorSize() = 0;

    /**
     * Get the amount of sectors, that the device consists of.
     */
    virtual uint64_t getSectorCount() = 0;

    /**
     * Read sectors from the device.
     *
     * @param buff The buffer, where the read data will be stored in
     * @param sector The sector number
     * @param count The amount of sectors, that will be read
     * @return True, on success
     */
    virtual bool read(uint8_t *buff, uint32_t sector, uint32_t count) = 0;

    /**
     * Write sectors to the device.
     *
     * @param buff Contains the data, that will be written to the device
     * @param sector The sector number
     * @param count The amount of sectors, that will be written
     * @return True, on success
     */
    virtual bool write(const uint8_t *buff, uint32_t sector, uint32_t count) = 0;
};

#endif