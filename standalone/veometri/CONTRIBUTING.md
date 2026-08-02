# Contributing

Configure and test with the commands in the README. Before a PR, run CTest, the independence script, and `format-check` when clang-format is available. Use focused branches and explain behavior, tests, and platform impact in the PR.

Enable `VEOMETRI_WARNINGS_AS_ERRORS` for strict builds. `VEOMETRI_ENABLE_SANITIZERS` enables address/undefined sanitizers only on supported non-Windows GCC/Clang configurations. Do not force these policies onto fetched code.

Preserve dependency direction: core must remain graphics-independent, I/O may depend on core, and runtime/UI may depend on both. Core and I/O tests must remain headless and must not link graphics libraries. Avoid unrelated formatting; use the checked-in clang-format configuration for touched C/C++.
