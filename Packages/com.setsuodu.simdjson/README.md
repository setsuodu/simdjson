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

## Building the Native Plugin

### CI (recommended)

GitHub Actions workflow [`.github/workflows/build-native.yml`](../../.github/workflows/build-native.yml) builds for:

| Platform | Arch     | Output                        |
|----------|----------|-------------------------------|
| Linux    | x86_64   | `libsimdjson_unity.so`        |
| Linux    | arm64    | `libsimdjson_unity.so`        |
| Windows  | x86_64   | `simdjson_unity.dll`          |
| macOS    | arm64    | `libsimdjson_unity.dylib`     |
| macOS    | x86_64   | `libsimdjson_unity.dylib`     |

- **Automatic** on push/PR that touch `Native/`
- **Manual**: Actions → *Build Native Plugins* → Run workflow
  - Optional: check **commit_plugins** to push binaries back into `Runtime/Plugins/`

Artifacts are uploaded as `native-<platform>-<arch>` and a combined `unity-plugins-all`.

### Local build

```bash
cd Packages/com.setsuodu.simdjson/Native
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
# macOS arm64 example:
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build . --config Release
```

Copy the resulting shared library into the matching folder under `Runtime/Plugins/` and set Unity plugin import settings (CPU / OS).

Suggested layout:

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
