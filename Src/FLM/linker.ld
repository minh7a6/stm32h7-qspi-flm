/* Flash OS Routines
 * Copyright (c) 2009-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

PHDRS {Loader PT_LOAD ; SgInfo PT_LOAD ; }
_Min_Stack_Size = 0x400; /* required amount of stack */
/*
 * The PrgCode and PrgData output section names come from the CMSIS-Pack flash algo
 * templates and armlink. It is used here because several tools that work
 * with these flash algos expect this section name.
 */
SECTIONS
{
    PrgCode 0 : ALIGN_WITH_INPUT
    {
      *(.text)           /* .text sections (code) */
      *(.text*)          /* .text* sections (code) */
      *(.glue_7)         /* glue arm to thumb code */
      *(.glue_7t)        /* glue thumb to arm code */
      *(.eh_frame)

      /* Ensure entry points are pulled in here */
      . = ALIGN(4);
      KEEP (*(.text.Init))
      KEEP (*(.text.UnInit))
      KEEP (*(.text.BlankCheck))
      KEEP (*(.text.Read))
      KEEP (*(.text.EraseChip))
      KEEP (*(.text.EraseSector))
      KEEP (*(.text.ProgramPage))
      KEEP (*(.text.Verify))

      KEEP (*(.init))
      KEEP (*(.fini))
      *(.rodata)         /* .rodata sections (constants, strings, etc.) */
      *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
      *(COMMON)
        . = ALIGN(16);
    } : Loader

    PrgData : ALIGN_WITH_INPUT
    {
      . = ALIGN(16);

      *(.data)           /* .data sections */
      *(.data*)          /* .data* sections */
      KEEP (*(.got*))
      . = ALIGN(16);

    } : Loader
    /* PrgData (NOLOAD): ALIGN_WITH_INPUT
    {
      _sbss = .; 
      __bss_start__ = _sbss;
      *(.bss)
      *(.bss*)
      *(COMMON)
      _ebss = .;
      __bss_end__ = _ebss;
      KEEP(*(PrgDataBss))
        . = ALIGN(16);
    } : Loader */

    DevDscr : ALIGN_WITH_INPUT
    {
        KEEP(*(DevDscr))
    } : SgInfo
      /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
      . = ALIGN(4);
      PROVIDE ( end = . );
      PROVIDE ( _end = . );
      . = . + _Min_Stack_Size;
      . = ALIGN(4);
    } : Loader
    /DISCARD/ : {
        /* Unused exception related info that only wastes space */
        *(.ARM.exidx);
        *(.ARM.exidx.*);
        *(.ARM.extab.*);
    }
}