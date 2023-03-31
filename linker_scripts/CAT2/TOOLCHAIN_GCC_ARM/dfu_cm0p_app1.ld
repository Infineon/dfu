/***************************************************************************//**
* \file dfu_cm0p_app0.ld
* \version 5.0
*
* Linker file for the GNU C compiler.
* Used for the DFU SDK application 1 firmware project.
* Application 1 is uploaded through Bootloader Host Tool
* using the generated cyadc2 file.
*
* The main purpose of the linker script is to describe how the sections in the
* input files should be mapped into the output file, and to control the memory
* layout of the output file.
*
* \note The entry point location is fixed and starts at 0x00000000. The valid
* application image should be placed there.
*
* \note The linker files included with the PDL template projects must be generic
* and handle all common use cases. Your project may not use every section
* defined in the linker files. In that case you may see warnings during the
* build process. In your project, you can simply comment out or remove the
* relevant code in the linker file.
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation. All rights reserved.
********************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation or one of its
* affiliates ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/*--------------------- Flash Configuration ----------------------------------
; <h> Flash Configuration
;   <o0> Flash Base Address <0x0-0xFFFFFFFF:8>
;   <o1> Flash Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
 -----------------------------------------------------------------------------*/
__FLASH_START = 0x00000000;
__FLASH_SIZE =  0x00020000;

/*--------------------- Embedded RAM Configuration ---------------------------
; <h> RAM Configuration
;   <o0> RAM Base Address    <0x0-0xFFFFFFFF:8>
;   <o1> RAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
 -----------------------------------------------------------------------------*/
__RAM_START = 0x20000000;
__RAM_SIZE =  0x00004000;

/*--------------------- Stack / Heap Configuration ---------------------------
; <h> Stack / Heap Configuration
;   <o0> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;   <o1> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
 -----------------------------------------------------------------------------*/
__STACK_SIZE = 0x00000400;
__HEAP_SIZE  = 0x00000080;

/* The MEMORY section below describes the location and size of blocks of memory in the target.
* Use this section to specify the memory regions available for allocation.
*/
MEMORY
{
 /* The ram and FLASH regions control RAM and FLASH memory allocation.
  *  You can change the memory allocation by editing the 'ram' and 'FLASH' regions.
  */
  FLASH_app0 (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00009000
  FLASH_app1 (rx)  : ORIGIN = 0x00010000, LENGTH = 0x00008000

  ram_common (rwx) : ORIGIN = 0x20000000,   LENGTH = 0x0400
  RAM_app0   (rwx) : ORIGIN = 0x20000400,   LENGTH = 0x00003000
  RAM_app1   (rwx) : ORIGIN = 0x20000400,   LENGTH = 0x00003000

  flash_boot_meta   (rw)  : ORIGIN = 0x0001FA00, LENGTH = 0x100

}

/* The DFU SDK metadata limits */
__cy_boot_metadata_addr = ORIGIN(flash_boot_meta);
__cy_boot_metadata_length = __cy_memory_0_row_size;

/* The Product ID, used by CyMCUElfTool to generate a updating file */
__cy_product_id = 0x01020304;

/* The checksum type used by CyMCUElfTool to generate a updating file */
__cy_checksum_type = 0x00;

/* Used by the DFU SDK application to set the metadata */
__cy_app0_verify_start  = ORIGIN(FLASH_app0);
__cy_app0_verify_length = LENGTH(FLASH_app0) - __cy_boot_signature_size;
__cy_app1_verify_start  = ORIGIN(FLASH_app1);
__cy_app1_verify_length = LENGTH(FLASH_app1) - __cy_boot_signature_size;

/*
* The size of the application signature.
* E.g. 4 for CRC-32,
*     32 for SHA256,
*    256 for RSA 2048.
*/
__cy_boot_signature_size = 4;

/*
* DFU SDK specific: aliases regions, so the rest of code does not use
* application specific memory region names
*/
REGION_ALIAS("FLASH", FLASH_app1);
REGION_ALIAS("RAM",     RAM_app1);


/* DFU SDK specific: sets an app Id */
__cy_app_id = 1;

/* DFU SDK specific */
/* CyMCUElfTool uses these ELF symbols to generate an application signature */
__cy_app_verify_start  = ORIGIN(FLASH);
__cy_app_verify_length = LENGTH(FLASH) - __cy_boot_signature_size;


/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
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
ENTRY(Reset_Handler)

SECTIONS
{
    /* App0 uses it to initialize DFU SDK metadata, in the dfu_user.c file */
    .cy_boot_metadata :
    {
        KEEP(*(.cy_boot_metadata))
    } > flash_boot_meta

   .cy_app_header :
    {
        KEEP(*(.cy_app_header))
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        __Vectors = . ;
        KEEP(*(.vectors))
        . = ALIGN(4);
        __Vectors_End = .;
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
    } > FLASH

   .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;

        /* Copy data section to RAM */
        LONG (__etext)                                      /* From */
        LONG (__data_start__)                               /* To   */
        LONG ((__data_end__ - __data_start__) / 4)          /* Size */

        __copy_table_end__ = .;
    } > FLASH

    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG ((__bss_end__ - __bss_start__) / 4)
        __zero_table_end__ = .;
    } > FLASH

   /* Location counter can end up 2byte aligned with narrow Thumb code but
    *  __etext is assumed by startup code to be the LMA of a section in RAM
    *  which must be 4byte aligned.
    */
    __etext = ALIGN (4);


    .ramVectors (NOLOAD) : ALIGN(8)
    {
        __ram_vectors_start__ = .;
        KEEP(*(.ram_vectors))
        __ram_vectors_end__   = .;
    } > RAM


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
        /* All data end */
        __data_end__ = .;

    } > RAM

    /* Place variables in the section that should not be initialized during the
    *  device startup.
    */
    .noinit (NOLOAD) : ALIGN(8)
    {
      KEEP(*(.noinit))
    } > RAM


    /* Secondary data section, optional
    *
    * Remember to add each additional data section
    * to the .copy.table above to asure proper
    * initialization during startup.
    */
    /*
    __etext2 = ALIGN (4);

    .data2 : AT (__etext2)
    {
    . = ALIGN(4);
        __data2_start__ = .;
    *(.data2)
    *(.data2.*)
        . = ALIGN(4);
        __data2_end__ = .;

    } > RAM
    */

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
    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
    *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    /**
    * Secondary bss section, optional
    *
    * Remember to add each additional bss section
    * to the .zero.table above to asure proper
    * initialization during startup.
    */
    /*
    .bss2 :
    {
        . = ALIGN(4);
        __bss2_start__ = .;
    *(.bss2)
        *(.bss2.*)
        . = ALIGN(4);
        __bss2_end__ = .;
    } > RAM2
    */

    .heap :
    {
    . = ALIGN(4);
        __end__ = .;
        PROVIDE(end = .);
        . = . + __HEAP_SIZE;
    . = ALIGN(4);
        __HeapLimit = .;
    } > RAM
   /* .stack section doesn't contains any symbols. It is only used
    * for linker to calculate size of stack sections, and assign
    * values to stack symbols later.
    */
    .stack :
    {
       /* Set stack top to end of RAM, and stack limit move down by
        * size of stack_dummy section
        */
        . = ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE;
        . = ALIGN(4);
        __StackLimit = .;
        . = . + __STACK_SIZE;
        . = ALIGN(4);
        __StackTop = .;
    } > RAM
    PROVIDE(__stack = __StackTop);

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

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

    /*
    * The DFU SDK section for an app verification signature.
    * Must be placed at the end of the application.
    * In this case, last N bytes of the last FLASH row inside the application.
    */
    .cy_app_signature ABSOLUTE(ORIGIN(FLASH) + LENGTH(FLASH) - __cy_boot_signature_size) :
    {
        KEEP(*(.cy_app_signature))
    } > FLASH = 0

    /* These sections are used for additional metadata (silicon revision,
    * Silicon/JTAG ID, etc.) storage.
    */
    .cyflashprotect 0x90400000 : { KEEP(*(.cyflashprotect)) } :NONE
    .cymeta         0x90500000 : { KEEP(*(.cymeta)) } :NONE
    .cychipprotect  0x90600000 : { KEEP(*(.cychipprotect)) } :NONE
}


/* The following symbols used by the cymcuelftool. */
/* Flash */
__cy_memory_0_start    = 0x00000000;
__cy_memory_0_length   = 0x00020000;
__cy_memory_0_row_size = 0x100;

/* EOF */
