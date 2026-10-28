/*
  TestMain.cpp — console runner for the frozen ctest suite.
  Usage: grimamp_tests [test-name ...]      (no args = run all)
  Env:   GRIMAMP_FUZZ_DIR=<corpus dir> for fuzz_presets (T-013)
*/
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);

    auto all = juce::UnitTest::getAllTests();

    if (argc <= 1)
    {
        runner.runTests (all);
    }
    else
    {
        juce::Array<juce::UnitTest*> selected;
        for (int i = 1; i < argc; ++i)
            for (auto* t : all)
                if (t->getName() == juce::String (argv[i]).trim())
                    selected.addIfNotAlreadyThere (t);

        if (selected.isEmpty())
        {
            std::cerr << "no matching tests\n";
            return 2;
        }
        runner.runTests (selected);
    }

    int failed = 0, succeeded = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* r = runner.getResult (i);
        failed += r->failures;
        succeeded += r->passes;
    }
    std::cout << (failed == 0 ? "ALL TESTS PASSED" : "TESTS FAILED") << " ("
              << succeeded << " ok, " << failed << " failed)\n";
    return failed == 0 ? 0 : 1;
}
