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

Prebuilt `libsimdjson_unity.so` (Linux x86_64) is included under `Runtime/Plugins/x86_64/`.

For other platforms (Windows, macOS, Android, iOS):

```bash
cd Native
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Then copy the resulting shared library into the appropriate `Runtime/Plugins/<platform>/` folder and set Unity plugin import settings (CPU, OS).

> **Note**: simdjson requires a 64-bit platform and a C++17 compiler. On-demand API is single-pass; keep the `Document` alive while using `Element`s.

## Benchmark

The sample (`Samples~/SimdJsonDemo.cs`) compares parse + light traversal against Newtonsoft.Json (`JToken.Parse`). On typical desktop hardware you can expect **several times** higher throughput for large payloads thanks to SIMD.

## License

- This Unity package wrapper: MIT
- simdjson: Apache-2.0 (see upstream)

## Credits

- [simdjson](https://github.com/setsuodu/simdjson) by Daniel Lemire et al.
- Unity package structure designed for OpenUPM distribution.
