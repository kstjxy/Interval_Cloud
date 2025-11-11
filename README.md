# Interval Cloud (Falcor Fork)

Fork of NVIDIA Falcor for the Interval Cloud class project. We build only on Windows/VS2022.

## Quick Start (Windows + VS2022)

1. Install Visual Studio 2022 with **Desktop development with C++**, **MSVC v143**, **C++ CMake tools**, and the **Windows 10/11 SDK** (add Graphics Tools + PIX for DX12 debugging).
2. Clone the repo, then in a **VS2022 Developer Command Prompt** run:
   ```bat
   git clone <THIS_REPO_URL>
   cd <REPO_DIR>
   bootstrap_and_build.bat
   ```
3. Open `build/windows-vs2022/Falcor.sln` and run `IntervalCloudSample` (or any Falcor tool).

## Workflow Essentials

- `bootstrap_and_build.bat` = init submodules + CMake configure/generate + Release build.
- Re-run the script whenever you add/rename/remove source or shader files, touch `CMakeLists.txt`, or add a new pass/sample. Editing existing files only → regular rebuild is enough.
- Clean build: delete `build/` and re-run the script.

### Common Tasks

| Task | Files | Extra steps |
| --- | --- | --- |
| Edit existing `.cpp/.h/.slang` | `Source/...` | Rebuild in VS. |
| Add/rename/remove `.cpp/.h` | `Source/...`, `Source/Samples/IntervalCloudSample/CMakeLists.txt` | Update `target_sources`, re-run script, rebuild. |
| Add shaders | `Source/...` | Re-run script (recommended) so copy paths regenerate, then rebuild. |
| Add new pass/sample | New dir + parent `CMakeLists.txt` | Wire up subdir, re-run script, rebuild. |

### Shader Hot Reload

Editing an existing `.slang` usually hot-reloads while the sample runs. If you add files or change paths, rebuild and re-run the bootstrap script to refresh copied assets.

## Repo Rules

- No merges from NVIDIA upstream.
- Never commit `build/`.
- Large future assets go through Git LFS or a fetch script.

## Troubleshooting

- Missing SDK/MSVC: reopen Visual Studio Installer and add the required workloads.
- Submodule errors: `git submodule update --init --recursive`.
- DX12 debug warnings: install Windows “Graphics Tools” optional feature.
- Build woes: try Debug first, or nuke `build/` and rerun `bootstrap_and_build.bat`.

Falcor’s original license applies; see the license file in the repo.
