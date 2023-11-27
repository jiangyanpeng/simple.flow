include(ExternalProject)

set(BASE_NAME simple.base)
set(BASE_WORK_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(BASE_BUILD_DIR ${PROJECT_BINARY_DIR}/third_party/${BASE_NAME})
set(BASE_INSTALL_DIR ${PROJECT_SOURCE_DIR}/install/${BASE_NAME})

set(BASE_CMAKE_ARGS)
list(APPEND BASE_CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${BASE_INSTALL_DIR}
        -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release)

if(NOT APPLE)
    list(APPEND BASE_CMAKE_ARGS
        -DCMAKE_CXX_FLAGS:STRING="-fPIC"
        -DCMAKE_CXX_FLAGS:STRING="-w"
        -DCMAKE_C_FLAGS:STRING="-fPIC"
        -DCMAKE_C_FLAGS:STRING="-w")
    list(APPEND BASE_CMAKE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
endif()

if(ANDROID)
    list(APPEND BASE_CMAKE_ARGS
            -DANDROID_ABI=${ANDROID_ABI}
            -DANDROID_PLATFORM=${ANDROID_PLATFORM}
            -DANDROID_ARM_NEON=${ANDROID_ARM_NEON})
elseif(APPLE)
    list(APPEND BASE_CMAKE_ARGS -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})

    if("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
        list(APPEND BASE_CMAKE_ARGS "-DCMAKE_SYSTEM_PROCESSOR=aarch64")
    endif()
elseif(UNIX)
    list(APPEND BASE_CMAKE_ARGS -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR})
endif()

pipe_download_dependency(
    "https://github.com/jiangyanpeng/simple.base.git"
    v2.0.4
    ${BASE_NAME}
    ${BASE_WORK_DIR}
    )

if(WIN32)
    set(BASE_LIB_NAME "basestatic.lib")
else()
    set(BASE_LIB_NAME "libbase.a")
endif()

ExternalProject_Add(
    ${BASE_NAME}
    PREFIX ${BASE_WORK_DIR}/${BASE_NAME}
    CMAKE_ARGS ${BASE_CMAKE_ARGS}
    SOURCE_DIR ${BASE_WORK_DIR}/${BASE_NAME}
    TMP_DIR ${BASE_BUILD_DIR}/tmp
    BINARY_DIR ${BASE_BUILD_DIR}
    STAMP_DIR ${BASE_BUILD_DIR}/stamp
    BUILD_BYPRODUCTS ${BASE_INSTALL_DIR}/lib/${BASE_LIB_NAME}
)

####### 使用编译好的外部库 #########
# step1: 使用add_library，并使用IMPORTED标志，表明使用外部库
# step2: 使用set_preperty，指定外部库的位置
# step3: 使用include_directories，添加头文件
add_library(base STATIC IMPORTED)
set_property(TARGET base
        PROPERTY IMPORTED_LOCATION
        ${BASE_INSTALL_DIR}/lib/${BASE_LIB_NAME})
include_directories(${BASE_INSTALL_DIR}/include)

list(APPEND CV_LINK_DEPS base)

if(NOT BUILD_SHARED_LIBS)
    list(APPEND CV_EXPORT_LIBS ${BASE_INSTALL_DIR}/lib/${BASE_LIB_NAME})
endif()

list(APPEND CV_EXTERNAL_DEPS ${BASE_NAME})
