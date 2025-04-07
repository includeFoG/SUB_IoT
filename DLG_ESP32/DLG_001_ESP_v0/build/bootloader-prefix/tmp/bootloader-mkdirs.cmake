# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "F:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/tmp"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/src/bootloader-stamp"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/src"
  "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/Users/ssm-t/OneDrive/Escritorio/fran/FIRMWARE/EspressifIDE_Workspace/DLG_001_ESP_v0/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
