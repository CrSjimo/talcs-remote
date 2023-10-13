find_package(rpclib CONFIG REQUIRED)
find_package(Boost REQUIRED)

file(GLOB _src ./*)
add_library(TalcsRemote STATIC ${_src})
target_link_libraries(TalcsRemote
    PUBLIC
        juce::juce_audio_utils
        rpclib::rpc
    PRIVATE
        Boost::boost
)
target_include_directories(TalcsRemote PUBLIC .)