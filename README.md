# advanDS DLDI Driver

A DLDI driver for the advanDS Nintendo DS emulator.

## Building

This driver requires the [devkitARM toolchain](https://devkitpro.org/wiki/Getting_Started) to build.

Simply run `make` to build the `advanDS.dldi` driver.

The constexpr flag `kAllowUnalignedTransfers` in driver.cpp specifies whether to support slow unaligned transfers. It is turned off by default, but this results in a non-compliant driver. Enabling the option increases the driver binary size and slightly reduces transfer performance.

## Emulator interface

The driver uses three MMIO ports to communicate with the emulator. All registers are visible on both ARM9 and ARM7 and only accept 32-bit accesses.

### 4EAD000h - ADVANDS_DLDI_XFERCTL - advanDS DLDI Transfer Control

Configures a transfer or queries the status of the last/current transfer.

#### Read

- Bits 0-8: Interface Version (currently 0x01)
- Bits 9-15: Reserved (should be zero)
- Bits 16-24: Transfer Offset (from the start of the sector specified in ADVANDS_DLDI_SECTOR, in bytes)
- Bits 25-28: Reserved (should be zero)
- Bit 29: Media Present (0=No, 1=Yes)
- Bit 30: Transfer Direction (0=Read (default), 1=Write)
- Bit 31: Transfer Error (0=OK (default), 1=Error)

During initialization the driver checks for a minimum Interface Version to be implemented by the emulator. If the emulator reports a lower version than the driver expects, `startup()` and all other functions except `shutdown()` will fail.

The driver uses the Transfer Error bit to detect transfer failures, returning `false` in such cases from `readSectors` and `writeSectors`.

The Media Present bit indicates if the SD card is inserted. The driver reads this bit to respond in `isInserted`.

The Transfer Offset tells how many words have been transferred from the current sector. It is not used by the driver, but may be useful for debuggers.

#### Write

- Bit 30: Transfer Direction (0=Read, 1=Write)
- Bit 31: Clear Transfer Error (when set to 1)

Writing a 1 to Clear Transfer Error acknowledges (clears) the Transfer Error bit. The driver writes a 1 to this before every transfer.

The Transfer Direction is set by the driver at the beginning of a `readSectors` or `writeSectors` call along with the transfer error acknowledgement.

### 4EAD004h - ADVANDS_DLDI_SECTOR - advanDS DLDI Sector

Specifies the 512 byte sector to read or write. Autoincremented once a sector is fully read/written.

Writing to this registers resets the Transfer Offset used with ADVANDS_DLDI_DATA.

### 4EAD008h - ADVANDS_DLDI_DATA - advanDS DLDI Data Transfer

Transfer data from/to the sector 4 bytes at a time. Must match the Transfer Direction in XFERCTL -- reads are ignored when direction is set to Write and vice-versa. After 128 successful transfers (for a total of 512 bytes), ADVANDS_DLDI_SECTOR is incremented by one.

If any error occurs, the Transfer Error bit in ADVANDS_DLDI_XFERCTL is set. Failed/ignored reads return FFFFFFFF. Failed transfers do not increment the Transfer Offset. Transfers will not occur if the Transfer Error bit is set; the driver must acknowledge the error before resuming transfers.

Writes may be silently ignored depending on emulator configuration.
