# SimdJSON for Unity (`com.setsuodu.simdjson`)

High-performance **SIMD-accelerated JSON parser** for Unity, powered by [simdjson](https://simdjson.org/).

## Features

- Runtime C# API (`Parser`, `Document`, `Element`): parse / validate / minify, scalars, object & array access, nested fields, `SimdJsonException`
- Native plugin from official simdjson single-header
- Sample + benchmark vs `com.unity.nuget.newtonsoft-json` (AOT-ready)
- OpenUPM / UPM package layout

## Installation

```bash
openupm add com.setsuodu.simdjson
```

Or add the OpenUPM scoped registry and dependency in `Packages/manifest.json`.

## Quick Start

```csharp
using SimdJson;

using (var parser = new Parser())
using (var doc = parser.Parse("{\"hello\":42}"))
using (var root = doc.GetRoot())
using (var hello = root.FindField("hello"))
{
    Debug.Log(hello.GetInt64()); // 42
}
```

---

## Releases & downloading native plugins

Built plugins are published to:

**https://github.com/setsuodu/simdjson/releases**

Each formal release includes:

| Asset | Contents |
|-------|----------|
| `simdjson-native-plugins-<ver>.zip` | Full `Runtime/Plugins` tree |
| `simdjson_unity.dll` | Windows x64 |
| `libsimdjson_unity.so` (arm64-v8a) | Android 64-bit |
| `libsimdjson_unity.so` (armeabi-v7a) | Android 32-bit |

Unpack the zip into the package’s `Runtime/Plugins/` (or copy individual files into the paths below), then in Unity set plugin import settings (CPU / OS / Android ABI).

### Plugin layout

```
Runtime/Plugins/
  x86_64/
    simdjson_unity.dll              # Windows
  Android/libs/
    arm64-v8a/libsimdjson_unity.so
    armeabi-v7a/libsimdjson_unity.so
```

---

## How a GitHub Release is created

### Formal release (OpenUPM + Releases page)

1. Set `version` in this package’s `package.json` (e.g. `1.0.0`).
2. Push a **namespaced** tag only:

```bash
git tag com.setsuodu.simdjson/1.0.0
git push origin com.setsuodu.simdjson/1.0.0
```

3. Workflow [Build Native Plugins](../../.github/workflows/build-native.yml) runs, builds Win + Android, and creates a [GitHub Release](https://github.com/setsuodu/simdjson/releases) for that tag with the zip + binaries.

### Manual / test build (no OpenUPM version)

1. GitHub → **Actions** → **Build Native Plugins** → **Run workflow**
2. Optional:
   - **commit_plugins** — write binaries back into `Runtime/Plugins` on `main`
   - **publish_prerelease** — attach zip to a **pre-release** named `native-ci-<sha>` (not a package version tag)

Ordinary pushes only upload **Actions artifacts** (30-day retention); they do **not** appear on the Releases page.

---

## Git tags (OpenUPM) — required naming

Per [ADR-0003](https://github.com/LongLongGames/.github/blob/main/docs/adr/0003-openupm-gittagprefix.md):

| Use | Tag format | Example |
|-----|------------|---------|
| This Unity package | `com.setsuodu.simdjson/<semver>` | `com.setsuodu.simdjson/1.0.0` |

**Rules**

- Tag **must** start with package name + `/`.
- **Forbidden**: bare `1.0.0`, `v1.0.0` — OpenUPM scans all tags and will mis-ingest them.
- Do **not** put `gitTagPrefix` in `package.json`. Configure it only in OpenUPM metadata:

```yaml
# openupm/openupm → data/packages/com.setsuodu.simdjson.yml
name: com.setsuodu.simdjson
gitTagPrefix: 'com.setsuodu.simdjson/'
```

- `package.json` `version` must equal the version segment of the tag.

---

## Native CI (current matrix)

| Platform | Runner | Output |
|----------|--------|--------|
| Windows x86_64 | `windows-latest` | `simdjson_unity.dll` |
| Android arm64-v8a | Ubuntu + NDK | `libsimdjson_unity.so` |
| Android armeabi-v7a | Ubuntu + NDK | `libsimdjson_unity.so` |

Linux / macOS jobs are disabled for now (can be re-enabled in the workflow when needed). Avoid `macos-13` (Intel) — runner queue is often hours long.

**Triggers:** push/PR on `Native/**`, tags `com.setsuodu.simdjson/*`, or manual `workflow_dispatch`.

### Local build

```bash
cd Packages/com.setsuodu.simdjson/Native
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Android (example arm64-v8a):

```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24 \
  -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

> simdjson needs 64-bit + C++17. On-demand API is single-pass; keep `Document` alive while using `Element`s.

---

## Benchmark

Sample (`Samples~/SimdJsonDemo.cs`) compares parse + light traversal against Newtonsoft.Json. Large payloads are typically several times faster thanks to SIMD.

## License

- This Unity package wrapper: MIT
- simdjson: Apache-2.0 (upstream)

## Credits

- [simdjson](https://github.com/simdjson/simdjson) by Daniel Lemire et al.
