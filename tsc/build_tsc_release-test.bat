pushd
cd ../__build/tsc-release
cmake --build . --config Release -j 24
set CTEST_OUTPUT_ON_FAILURE=TRUE
cmake --build . --target RUN_TESTS --config Release -j 24
popd