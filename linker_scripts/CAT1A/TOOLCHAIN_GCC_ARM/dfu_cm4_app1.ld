/***************************************************************************//**
* \file dfu_cm4_app1.ld
* \version 4.10
*
* The linker file for the GNU C compiler.
* Used for the DFU SDK application 1 firmware project. 
* Contains only CM4 sections (no section for the CM0p image that 
* starts CM4 application 0). Application 1 is uploaded through 
* Bootloader Host Tool using the generated cyadc2 file.
*
* \note
* This linker file is generic and handles common use cases. Your project may not 
* use each section defined in the linker file and then you may see warnings 
* during the build process. Simply comment out or remove the relevant code
* in the linker file.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)
ENTRY(Reset_Handler)

STACK_SIZE = 0x1000;

/* Force symbol to be entered in the output file as an undefined symbol. Doing
* this may, for example, trigger linking of additional modules from standard
* libraries. You may list several symbols for each EXTERN, and you may use
* EXTERN multiple times. This command has the same effect as the -u command-line
* option.
*/
EXTERN(Reset_Handler)

/* The MEMORY section below describes the location and size of blocks of memory in the target.
* Use this section to specify the memory regions available for allocation.
*/
MEMORY
{
    /* The ram and flash regions control RAM and flash memory allocation for the CM4 core.
     * Note that 2 KB of RAM (at the end of the SRAM) are reserved for system use.
     * Using this memory region for other purposes will lead to unexpected behavior.
     */
    ram_common        (rwx) : ORIGIN = 0x08002000, LENGTH = 0x0400

    /* Note: the ram_appX_core1 regions has to be 0x400 aligned
     * as they contain Interrupt Vector Table Remapped at the start.
    */ 
    ram_app0    (rwx) : ORIGIN = 0x08002400, LENGTH = 0x8000
    ram_app1    (rwx) : ORIGIN = 0x08002400, LENGTH = 0x8000
         
    flash_cm0p  (rx)  : ORIGIN = 0x10000000, LENGTH = 0x2000     
    flash_app0  (rx)  : ORIGIN = 0x10002000, LENGTH = 0x10000
    flash_app1  (rx)  : ORIGIN = 0x10050000, LENGTH = 0x10000

    flash_boot_meta   (rw)  : ORIGIN = 0x100FFA00, LENGTH = 0x400

    /* This is a 32K flash region used for EEPROM emulation. This region can also be used as the general purpose flash.
     * You can assign sections to this memory region for only one of the cores.
     * Note some middleware (e.g. BLE, Emulated EEPROM) can place their data into this memory region.
     * Therefore, repurposing this memory region will prevent such middleware from operation.
     */
    em_eeprom         (rx)    : ORIGIN = 0x14000000, LENGTH = 0x8000       /*  32 KB */


    /* The following regions define device specific memory regions and must not be changed. */
    sflash_user_data  (rx)    : ORIGIN = 0x16000800, LENGTH = 0x800        /* Supervisory flash: User data */
    sflash_nar        (rx)    : ORIGIN = 0x16001A00, LENGTH = 0x200        /* Supervisory flash: Normal Access Restrictions (NAR) */
    sflash_public_key (rx)    : ORIGIN = 0x16005A00, LENGTH = 0xC00        /* Supervisory flash: Public Key */
    sflash_toc_2      (rx)    : ORIGIN = 0x16007C00, LENGTH = 0x200        /* Supervisory flash: Table of Content # 2 */
    sflash_rtoc_2     (rx)    : ORIGIN = 0x16007E00, LENGTH = 0x200        /* Supervisory flash: Table of Content # 2 Copy */
    xip               (rx)    : ORIGIN = 0x18000000, LENGTH = 0x8000000    /* 128 MB */
    efuse             (r)     : ORIGIN = 0x90700000, LENGTH = 0x100000     /*   1 MB */
}

/* Size and start address of the Cortex-M0+ application image */
FLASH_CM0P_START = ORIGIN(flash_cm0p);

/* The DFU SDK metadata limits */
__cy_boot_metadata_addr = ORIGIN(flash_boot_meta);
__cy_boot_metadata_length = __cy_memory_0_row_size;

/* The Product ID, used by CyMCUElfTool to generate a updating file */
__cy_product_id = 0x01020304;

/* The checksum type used by CyMCUElfTool to generate a updating file */
__cy_checksum_type = 0x00;

/* Used by the DFU SDK application to set the metadata */
__cy_app0_verify_start  = ORIGIN(flash_app0);
__cy_app0_verify_length = LENGTH(flash_app0) - __cy_boot_signature_size;
__cy_app1_verify_start  = ORIGIN(flash_app1);
__cy_app1_verify_length = LENGTH(flash_app1) - __cy_boot_signature_size;

/*
* The size of the application signature.
* E.g. 4 for CRC-32,
*     32 for SHA256,
*    256 for RSA 2048.
*/
__cy_boot_signature_size = 4;


/*******************************************************************************
* End of CM4 and CM0+ linker script common region
*******************************************************************************/

/*
* DFU SDK specific: aliases regions, so the rest of code does not use
* application specific memory region names
*/
REGION_ALIAS("flash", flash_app1);
REGION_ALIAS("ram",     ram_app1);


/* DFU SDK specific: sets an app Id */
__cy_app_id = 1;

/* Size and start address of the Cortex-M4 application image */
FLASH_CM4_START  = ORIGIN(flash);

/* DFU SDK specific */
/* CyMCUElfTool uses these ELF symbols to generate an application signature */
__cy_app_verify_start  = ORIGIN(flash);
__cy_app_verify_length = LENGTH(flash) - __cy_boot_signature_size;


/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions flash and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 *
 * For the DFU SDK, these additional symbols are defined:
 *   __cy_app_id
 *   __cy_product_id
 *   __cy_checksum_type
 *   __cy_boot_metadata_addr
 *   __cy_boot_metadata_length
 */


SECTIONS
{   
    /* App0 uses it to initialize DFU SDK metadata, in the dfu_user.c file */
    .cy_boot_metadata :
    {
        KEEP(*(.cy_boot_metadata))
    } > flash_boot_meta

    /* Cortex-M4 application image */
    .text FLASH_CM4_START :
    {
        . = ALIGN(4);
        __Vectors = . ;
        KEEP(*(.vectors))
        . = ALIGN(4);
        __Vectors_End = .;
        __Vectors_Size = __Vectors_End - __Vectors;
        __end__ = .;

        . = ALIGN(4);
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        /* Read-only code (constants). */
        *(.rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)

        KEEP(*(.eh_frame*))
    } > flash


    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > flash

    __exidx_start = .;

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > flash
    __exidx_end = .;


    /* To copy multiple ROM to RAM sections,
     * uncomment .copy.table section and,
     * define __STARTUP_COPY_MULTIPLE in startup_psoc6_01_cm4.S */
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;

        /* Copy interrupt vectors from flash to RAM */
        LONG (__Vectors)                                    /* From */
        LONG (__ram_vectors_start__)                        /* To   */
        LONG (__Vectors_End - __Vectors)                    /* Size */

        /* Copy data section to RAM */
        LONG (__etext)                                      /* From */
        LONG (__data_start__)                               /* To   */
        LONG (__data_end__ - __data_start__)                /* Size */

        __copy_table_end__ = .;
    } > flash


    /* To clear multiple BSS sections,
     * uncomment .zero.table section and,
     * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_psoc6_01_cm4.S */
    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        __zero_table_end__ = .;
    } > flash

    __etext =  . ;


    .ramVectors (NOLOAD) : ALIGN(8)
    {
        __ram_vectors_start__ = .;
        KEEP(*(.ram_vectors))
        __ram_vectors_end__   = .;
    } > ram


    .data __ram_vectors_end__ : AT (__etext)
    {
        __data_start__ = .;

        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);

        KEEP(*(.cy_ramfunc*))
        . = ALIGN(4);

        __data_end__ = .;

    } > ram


    /* Place variables in the section that should not be initialized during the
    *  device startup.
    */
    .noinit (NOLOAD) : ALIGN(8)
    {
      KEEP(*(.noinit))
    } > ram


    /* The uninitialized global or static variables are placed in this section.
    *
    * The NOLOAD attribute tells linker that .bss section does not consume
    * any space in the image. The NOLOAD attribute changes the .bss type to
    * NOBITS, and that  makes linker to A) not allocate section in memory, and
    * A) put information to clear the section with all zeros during application
    * loading.
    *
    * Without the NOLOAD attribute, the .bss section might get PROGBITS type.
    * This  makes linker to A) allocate zeroed section in memory, and B) copy
    * this section to RAM during application loading.
    */
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > ram


    .heap (NOLOAD):
    {
        __HeapBase = .;
        __end__ = .;
        end = __end__;
        KEEP(*(.heap*))
        . = ORIGIN(ram) + LENGTH(ram) - STACK_SIZE;
        __HeapLimit = .;
    } > ram


    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (NOLOAD):
    {
        KEEP(*(.stack*))
    } > ram
    
    /* DFU SDK specific */
    /* The noinit section, used across all the applications */
    .cy_boot_noinit (NOLOAD) :
    {
        KEEP(*(.cy_boot_noinit));
    } > ram_common

    /* The last byte of the section is used for AppId to be shared between all the applications */
    .cy_boot_noinit.appId ORIGIN(ram_common) + LENGTH(ram_common) - 1 (NOLOAD) :
    {
        KEEP(*(.cy_boot_noinit.appId));
    } > ram_common    


    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(ram) + LENGTH(ram);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

    /*
    * The DFU SDK section for an app verification signature.
    * Must be placed at the end of the application.
    * In this case, last N bytes of the last flash row inside the application.
    */
    .cy_app_signature ABSOLUTE(ORIGIN(flash) + LENGTH(flash) - __cy_boot_signature_size) :
    {
        KEEP(*(.cy_app_signature))
    } > flash = 0


    /* Emulated EEPROM flash area */
    .cy_em_eeprom :
    {
        KEEP(*(.cy_em_eeprom))
    } > em_eeprom


    /* Supervisory flash: User data */
    .cy_sflash_user_data :
    {
        KEEP(*(.cy_sflash_user_data))
    } > sflash_user_data


    /* Supervisory flash: Normal Access Restrictions (NAR) */
    .cy_sflash_nar :
    {
        KEEP(*(.cy_sflash_nar))
    } > sflash_nar


    /* Supervisory flash: Public Key */
    .cy_sflash_public_key :
    {
        KEEP(*(.cy_sflash_public_key))
    } > sflash_public_key


    /* Supervisory flash: Table of Content # 2 */
    .cy_toc_part2 :
    {
        KEEP(*(.cy_toc_part2))
    } > sflash_toc_2

    
    /* Supervisory flash: Table of Content # 2 Copy */
    .cy_rtoc_part2 :
    {
        KEEP(*(.cy_rtoc_part2))
    } > sflash_rtoc_2


    /* Places the code in the Execute in Place (XIP) section. See the smif driver
    *  documentation for details.
    */
    .cy_xip :
    {
        KEEP(*(.cy_xip))
    } > xip


    /* eFuse */
    .cy_efuse :
    {
        KEEP(*(.cy_efuse))
    } > efuse


    /* These sections are used for additional metadata (silicon revision,
    *  Silicon/JTAG ID, etc.) storage.
    */
    .cymeta         0x90500000 : { KEEP(*(.cymeta)) } :NONE
}


/* The following symbols used by the cymcuelftool. */
/* flash */
__cy_memory_0_start    = 0x10000000;
__cy_memory_0_length   = 0x00100000;
__cy_memory_0_row_size = 0x200;

/* Emulated EEPROM flash area */
__cy_memory_1_start    = 0x14000000;
__cy_memory_1_length   = 0x8000;
__cy_memory_1_row_size = 0x200;

/* Supervisory flash */
__cy_memory_2_start    = 0x16000000;
__cy_memory_2_length   = 0x8000;
__cy_memory_2_row_size = 0x200;

/* XIP */
__cy_memory_3_start    = 0x18000000;
__cy_memory_3_length   = 0x08000000;
__cy_memory_3_row_size = 0x200;

/* eFuse */
__cy_memory_4_start    = 0x90700000;
__cy_memory_4_length   = 0x100000;
__cy_memory_4_row_size = 1;

/* EOF */
