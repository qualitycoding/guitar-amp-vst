/*
  Parameters.h — GrimAmp parameter layout (D-005).

  Every user-facing control is declared ONCE here as an APVTS parameter with a
  stable kebab-case string ID. GUI (SliderAttachment/ButtonAttachment/
  ComboBoxAttachment), factory presets and host automation all read from this
  header. IDs are frozen at v1.0.0 and must never be renamed or reused.

  The frozen inventory (tests/parameter_ids.txt) is generated from this file.
*/
#pragma once

namespace gramp
{
namespace param
{
    // Input / gate
    inline constexpr auto inputGain    = "input-gain";      // float, dB
    inline constexpr auto gateOn       = "gate-on";         // bool
    inline constexpr auto gateThreshold= "gate-threshold";  // float, dB

    // Boost (TS-style)
    inline constexpr auto boostOn      = "boost-on";        // bool
    inline constexpr auto boostAmount  = "boost-amount";    // float 0..1

    // Preamp
    inline constexpr auto drive        = "drive";           // float 0..1

    // Tone stack
    inline constexpr auto bass         = "bass";            // float 0..1
    inline constexpr auto mid          = "mid";             // float 0..1
    inline constexpr auto treble       = "treble";          // float 0..1
    inline constexpr auto presence     = "presence";        // float 0..1
    inline constexpr auto depth        = "depth";           // float 0..1

    // Power amp
    inline constexpr auto sag          = "sag";             // float 0..1

    // Stepped choices
    inline constexpr auto ampVoicing   = "amp-voicing";     // choice, 7 voicings (A-005)
    inline constexpr auto cabIr        = "cab-ir";          // choice, 8 factory + user slot
    inline constexpr auto oversampling = "oversampling";    // choice Eco/High

    // Cabinet shaping
    inline constexpr auto micPosition  = "mic-position";    // float 0..1 (close -> off-axis)
    inline constexpr auto irMix        = "ir-mix";          // float 0..1 (dry/wet)

    // Post EQ
    inline constexpr auto postEqOn     = "post-eq-on";      // bool
    inline constexpr auto postEqLow    = "post-eq-low";     // float, dB shelf
    inline constexpr auto postEqHigh   = "post-eq-high";    // float, dB shelf

    // Output
    inline constexpr auto master       = "master";          // float 0..1
} // namespace param

namespace voicing
{
    inline constexpr const char* const names[] =
    {
        "Tweed", "Plexi", "JCM800", "Recto", "5150", "Chainsaw", "Frost"
    };
    inline constexpr int count = 7;
} // namespace voicing

namespace cab
{
    inline constexpr const char* const names[] =
    {
        "British 4x12", "American 4x12", "Vintage 2x12", "Tweed 1x12",
        "British 2x12", "Modern 4x12",  "Green 4x12",   "Doom 4x15",
        "User IR"
    };
    inline constexpr int factoryCount = 8;
    inline constexpr int userSlotIndex = 8;
    inline constexpr int count = 9;

    // BinaryData file naming: ir/cab{N}_{close|off}.wav -> BinaryData::{name}
    inline const char* getBinaryDataName (int cabIndex, bool offAxis)
    {
        static const char* closeNames[] =
        {
            "cab1_close_wav", "cab2_close_wav", "cab3_close_wav", "cab4_close_wav",
            "cab5_close_wav", "cab6_close_wav", "cab7_close_wav", "cab8_close_wav"
        };
        static const char* offNames[] =
        {
            "cab1_off_wav", "cab2_off_wav", "cab3_off_wav", "cab4_off_wav",
            "cab5_off_wav", "cab6_off_wav", "cab7_off_wav", "cab8_off_wav"
        };
        jassert (cabIndex >= 0 && cabIndex < factoryCount);
        return offAxis ? offNames[cabIndex] : closeNames[cabIndex];
    }
} // namespace cab

namespace oversampling
{
    inline constexpr const char* const names[] = { "Eco", "High" };
    inline constexpr int ecoIndex = 0;
    inline constexpr int highIndex = 1;
    inline constexpr int count = 2;
} // namespace oversampling
} // namespace gramp
