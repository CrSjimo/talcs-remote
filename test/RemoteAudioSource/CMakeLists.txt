juce_add_gui_app(tst_talcs_RemoteAudioSource)

file(GLOB _src ./*)

target_sources(tst_talcs_RemoteAudioSource PRIVATE ${_src})

target_compile_definitions(tst_talcs_RemoteAudioSource
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:tst_talcs_RemoteAudioSource,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:tst_talcs_RemoteAudioSource,JUCE_VERSION>")


target_link_libraries(tst_talcs_RemoteAudioSource
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        TalcsRemote
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
