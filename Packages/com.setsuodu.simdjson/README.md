# SimdJSON for Unity (`com.setsuodu.simdjson`)

High-performance **SIMD-accelerated JSON parser** for Unity, powered by [simdjson](https://simdjson.org/).

## Features

- Runtime C# API (`Parser`, `Document`, `Element`): parse / validate / minify, scalars, object & array access, nested fields, `SimdJsonException`
- Native plugin (DOM backend — random-access safe in the Editor)
- Sample + benchmark vs `com.unity.nuget.newtonsoft-json`
- OpenUPM / UPM package layout

## Installation

```bash
openupm add com.setsuodu.simdjson
```

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

## Native plugins — where to put the Release zip

Download from: **https://github.com/setsuodu/simdjson/releases**

### Correct path (important)

```
Packages/com.setsuodu.simdjson/Runtime/Plugins/
```

**Not** `Packages/com.setsuodu.simdjson/Plugins/` (wrong — Editor will not load / may crash).

### After unpacking

```
Runtime/Plugins/
  x86_64/
    simdjson_unity.dll              ← Windows Editor & Standalone
  Android/libs/
    arm64-v8a/libsimdjson_unity.so
    armeabi-v7a/libsimdjson_unity.so
```

1. Select each binary in Unity → Inspector → **Plugin Importer**
2. **Windows dll**
   - Include Platforms: Editor + Standalone
   - CPU: x86_64
   - OS: Windows
3. **Android .so**
   - Include Platforms: Android only (uncheck Editor)
   - CPU: matching ABI (ARM64 / ARMv7)

If the Editor still cannot load the library: Console will show `DllNotFoundException: simdjson_unity`. Fix path / importer, then restart the Editor.

---

## Releases & tags (OpenUPM)

Formal release:

```bash
# bump version in package.json first
git tag com.setsuodu.simdjson/1.0.1
git push origin com.setsuodu.simdjson/1.0.1
```

CI builds Win + Android and attaches assets to [Releases](https://github.com/setsuodu/simdjson/releases).

**Do not** use bare tags (`v1.0.0`, `1.0.0`) — see [ADR-0003](https://github.com/LongLongGames/.github/blob/main/docs/adr/0003-openupm-gittagprefix.md).

OpenUPM metadata only:

```yaml
gitTagPrefix: 'com.setsuodu.simdjson/'
```

---

## Editor crash (fixed on main)

Earlier native code used **ondemand** (single-pass). Calling `ObjectCount()` then `FindField()`, or `GetString()` twice, could hard-crash the Editor.

Current code uses **DOM** so repeated access is safe. **Rebuild / re-download plugins from a new CI run after this fix** — old `v1.0.0` binaries still contain the bug.

```bash
git tag com.setsuodu.simdjson/1.0.1
git push origin com.setsuodu.simdjson/1.0.1
```

---

## Local build

```bash
cd Packages/com.setsuodu.simdjson/Native
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
# copy simdjson_unity.dll or libsimdjson_unity.so into Runtime/Plugins/…
```

## License

- Package wrapper: MIT
- simdjson: Apache-2.0
