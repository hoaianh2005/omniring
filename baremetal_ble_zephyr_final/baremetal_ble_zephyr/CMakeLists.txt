cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(untitled_1)

target_sources(app PRIVATE src/app/main.c src/app/bleCtrlTasks.c 
src/app/imuCtrlTasks.c src/app/ppgCtrlTasks.c src/app/gnssCtrlTasks.c src/app/lteCtrlTasks.c src/app/gaugeCtrlTasks.c 
src/lib/bmi270.c src/lib/lexiR10.c src/lib/as7057.c src/lib/bq27220.c 
src/hwdefine/hwDefine.c)

target_include_directories(app PRIVATE
    src/app
    src/lib
    src/hwdefine
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/app
${CMAKE_CURRENT_SOURCE_DIR}/src/lib
${CMAKE_CURRENT_SOURCE_DIR}/src/hwdefine)
