find_package(rpclib CONFIG REQUIRED)

file(GLOB _src ./*)
add_executable(tst_talcs_RemoteSocket)
target_sources(tst_talcs_RemoteSocket PRIVATE ${_src})
target_link_libraries(tst_talcs_RemoteSocket PRIVATE TalcsRemote)