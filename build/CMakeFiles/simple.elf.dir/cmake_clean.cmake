file(REMOVE_RECURSE
  "config/sdkconfig.h"
  "config/sdkconfig.cmake"
  "bootloader/bootloader.elf"
  "bootloader/bootloader.bin"
  "bootloader/bootloader.map"
  "simple.bin"
  "simple.map"
  "project_elf_src.c"
  "project_elf_src.c"
  "CMakeFiles/simple.elf.dir/project_elf_src.c.obj"
  "simple.elf.pdb"
  "simple.elf"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/simple.elf.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
