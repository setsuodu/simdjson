# SimdJSON for Unity (`com.setsuodu.simdjson`)

High-performance **SIMD-accelerated JSON parser** for Unity, powered by the excellent [simdjson](https://simdjson.org/) C++ library.

## Features

- Full runtime C# API (`Parser`, `Document`, `Element`) covering:
  - Parse (string / UTF-8 bytes)
  - Validate
  - Minify
  - Scalar getters: `GetBool`, `GetInt64`, `GetUInt64`, `GetDouble`, `GetString`
  - Object: `ObjectCount`, `FindField`, `TryGetObjectAt`, indexer
  - Array: `ArrayCount`, `ArrayAt`, `TryArrayAt`, indexer
  - Nested access
  - Proper error codes via `SimdJsonException`
- Native plugin built from official simdjson single-header (v4.6.x)
- Sample scene script that **demos every method** and runs a **benchmark vs `com.unity.nuget.newtonsoft-json`** (AOT-ready)
- OpenUPM / UPM ready package layout

## Installation (OpenUPM)

```bash
openupm add com.setsuodu.simdjson
```

Or add the scoped registry and dependency manually in `Packages/manifest.json`.

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

## Releasing / Git tags (OpenUPM)

Per [ADR-0003](https://github.com/LongLongGames/.github/blob/main/docs/adr/0003-openupm-gittagprefix.md):

| Purpose | Tag format | Consumer |
|---------|------------|----------|
| **This Unity package** | `com.setsuodu.simdjson/<semver>` | OpenUPM |
| Example | `com.setsuodu.simdjson/1.0.0` | |

**Rules**

- Package tags **must** use the package name + `/` prefix.
- **Do not** create bare tags (`1.0.0`, `v1.0.0`) — OpenUPM scans all tags and will mis-ingest them.
- Do **not** put `gitTagPrefix` in `package.json` (not a Unity field). Set it only in OpenUPM metadata:

  ```yaml
  # openupm/openupm → data/packages/com.setsuodu.simdjson.yml
  name: com.setsuodu.simdjson
  gitTagPrefix: 'com.setsuodu.simdjson/'
  ```

- `package.json` `version` must match the version segment of the tag.

```bash
# Release example
git tag com.setsuodu.simdjson/1.0.0
git push origin com.setsuodu.simdjson/1.0.0
```

CI runs on those tags and attaches multi-platform native plugins to the GitHub Release.

## Building the Native Plugin

### CI (recommended)

Workflow [`.github/workflows/build-native.yml`](../../.github/workflows/build-native.yml) builds:

| Platform | Arch     | Output                        |
|----------|----------|-------------------------------|
| Linux    | x86_64   | `libsimdjson_unity.so`        |
| Linux    | arm64    | `libsimdjson_unity.so`        |
| Windows  | x86_64   | `simdjson_unity.dll`          |
| macOS    | arm64    | `libsimdjson_unity.dylib`     |
| macOS    | x86_64   | `libsimdjson_unity.dylib`     |

Triggers:

- Push/PR touching `Native/`
- Tags matching `com.setsuodu.simdjson/*`
- Manual: Actions → *Build Native Plugins* (optional **commit_plugins**)

### Local build

```bash
cd Packages/com.setsuodu.simdjson/Native
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
# macOS arm64:
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build . --config Release
```

Copy into `Runtime/Plugins/` and set Unity import settings (CPU / OS):

```
Runtime/Plugins/
  x86_64/
    libsimdjson_unity.so      # Linux
    simdjson_unity.dll        # Windows
  ARM64/
    libsimdjson_unity.so      # Linux arm64
  macOS/
    arm64/libsimdjson_unity.dylib
    x86_64/libsimdjson_unity.dylib
```

> **Note**: simdjson requires a 64-bit platform and a C++17 compiler. On-demand API is single-pass; keep the `Document` alive while using `Element`s.

## Benchmark

The sample (`Samples~/SimdJsonDemo.cs`) compares parse + light traversal against Newtonsoft.Json (`JToken.Parse`). On typical desktop hardware you can expect **several times** higher throughput for large payloads thanks to SIMD.

## License

- This Unity package wrapper: MIT
- simdjson: Apache-2.0 (see upstream)

## Credits

- [simdjson](https://github.com/simdjson/simdjson) by Daniel Lemire et al.
- Unity package structure designed for OpenUPM distribution.
