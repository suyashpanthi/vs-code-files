cmake_minimum_required(VERSION 3.22)
project(DAW VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_PATH "$ENV{HOME}/Developer/JUCE")
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_gui_app(DAW
    COMPANY_NAME "DAW Studio"
    BUNDLE_ID "com.dawstudio.daw"
    PRODUCT_NAME "DAW"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE
)

juce_generate_juce_header(DAW)

target_sources(DAW PRIVATE
    Source/Main.cpp

    # App
    Source/App/DAWApplication.cpp
    Source/App/MainWindow.cpp

    # Engine
    Source/Engine/AudioEngine.cpp
    Source/Engine/Transport.cpp
    Source/Engine/TempoMap.cpp
    Source/Engine/MasterBusSource.cpp
    Source/Engine/TrackAudioSource.cpp

    # Model
    Source/Model/Project.cpp
    Source/Model/Track.cpp
    Source/Model/AudioRegion.cpp
    Source/Model/AudioFileCache.cpp

    # UI
    Source/UI/DAWLookAndFeel.cpp
    Source/UI/MainComponent.cpp

    # UI - Toolbar
    Source/UI/Toolbar/TransportBar.cpp

    # UI - Tracks
    Source/UI/Tracks/TrackListPanel.cpp
    Source/UI/Tracks/TrackHeaderComponent.cpp
    Source/UI/Tracks/TrackLaneComponent.cpp
    Source/UI/Tracks/TimelineRuler.cpp
    Source/UI/Tracks/PlayheadOverlay.cpp
    Source/UI/Tracks/WaveformView.cpp

    # UI - Mixer
    Source/UI/Mixer/MixerPanel.cpp
    Source/UI/Mixer/ChannelStrip.cpp
    Source/UI/Mixer/MasterStrip.cpp
    Source/UI/Mixer/LevelMeter.cpp

    # Utils
    Source/Utils/TimeFormat.cpp

    # File IO
    Source/FileIO/ProjectSerializer.cpp
    Source/FileIO/AudioImporter.cpp
    Source/FileIO/BounceExporter.cpp
)

target_compile_definitions(DAW PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DAW,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DAW,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DAW
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
