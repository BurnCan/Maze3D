# Repository migration

The intended standalone repository is `BurnCan/Veometri`. Preserve history with a path-filtered export or restart it with documented provenance before moving this directory's contents to the new repository root.

1. Create `BurnCan/Veometri` only after license status is resolved.
2. Copy this directory's contents to the new repository root.
3. Move `ci/veometri-ci.yml` to `.github/workflows/veometri-ci.yml`.
4. Replace nested commands such as `cmake -S standalone/veometri -B build/veometri` with root commands such as `cmake -S . -B build`.
5. Run configure, tests, the identity and independence audits, and clean-copy verification.
6. Verify install and CPack output, including build-tree and installed runtime assets.
7. Tag the first prerelease only after clean-clone release verification.
8. Remove or deprecate the staging copy in Maze3D after migration is complete.

Do not create the repository or remove the existing Maze3D staging copy as part of this identity change.
