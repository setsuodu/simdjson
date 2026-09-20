# SimdJSON for Unity Documentation

## Runtime API Overview

| Class | Purpose |
|-------|---------|
| `Parser` | Create once (or per-thread). Call `Parse` / static `TryValidate` / `Minify`. |
| `Document` | Owns the parsed document. Call `GetRoot()`. Dispose when done. |
| `Element` | A JSON value. Supports type checks and typed getters. Dispose when done. |

## Important lifetime rules (on-demand)

simdjson On-Demand is a **single-pass** design for maximum speed:

1. Keep the `Document` alive while any `Element` derived from it is used.
2. Prefer sequential field/array access; re-finding the same field after consuming later siblings may fail with `OutOfOrderIteration` under development checks.
3. Dispose every `Element` and the `Document` (or use `using`).

## Platform support

| Platform | Status |
|----------|--------|
| Linux x86_64 | Prebuilt `.so` included |
| Windows / macOS / Android / iOS | Build from `Native/` with CMake |

## Benchmark notes

The sample measures **parse + light field access**. Pure parse throughput of simdjson can exceed several GB/s on modern CPUs; the C# interop and managed string conversions add overhead, but the package still typically outperforms managed Newtonsoft.Json on large documents.
