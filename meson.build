project('stm32h7-flm', 'c', 'cpp',
          default_options : ['b_lto=false',
                             'b_asneeded=false',
                             'c_std=c11', 
                             'cpp_std=c++20'])

#==============================================================================#

flash_name = 'W25Q64JV'
flash_driver_name = '-DFLASH_LDR_NAME="@0@_STM32H7x3"'.format(flash_name)
# Initialize some globals
fpu           = 'soft' # FPU usage
fpu_arch      = 'none' # FPU technology
# flash driver name for stldr and flm

# common source file
srcs          = ['Src/QSPI/qspi.cpp',  'Src/Test/sysmem.c', 'Src/Config/SystemInit.c']
# common inclusion
incdirs       = ['Src/Config','Src/QSPI', 'cmsis_device_h7/Include', 'CMSIS_5/CMSIS/Core/Include']
c_args_plus   = [flash_driver_name]
cpp_args_plus = ['-nostdlib', '-fno-rtti', '-fno-exceptions', '-fno-unwind-tables',flash_driver_name]
link_args     = ['-specs=nosys.specs']
link_deps     = []

#==============================================================================#
# uController / HAL Driver dependend options
c_args_plus     += '-DSTM32H750xx' # MCU define
cpp_args_plus   += '-DSTM32H750xx' # MCU define

c_args_plus     += '-D@0@'.format(flash_name) # Flash define
cpp_args_plus   += '-D@0@'.format(flash_name) # Flash define


#==============================================================================#
# convenience function : get correct -mcpu flag depending on hostmachine
cpu = host_machine.cpu() == 'cortex-m0+' ? 'cortex-m0plus' : host_machine.cpu()
if fpu_arch == 'none'
  cpu_args = ['-mcpu=@0@@1@'.format( cpu, '+nofp' )]
else 
  cpu_args = ['-mcpu=@0@@1@'.format( cpu, '' )]
  cpu_args += '-mfloat-abi=@0@'.format (fpu)
  cpu_args += '-mfpu=@0@'.format(fpu_arch)
endif
c_args_plus += cpu_args
cpp_args_plus += cpu_args
link_args += cpu_args

#==============================================================================#
# convenience function : link correct C-runtime for each controller
# TODO: use llvm compiler-rt
arch       = (host_machine.cpu() == 'cortex-m0') or (host_machine.cpu() == 'cortex-m0+') or (host_machine.cpu() == 'cortex-m0+') or (host_machine.cpu() == 'cortex-m1') ? 'armv6-m'  : ''
arch      += (host_machine.cpu() == 'cortex-m3') ?                                                                                'armv7-m'  : ''
arch      += (host_machine.cpu() == 'cortex-m4') or (host_machine.cpu() == 'cortex-m7') ?                                         'armv7e-m' : ''
#==============================================================================#
# create Debug Flags, depending on meson buildtype
c_args_plus += get_option('buildtype') == 'release' ? '-DNDEBUG' : '-DDEBUG'

cpp_args_plus += get_option('buildtype') == 'release' ? '-DNDEBUG' : '-DDEBUG'
#==============================================================================#
# parse linkerfiles to a linker commando
#==============================================================================#
main = executable(
            'main',
            [srcs, 'Src/Test/main.cpp', 'Src/Test/startup.c'],
            name_suffix         : 'elf',
            c_args              : [c_args_plus ],
            cpp_args            : [cpp_args_plus ],
            link_args           : [link_args,'-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'Src/Test/linker.ld'), 
                                              '-Wl,-Map=@0@.map,--cref'.format('main'),
                                              '-Wl,--gc-sections'],
            dependencies        : link_deps,
            include_directories : [incdirs] )


flm_related_flag = ['-fpic', '-msingle-pic-base', '-mpic-register=9' ,'-fno-jump-tables']
flm = executable(
            'ext_loader_flm',
            [srcs, 'Src/FLM/FlashDev.cpp', 'Src/FLM/FlashPrg.cpp'] ,
            name_suffix         : 'flm',
            c_args              : [c_args_plus,flm_related_flag],
            cpp_args            : [cpp_args_plus,  flm_related_flag],
            link_args           : [link_args, flm_related_flag, 
                                              '-nostartfiles', 
                                              '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'Src/FLM/linker.ld'), 
                                              '-Wl,-Map=@0@.map,--cref'.format('flm'),
                                              '-Wl,--gc-sections'],
            dependencies        : link_deps,
            include_directories : [incdirs, 'Src/FLM'] )


stldr = executable(
            'ext_loader_stldr',
            [srcs, 'Src/STLDR/Loader_Src.cpp', 'Src/STLDR/Dev_Inf.cpp'] ,
            name_suffix         : 'stldr',
            c_args              : [c_args_plus ],
            cpp_args            : [cpp_args_plus ],
            link_args           : [link_args, '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'Src/STLDR/linker.ld'), 
                                              '-Wl,-Map=@0@.map,--cref'.format('stldr'),
                                              '-Wl,--gc-sections'],
            dependencies        : link_deps,
            include_directories : [incdirs, 'Src/STLDR'] )

#==============================================================================#
# import binary objects
objcopy  = '@0@'.format(find_program('objcopy').path())
objdump  = '@0@'.format(find_program('objdump').path())
size     = '@0@'.format(find_program('size').path())
gdb      = '@0@'.format(find_program('gdb').path())
# terminal = '@0@'.format(find_program('terminal',required:false).path())
# openocd  = '@0@'.format(find_program('openocd', required:false).path())

#==============================================================================#
# create custom target : create binary, hex dump, size and disassemble dump
mainbin = custom_target(
                        'main.bin',
    output           : ['main.bin'],
    # name_suffix      : 'bin',
    build_by_default : true,
    command          : [objcopy, '-O', 'binary', 'main.elf', 'main.bin'],
    depends          : [main])

mainhex = custom_target(
                            'main.hex',
        output           : ['main.hex'],
        build_by_default : true,
        command          : [objcopy, '-O', 'ihex', 'main.elf', 'main.hex'],
        depends          : [main])

mainsize = custom_target(
                          'size',
        capture          : true,
        output           : ['main.size'],
        build_by_default : true,
        command          : [size, '--format=berkeley', 'main.elf'],
        depends          : [main])