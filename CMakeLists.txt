cmake_minimum_required(VERSION 3.22)
project(LoFiTapeSimulator VERSION 1.0.0)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9 # Можно указать нужную версию
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LoFiTapeSimulator
    COMPANY_NAME "LoFiLabs"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Hntr
    PLUGIN_CODE Tape
    FORMATS VST3 Standalone
    PRODUCT_NAME "Lo-Fi Tape Simulator"
    PLUGIN_CATEGORY "Fx"
    VST3_CATEGORIES "Fx"
)

target_sources(LoFiTapeSimulator PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

target_compile_features(LoFiTapeSimulator PRIVATE cxx_std_17)
juce_generate_juce_header(LoFiTapeSimulator)

target_link_libraries(LoFiTapeSimulator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
