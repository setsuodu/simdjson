# SimdJSON for Unity

Unity UPM package: **`com.setsuodu.simdjson`** — high-performance SIMD JSON parser based on [simdjson](https://simdjson.org/).

| | |
|---|---|
| Package | [`Packages/com.setsuodu.simdjson`](Packages/com.setsuodu.simdjson) |
| Package README | [Packages/com.setsuodu.simdjson/README.md](Packages/com.setsuodu.simdjson/README.md) |
| Native CI | [`.github/workflows/build-native.yml`](.github/workflows/build-native.yml) |
| Releases (plugins) | [github.com/setsuodu/simdjson/releases](https://github.com/setsuodu/simdjson/releases) |

## Install

```bash
openupm add com.setsuodu.simdjson
```

## Publish a version (OpenUPM + GitHub Release)

1. Bump `version` in [`Packages/com.setsuodu.simdjson/package.json`](Packages/com.setsuodu.simdjson/package.json).
2. Tag with the **package-name prefix** (required by [ADR-0003](https://github.com/LongLongGames/.github/blob/main/docs/adr/0003-openupm-gittagprefix.md)):

```bash
git tag com.setsuodu.simdjson/v1.0.0
git push origin com.setsuodu.simdjson/v1.0.0
```

3. CI builds Windows + Android natives and attaches them to a [GitHub Release](https://github.com/setsuodu/simdjson/releases) for that tag.

**Do not** use bare tags like `1.0.0` or `v1.0.0` — OpenUPM will mis-scan them.

## Native plugins (current CI)

| Platform | Output |
|----------|--------|
| Windows x86_64 | `simdjson_unity.dll` |
| Android arm64-v8a | `libsimdjson_unity.so` |
| Android armeabi-v7a | `libsimdjson_unity.so` |

Download from [Releases](https://github.com/setsuodu/simdjson/releases) (`simdjson-native-plugins-*.zip` or individual files) and place under `Runtime/Plugins/`.

Details, local build, and sample API: see the [package README](Packages/com.setsuodu.simdjson/README.md).
