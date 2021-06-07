#include <nds/ndstypes.h>

#define ADVANDS_DLDI_XFERCTL (*(volatile u32 *)0x4EAD000)
#define ADVANDS_DLDI_SECTOR (*(volatile u32 *)0x4EAD004)
#define ADVANDS_DLDI_DATA (*(volatile u32 *)0x4EAD008)

constexpr bool kAllowUnalignedTransfers = false;

extern "C" {
    static bool ok __attribute__((used));
    static constexpr u8 expectedVersion = 0x01;

    bool doTransfer(u32 sector, u32 numSectors, void *buffer, bool write);

    bool startup() { return ok = ((ADVANDS_DLDI_XFERCTL & 0xFF) >= expectedVersion); }
    bool isInserted() { return ok && (ADVANDS_DLDI_XFERCTL & (1 << 29)); }
    bool clearStatus() { return ok; }
    bool readSectors(u32 sector, u32 numSectors, void *buffer) { return doTransfer(sector, numSectors, buffer, false); }
    bool writeSectors(u32 sector, u32 numSectors, void *buffer) { return doTransfer(sector, numSectors, buffer, true); }
    bool shutdown() { return true; }

    bool doTransfer(u32 sector, u32 numSectors, void *buffer, bool write) {
        if (!ok) return false;               // Don't attempt to transfer if not initialized
        if (buffer == nullptr) return false; // Don't write to nullptr
        if constexpr (!kAllowUnalignedTransfers) {
            if ((u32)buffer & 3) return false; // Require aligned buffer
        }
        [[maybe_unused]] const bool unaligned = (u32)buffer & 3;

        // Clear last transfer error and setup pointers
        ADVANDS_DLDI_XFERCTL = 0x80000000 | (write << 30);
        ADVANDS_DLDI_SECTOR = sector;
        u32 *buf32 = (u32 *)buffer;
        u8 *buf8 = (u8 *)buffer;
        for (u32 i = 0; i < numSectors; i++) {
            // Transfer 512 bytes, one word at a time
            for (u32 j = 0; j < 512 / sizeof(u32); j++, buf32++, buf8 += 4) {
                // Perform the transfer
                if (write) {
                    if constexpr (kAllowUnalignedTransfers) {
                        uint32_t value;
                        if (unaligned) {
                            value = (buf8[0] << 0) | (buf8[1] << 8) | (buf8[2] << 16) | (buf8[3] << 24);
                        } else {
                            value = *buf32;
                        }
                        ADVANDS_DLDI_DATA = value;
                    } else {
                        ADVANDS_DLDI_DATA = *buf32;
                    }
                } else {
                    if constexpr (kAllowUnalignedTransfers) {
                        uint32_t value = ADVANDS_DLDI_DATA;
                        if (unaligned) {
                            buf8[0] = (value >> 0);
                            buf8[1] = (value >> 8);
                            buf8[2] = (value >> 16);
                            buf8[3] = (value >> 24);
                        } else {
                            *buf32 = value;
                        }
                    } else {
                        *buf32 = ADVANDS_DLDI_DATA;
                    }
                }

                // Bail out on transfer error
                if (ADVANDS_DLDI_XFERCTL & 0x80000000) return false;
            }
        }
        return true;
    }
}
