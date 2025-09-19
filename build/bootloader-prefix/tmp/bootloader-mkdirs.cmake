# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/13588/esp/v5.3/esp-idf/components/bootloader/subproject"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/tmp"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/src/bootloader-stamp"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/src"
  "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/rengxing_work/project/rmt_ctrl/sample_project/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
