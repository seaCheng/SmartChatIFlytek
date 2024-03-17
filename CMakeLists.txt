CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

project(SmartTalkRobot)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

SET(CMAKE_BUILD_TYPE Release)
set(CMAKE_INSTALL_PREFIX ./lib)


# Create valoc cplusplus library
add_library(SmartTalkRobot SHARED lib
        SmartTalkRobot/Audiocontroler.cpp
        SmartTalkRobot/Timetool.hpp
        SmartTalkRobot/Utils.hpp
		SmartTalkRobot/Encoding.cpp
        SmartTalkRobot/Vocieai-vcs-api.cpp
		SmartTalkRobot/rtaudio/Rtaudio.cpp		
)

if(WIN32)

add_definitions(-DVCS_API_EXPORTS)
target_compile_definitions(SmartTalkRobot PRIVATE __WINDOWS_WASAPI__)

target_link_directories(SmartTalkRobot PRIVATE SmartTalkRobot/xunFei/libs)

target_link_libraries(SmartTalkRobot PRIVATE    
    msc_x64
)

else()
 target_compile_definitions(SmartTalkRobot PRIVATE __LINUX_ALSA__  __LINUX_PULSE__)

target_link_libraries(SmartTalkRobot PRIVATE
        
)   

endif()

include_directories(SmartTalkRobot)
include_directories(include)
include_directories(SmartTalkRobot/rtaudio)
include_directories(SmartTalkRobot/json)
include_directories(SmartTalkRobot/xunFei/include)
include_directories(SmartTalkRobot/spark)

target_include_directories(SmartTalkRobot PRIVATE
        SmartTalkRobot
        include
        SmartTalkRobot/rtaudio
		SmartTalkRobot/xunFei/libs
		SmartTalkRobot/spark
)

target_link_directories(SmartTalkRobot PRIVATE SmartTalkRobot/spark/lib)
target_link_libraries(SmartTalkRobot PRIVATE SparkChain)

install(TARGETS SmartTalkRobot LIBRARY DESTINATION lib)


add_executable(demo example/demo.cpp)

if(WIN32)

target_include_directories(demo PRIVATE include)

target_link_directories(demo PRIVATE lib)
target_link_libraries(demo PRIVATE SmartTalkRobot)


target_link_directories(demo PRIVATE SmartTalkRobot/xunFei/libs)

target_link_libraries(demo PRIVATE    
    msc_x64
)

target_link_directories(demo PRIVATE SmartTalkRobot/spark/lib)
target_link_libraries(demo PRIVATE SparkChain)

# Visual Studio中的调试工作目录（即exe程序内的“./”所指的目录，仅在VS调试中有效）
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${ProjectDir}./Release")

else()

target_include_directories(demo PRIVATE include)
target_include_directories(demo PRIVATE include/)
target_link_directories(demo PRIVATE lib)
target_link_libraries(demo PRIVATE SmartTalkRobot)
target_link_libraries(demo PRIVATE pulse)
target_link_libraries(demo PRIVATE pulse-simple)
target_link_libraries(demo PRIVATE asound)

#classifier-release -lCollectorSecondGen -lqualitycheck-release -lpulse -lpulse-simple -lasound

endif()

