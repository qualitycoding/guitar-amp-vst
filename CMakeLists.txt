# GrimAmp — rock & metal guitar amp VST
# Framework pin: JUCE 8.0.16-security (tag 8.0.16-security, commit ea77444750de45e036e0279bb0291144bf8e3688)
# — adopted at S-001 over 8.0.15 per D-001's security-patch evaluation, see DEVIATIONS.md.

cmake_minimum_required(VERSION 3.25)

project(GrimAmp VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# K-20261009T092101Z / K-20261009T092102Z: must be at directory scope BEFORE add_subdirectory(JUCE),
# because the Standalone target also compiles the VST3 client.
add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)

add_subdirectory(JUCE)

# ==============================================================================
# Binary data: factory cabinet IRs (8 cabs x close/off-axis) + 24 factory presets
# ==============================================================================
file(GLOB GRIMAMP_IR_FILES   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/ir/*.wav")
file(GLOB GRIMAMP_PRESET_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/presets/*.xml")

juce_add_binary_data(GrimAmpBinaryData
    SOURCES ${GRIMAMP_IR_FILES} ${GRIMAMP_PRESET_FILES}
)

# ==============================================================================
# Shared implementation: all plugin source compiles ONCE into the plugin's
# SharedCode library (created by juce_add_plugin). The unit-test runner and
# the benchmark harness link GrimAmp_SharedCode directly, so JUCE module
# objects and BinaryData exist exactly once (no duplicate-symbol link errors).
# ==============================================================================

# ---------------------------------------------------------------------------
# Header-only view of JUCE for the test/bench harnesses. Linking the real
# JUCE module interface targets here would compile a second copy of every
# module into the executables and duplicate-link against SharedCode; the
# include paths and matching compile definitions are all they need.
# ---------------------------------------------------------------------------
add_library(grimamp_juce_headers INTERFACE)
target_include_directories(grimamp_juce_headers INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules"
    # CabIR.h / PresetManager.h include the generated BinaryData.h; the file
    # materialises at build time in the binary-data target's JuceLibraryCode dir.
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_GrimAmpBinaryData/JuceLibraryCode>"
)
target_compile_definitions(grimamp_juce_headers INTERFACE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
)

# ==============================================================================
# Plugin targets (VST3 / AU / Standalone per A-003)
# ==============================================================================
juce_add_plugin(GrimAmp
    COMPANY_NAME qualitycoding
    COMPANY_WEBSITE "https://github.com/qualitycoding"
    COMPANY_EMAIL "qualitycoding@users.noreply.github.com"
    PLUGIN_MANUFACTURER_CODE Qcd0
    PLUGIN_CODE GrmA
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "GrimAmp"
    VST3_CATEGORIES "Distortion"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(GrimAmp PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Presets/PresetManager.cpp
    Source/DSP/UserIRLoader.cpp
)

target_compile_definitions(GrimAmp PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    GRIMAMP_BINARYDATA_AVAILABLE=1
)

target_link_libraries(GrimAmp PRIVATE
    GrimAmpBinaryData
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
)

juce_generate_juce_header(GrimAmp)

# ==============================================================================
# Tests + bench (see tests/CMakeLists.txt)
# ==============================================================================
option(GRIMAMP_BUILD_TESTS "Build the GrimAmp test and bench harnesses" ON)
if(GRIMAMP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
