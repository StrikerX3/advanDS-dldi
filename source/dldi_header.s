#include <nds/arm9/dldi_asm.h>

@---------------------------------------------------------------------------------
@ Fixed-length ASCII string, padded with null characters
    .macro padded_string string, max
1:
    .ascii "\string"
2:
    .iflt \max - (2b - 1b)
    .error "String too long"
    .endif

    .ifgt \max - (2b - 1b)
    .zero \max - (2b - 1b)
    .endif

    .endm

@---------------------------------------------------------------------------------
	.section ".crt0","ax"
@---------------------------------------------------------------------------------
	.global _start
	.align	4
	.arm

@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
	.word	0xBF8DA5ED		@ Magic number to identify this region
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	DLDI_SIZE_1KB	@ Size
	.byte	0x00			@ Sections to fix
	.byte 	0x00			@ Space allocated in the application, not important here.
	
@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 48 bytes
	.align	4
	padded_string "advanDS DLDI Driver v1", 48
	
@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
	.word   __text_start	@ data start
	.word   __data_end		@ data end
	.word	0				@ Interworking glue start	-- Needs address fixing
	.word	0				@ Interworking glue end
	.word   0				@ GOT start					-- Needs address fixing
	.word   0				@ GOT end
	.word   0				@ bss start					-- Needs setting to zero
	.word   0				@ bss end

@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
	.ascii	"ADSD"			@ ioType
	.word	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_NDS
	.word	startup			@ 
	.word	isInserted		@ 
	.word	readSectors		@   Function pointers to standard device driver functions
	.word	writeSectors	@ 
	.word	clearStatus		@ 
	.word	shutdown		@ 
	
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	.align
	.pool
	.end
@---------------------------------------------------------------------------------
