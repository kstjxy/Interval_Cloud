# Interval Cloud (Falcor Fork, Frozen)

This repository is a **standalone fork of NVIDIA’s Falcor** engine, frozen for a short-lived class project.  
We won’t sync upstream changes. Collaborators should follow the steps below to build on Windows.

## Quick Start (Windows, VS2022)

1. Install **Visual Studio 2022** with the **Desktop development with C++** workload.  
   Also install: **MSVC v143**, **C++ CMake tools for Windows**, **Windows 10/11 SDK**.
2. (Optional but recommended) Install **Graphics Tools** (Windows Optional Features) and **PIX on Windows**.
3. Clone this repo, then in a **Developer Command Prompt for VS 2022**:

   ```bat
   git clone <THIS_REPO_URL>
   cd <REPO_DIR>
   bootstrap_and_build.bat
```

4. Open `build/windows-vs2022/Falcor.sln` in Visual Studio and run a sample/tool (e.g., RenderGraphEditor) or, later, our `IntervalCloudSample` when added.

## What’s in here?

* **Falcor engine** source, scripts, and tools.
* A **frozen branch** (e.g., `interval-cloud-freeze`) that pins a known-good Falcor commit.
* Our project code will live under `Source/Samples/IntervalCloudSample/` (added in a later commit).

## Collaboration Rules

* Work only on the **frozen branch** (e.g., `interval-cloud-freeze`).
* **Do not** fetch/merge from NVIDIA upstream.
* **Do not** commit anything under `build/` (already gitignored).
* If you add large assets later, use Git LFS or provide a `fetch_assets.bat`.

## Troubleshooting

* **Missing SDK or MSVC**: Re-run *Visual Studio Installer* → add **Desktop development with C++**, **Windows 10/11 SDK**, **C++ CMake tools**.
* **Submodule errors**: Ensure `git` is installed and run `git submodule update --init --recursive`.
* **DX12 debug layer warnings**: Install *Graphics Tools* (Settings → System → Optional features → Add a feature → Graphics Tools).
* **Build fails in Release**: Try `Debug` configuration in VS, then switch back to `Release`.
* **Fresh build**: Delete the `build/` folder and re-run `bootstrap_and_build.bat`.

## Notes

* This repo is intentionally *frozen*. If we ever need a Falcor bump, we’ll do it on a separate branch and migrate our sample.
* License terms follow Falcor’s license. See below for the original license text.
