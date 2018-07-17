# path to compiler and utilities
# specify the cross compiler
SET(CMAKE_C_COMPILER arm-none-linux-gnueabi-gcc)

# Name of the target platform
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)

# Version of the system
SET(CMAKE_SYSTEM_VERSION 1)
#SET(TOOLCHAIN_DIR "/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib"  "/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/include/openssl ")
#SET(CMAKE_FIND_ROOT_PATH $(TOOLCHAIN_DIR))
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#SET(PAHO_WITH_SSL TRUE)
#SET(PAHO_BUILD_SAMPLES TRUE )
#SET(PAHO_BUILD_DOCUMENTATION TRUE )
#SET(PAHO_BUILD_STATIC TRUE)
#SET(OPENSSL_LIB_SEARCH_PATH /tmp/libssl-dev/usr/lib/arm-linux-gnueabihf )
#SET(OPENSSL_INC_SEARCH_PATH  "/tmp/libssl-dev/usr/include/openssl;/tmp/libssl-dev/usr/include/arm-linux-gnueabihf" )
#SET(CMAKE_TOOLCHAIN_FILE ~/git/org.eclipse.paho.mqtt.c/cmake/toolchain.linux-arm11.cmake ~/git/org.eclipse.paho.mqtt.c)
#

#需要设置OPENSSL_SEARCH_PATH路经
#cmake -DPAHO_WITH_SSL=TRUE -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain.linux-arm11.cmake ..
#下面是IMX的OPENSSL库文件路经
#OPENSSL_LIB=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib 
#OPENSSL_INCLUDE_DIR=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/include/openssl -DOPENSSLCRYPTO_LIB=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib ..

SET(OPENSSL_SEARCH_PATH "/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr")

#修改了../src/CMakeLists.txt  Line:105 通过CMAKE_SYSTEM_PROCESSOR 来选择OPENSSL_SEARCH_PATH 搜索路径

