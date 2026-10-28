# tests/CMakeLists.txt — frozen suite (T-001..T-013) + benchmark harness (T-050/T-051).

find_package (Python3 COMPONENTS Interpreter REQUIRED)

set (GRIMAMP_TEST_DATA "${CMAKE_CURRENT_SOURCE_DIR}")
set (GRIMAMP_FUZZ_CORPUS "${CMAKE_CURRENT_BINARY_DIR}/fuzz_corpus")

# Both harnesses link the plugin's SharedCode library so the JUCE module
# objects (and BinaryData) exist exactly once — compiling module sources into
# the test targets as well would duplicate every symbol at link time.

if (NOT TARGET GrimAmp)
    message (FATAL_ERROR "GrimAmp target missing; tests link against its shared-code archive")
endif()

# ---------------------------------------------------------------------------
# Unit test runner (grimamp_tests)
# ---------------------------------------------------------------------------
add_executable (grimamp_tests
    TestMain.cpp
    UnitTests.cpp
)

target_include_directories (grimamp_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Source"
)

target_compile_definitions (grimamp_tests PRIVATE
    GRIMAMP_TEST_DATA_DIR="${GRIMAMP_TEST_DATA}"
)

target_link_libraries (grimamp_tests PRIVATE
    GrimAmp
    grimamp_juce_headers
)
add_dependencies (grimamp_tests GrimAmpBinaryData)

# ---------------------------------------------------------------------------
# Benchmark / render harness (grimamp_bench) — T-050, T-051, S-009 renders
# ---------------------------------------------------------------------------
add_executable (grimamp_bench
    bench/BenchMain.cpp
)

target_include_directories (grimamp_bench PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Source"
)

target_compile_definitions (grimamp_bench PRIVATE
    GRIMAMP_TEST_DATA_DIR="${GRIMAMP_TEST_DATA}"
)

target_link_libraries (grimamp_bench PRIVATE
    GrimAmp
    grimamp_juce_headers
)
add_dependencies (grimamp_bench GrimAmpBinaryData)

# ---------------------------------------------------------------------------
# ctest wiring
# ---------------------------------------------------------------------------
set (GRIMAMP_UNIT_TESTS
    skeleton param_inventory param_ranges silence sine_sanity harmonics
    denormals preset_roundtrip state_restore smoothing gestures ir_loading
)

foreach (t ${GRIMAMP_UNIT_TESTS})
    add_test (NAME ${t} COMMAND grimamp_tests ${t})
endforeach()

# T-013 fuzz corpus: generate 1000 mutated presets (seed 42, checked-in script)
add_test (
    NAME fuzz_corpus_gen
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_preset.py"
            --seed 42 --count 1000 --out-dir "${GRIMAMP_FUZZ_CORPUS}"
            --seed-preset "${CMAKE_SOURCE_DIR}/Resources/presets/preset01.xml"
)
set_tests_properties (fuzz_corpus_gen PROPERTIES FIXTURES_SETUP fuzz_corpus)

add_test (NAME fuzz_presets COMMAND grimamp_tests fuzz_presets)
set_tests_properties (fuzz_presets PROPERTIES
    ENVIRONMENT "GRIMAMP_FUZZ_DIR=${GRIMAMP_FUZZ_CORPUS}"
    FIXTURES_REQUIRED fuzz_corpus
)

# T-050/T-051: benchmark job (informational locally; the regression gate lives
# in CI where the runner class is pinned)
add_test (NAME bench COMMAND grimamp_bench --bench --out "${CMAKE_CURRENT_BINARY_DIR}/bench-results.json")
set_tests_properties (bench PROPERTIES TIMEOUT 900)
