# Repository migration

Choose whether history will be preserved with a path-filtered export or restarted with documented provenance before moving code.

1. Copy this directory's contents to the new repository root.
2. Move `ci/mesh-sculpt-ci.yml` to `.github/workflows/mesh-sculpt-ci.yml`.
3. Change nested CI source paths to `cmake -S . -B build`.
4. Resolve and verify license status.
5. Choose the final product name.
6. Complete `RENAMING_CHECKLIST.md`.
7. Configure a clean build.
8. Run all tests.
9. Run the independence check.
10. Run clean-copy verification.
11. Install to a staging prefix.
12. Run CPack.
13. Verify build-tree and installed runtime assets.
14. Add the repository description and topics.
15. Tag the first prerelease only after clean-clone release verification.

Do not create the repository or remove the existing Maze3D staging copy as part of this preparation change.
