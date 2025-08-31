#ifndef CACCELDRIVE_HPP
#define CACCELDRIVE_HPP

// Requires <map>, <stdint.h>

//  This class takes care of the low-level configuration of addresses.
// The class stores internally the address of the device registers in the application virtual space,
// and a map of DMA-compatible memory allocations that relates virtual with physical addresses.

class CAccelDriver {
  protected:
    volatile void * accelRegs;
    uint64_t baseAddr, mappingSize;

  protected:  //Static 
    // Map of virtual addresses to physical addresses
    static std::map<uint64_t, uint64_t> dmaMappings;
    static uint32_t numModules; // Number of created modules.
    // Called by the destructor to free any dangling DMA allocations.
    static void InternalEmptyDMAAllocs();
    static bool logging;
    static int driver;

  public:
    typedef enum {OK = 0, DEVICE_ALREADY_INITIALIZED = 1, DEVICE_NOT_INITIALIZED = 2, ERROR_MAPPING_BASE_ADDR = 3,
                VIRT_ADDR_NOT_FOUND = 4, ERROR_OPENING_DRIVER = 5} TErrors;

  public:
    CAccelDriver(bool Logging = false);
    virtual ~CAccelDriver();

    // Maps the address of the peripheral registers in the physical address space into the application virtual address space.
    uint32_t Open(uint64_t BaseAddr, uint32_t MappingSize = 65536, volatile void ** AccelRegsPointer = NULL);
    uint32_t OpenDriver(const char * driver_name);
    void CloseDriver();

    // Static methods

    // Allocates a block of DMA-compatible memory and returns the corresponding address in this application virtual address space.
    // The class keeps an internal map of virtual to physical addresses, so that derived classes can translate the virtual 
    // addresses supplied by the applications.
    static void * AllocDMACompatible(uint32_t Size, uint32_t Cacheable = 0);
    static bool FreeDMACompatible(void * VirtAddr);
    // The application should never use the physical address. This is just for debugging purposes.
    static uint64_t GetDMAPhysicalAddr(void * VirtAddr);
    static void SetLogging(bool Logging = false) { logging = Logging; }
};


#endif  // CACCELDRIVE_HPP
