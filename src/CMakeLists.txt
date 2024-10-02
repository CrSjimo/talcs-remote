find_package(rpclib CONFIG REQUIRED)
find_package(Boost REQUIRED)

file(GLOB _src ./*)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library(TalcsRemote STATIC ${_src})
target_link_libraries(TalcsRemote
    PUBLIC
        juce::juce_core
        juce::juce_audio_basics
        rpclib::rpc
    PRIVATE
        Boost::boost
)

target_compile_definitions(TalcsRemote
    PRIVATE
        JUCE_USE_CURL=0
)
target_include_directories(TalcsRemote PUBLIC .)