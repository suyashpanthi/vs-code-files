cmake_minimum_required(VERSION 3.22)
project(ProVocal VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to JUCE — adjust if cloned elsewhere
set(JUCE_PATH "$ENV{HOME}/Developer/JUCE")
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(ProVocal
    COMPANY_NAME "ProVocal Audio"
    BUNDLE_ID "com.provocalaudio.provocal"
    PLUGIN_MANUFACTURER_CODE PvAu
    PLUGIN_CODE PrVo
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "ProVocal"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
    VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
)

juce_generate_juce_header(ProVocal)

target_sources(ProVocal PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(ProVocal PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ProVocal
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
