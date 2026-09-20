using System;
using System.Diagnostics;
using System.Text;
using UnityEngine;
using SimdJson;
using Debug = UnityEngine.Debug;

#if HAS_NEWTONSOFT
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
#endif

/// <summary>
/// Sample that demonstrates ALL runtime API methods of SimdJSON for Unity,
/// and optionally runs a benchmark against AOT-optimized com.unity.nuget.newtonsoft-json
/// when that package is present (define HAS_NEWTONSOFT via asmdef versionDefines).
/// Attach to any GameObject and press Play, or call RunAll() from code.
/// </summary>
public class SimdJsonDemo : MonoBehaviour
{
    [Header("Benchmark Settings")]
    [Tooltip("Number of iterations for benchmark")]
    public int BenchmarkIterations = 500;

    [Tooltip("Use a larger synthetic payload for stress test")]
    public bool UseLargePayload = true;

    void Start()
    {
        RunAll();
    }

    [ContextMenu("Run All Demos & Benchmark")]
    public void RunAll()
    {
        Debug.Log("========== SimdJSON for Unity – Full API Demo ==========");
        Debug.Log($"Native library version: {Parser.Version}");

        DemoValidate();
        DemoMinify();
        DemoParseAndScalars();
        DemoObject();
        DemoArray();
        DemoNested();
        DemoErrorHandling();

        Debug.Log("========== Benchmark vs Newtonsoft.Json ==========");
        RunBenchmark();
    }

    // ------------------------------------------------------------------
    // 1. Validate
    // ------------------------------------------------------------------
    void DemoValidate()
    {
        Debug.Log("--- Validate ---");
        string good = "{\"ok\":true,\"n\":42}";
        string bad = "{\"ok\":true,\"n\":42"; // missing }

        if (Parser.TryValidate(good, out var err1))
            Debug.Log("Validate good JSON: OK");
        else
            Debug.LogError($"Unexpected fail: {err1}");

        if (!Parser.TryValidate(bad, out var err2))
            Debug.Log($"Validate bad JSON: correctly rejected ({err2})");
        else
            Debug.LogError("Should have failed");
    }

    // ------------------------------------------------------------------
    // 2. Minify
    // ------------------------------------------------------------------
    void DemoMinify()
    {
        Debug.Log("--- Minify ---");
        string pretty = "{\n  \"name\" : \"alice\" ,\n  \"age\" : 30\n}";
        string min = Parser.Minify(pretty);
        Debug.Log($"Original length={pretty.Length}, minified=\"{min}\" (len={min.Length})");
    }

    // ------------------------------------------------------------------
    // 3. Parse + scalar getters
    // ------------------------------------------------------------------
    void DemoParseAndScalars()
    {
        Debug.Log("--- Parse & Scalars ---");
        string json = @"{
            ""nullVal"": null,
            ""boolVal"": true,
            ""intVal"": -1234567890123,
            ""uintVal"": 18446744073709551615,
            ""doubleVal"": 3.141592653589793,
            ""strVal"": ""hello 世界 simd""
        }";

        using (var parser = new Parser())
        using (var doc = parser.Parse(json))
        using (var root = doc.GetRoot())
        {
            Debug.Log($"Root type: {root.Type}");

            using (var e = root.FindField("nullVal"))
            {
                Debug.Log($"nullVal type={e.Type}, IsNull={e.IsNull}");
            }

            using (var e = root.FindField("boolVal"))
            {
                Debug.Log($"boolVal = {e.GetBool()}");
            }

            using (var e = root.FindField("intVal"))
            {
                Debug.Log($"intVal = {e.GetInt64()}");
            }

            using (var e = root.FindField("uintVal"))
            {
                try { Debug.Log($"uintVal (as double) = {e.GetDouble()}"); }
                catch (Exception ex) { Debug.Log($"uintVal note: {ex.Message}"); }
            }

            using (var e = root.FindField("doubleVal"))
            {
                Debug.Log($"doubleVal = {e.GetDouble()}");
            }

            using (var e = root.FindField("strVal"))
            {
                Debug.Log($"strVal = \"{e.GetString()}\"");
            }
        }
    }

    // ------------------------------------------------------------------
    // 4. Object iteration
    // ------------------------------------------------------------------
    void DemoObject()
    {
        Debug.Log("--- Object ---");
        string json = @"{""a"":1,""b"":""two"",""c"":true}";

        using (var parser = new Parser())
        using (var doc = parser.Parse(json))
        using (var root = doc.GetRoot())
        {
            int count = root.ObjectCount();
            Debug.Log($"Object field count = {count}");

            for (int i = 0; i < count; i++)
            {
                if (root.TryGetObjectAt(i, out string key, out Element val))
                {
                    using (val)
                    {
                        string desc = val.Type switch
                        {
                            SimdJsonType.Number => val.GetInt64().ToString(),
                            SimdJsonType.String => "\"" + val.GetString() + "\"",
                            SimdJsonType.Bool => val.GetBool().ToString(),
                            _ => val.Type.ToString()
                        };
                        Debug.Log($"  [{i}] {key} = {desc}");
                    }
                }
            }

            using (var a = root.FindField("a"))
            {
                Debug.Log($"FindField(\"a\") => {a?.GetInt64()}");
            }
            var missing = root.FindField("nope");
            Debug.Log($"FindField(\"nope\") => {(missing == null ? "null (expected)" : "found?")}");
            missing?.Dispose();
        }
    }

    // ------------------------------------------------------------------
    // 5. Array
    // ------------------------------------------------------------------
    void DemoArray()
    {
        Debug.Log("--- Array ---");
        string json = @"[10, 20.5, ""three"", false, null]";

        using (var parser = new Parser())
        using (var doc = parser.Parse(json))
        using (var root = doc.GetRoot())
        {
            int n = root.ArrayCount();
            Debug.Log($"Array length = {n}");

            for (int i = 0; i < n; i++)
            {
                using (var e = root.ArrayAt(i))
                {
                    string s = e.Type switch
                    {
                        SimdJsonType.Number => e.GetDouble().ToString(),
                        SimdJsonType.String => "\"" + e.GetString() + "\"",
                        SimdJsonType.Bool => e.GetBool().ToString(),
                        SimdJsonType.Null => "null",
                        _ => e.Type.ToString()
                    };
                    Debug.Log($"  [{i}] = {s}");
                }
            }

            using (var e = root[2])
                Debug.Log($"root[2] = \"{e.GetString()}\"");
        }
    }

    // ------------------------------------------------------------------
    // 6. Nested structure
    // ------------------------------------------------------------------
    void DemoNested()
    {
        Debug.Log("--- Nested ---");
        string json = @"{
            ""user"": {
                ""id"": 42,
                ""tags"": [""admin"", ""dev""],
                ""profile"": { ""score"": 99.5 }
            }
        }";

        using (var parser = new Parser())
        using (var doc = parser.Parse(json))
        using (var root = doc.GetRoot())
        using (var user = root["user"])
        {
            using (var id = user["id"])
                Debug.Log($"user.id = {id.GetInt64()}");

            using (var tags = user["tags"])
            {
                Debug.Log($"user.tags count = {tags.ArrayCount()}");
                using (var t0 = tags[0])
                    Debug.Log($"  tags[0] = {t0.GetString()}");
            }

            using (var profile = user["profile"])
            using (var score = profile["score"])
                Debug.Log($"user.profile.score = {score.GetDouble()}");
        }
    }

    // ------------------------------------------------------------------
    // 7. Error handling
    // ------------------------------------------------------------------
    void DemoErrorHandling()
    {
        Debug.Log("--- Error Handling ---");
        try
        {
            using (var parser = new Parser())
            using (var doc = parser.Parse("{bad"))
            { }
        }
        catch (SimdJsonException ex)
        {
            Debug.Log($"Caught expected parse error: {ex.ErrorCode} – {ex.Message}");
        }

        try
        {
            using (var parser = new Parser())
            using (var doc = parser.Parse("{\"x\":1}"))
            using (var root = doc.GetRoot())
            using (var x = root["x"])
            {
                x.GetString(); // wrong type
            }
        }
        catch (SimdJsonException ex)
        {
            Debug.Log($"Caught expected type error: {ex.ErrorCode}");
        }
    }

    // ------------------------------------------------------------------
    // Benchmark vs Newtonsoft.Json (optional)
    // ------------------------------------------------------------------
    void RunBenchmark()
    {
        string payload = UseLargePayload ? BuildLargeJson() : BuildSmallJson();
        int iterations = BenchmarkIterations;
        Debug.Log($"Payload size: {Encoding.UTF8.GetByteCount(payload)} bytes, iterations: {iterations}");

#if HAS_NEWTONSOFT
        // Warm-up
        for (int i = 0; i < 10; i++)
        {
            WarmSimd(payload);
            WarmNewtonsoft(payload);
        }

        // SimdJSON
        var sw = Stopwatch.StartNew();
        long checksum1 = 0;
        for (int i = 0; i < iterations; i++)
            checksum1 += BenchSimd(payload);
        sw.Stop();
        double simdMs = sw.Elapsed.TotalMilliseconds;
        double simdPer = simdMs / iterations;

        // Newtonsoft
        sw.Restart();
        long checksum2 = 0;
        for (int i = 0; i < iterations; i++)
            checksum2 += BenchNewtonsoft(payload);
        sw.Stop();
        double nsMs = sw.Elapsed.TotalMilliseconds;
        double nsPer = nsMs / iterations;

        Debug.Log($"SimdJSON total: {simdMs:F2} ms  ({simdPer:F4} ms/op)  checksum={checksum1}");
        Debug.Log($"Newtonsoft total: {nsMs:F2} ms  ({nsPer:F4} ms/op)  checksum={checksum2}");
        if (nsPer > 0)
            Debug.Log($"Speedup: {nsPer / simdPer:F2}x faster than Newtonsoft.Json");
        else
            Debug.Log("Speedup: N/A");
#else
        // Only time SimdJSON; remind user to install Newtonsoft for comparison
        for (int i = 0; i < 10; i++)
            WarmSimd(payload);

        var sw = Stopwatch.StartNew();
        long checksum1 = 0;
        for (int i = 0; i < iterations; i++)
            checksum1 += BenchSimd(payload);
        sw.Stop();
        double simdMs = sw.Elapsed.TotalMilliseconds;
        double simdPer = simdMs / iterations;

        Debug.Log($"SimdJSON total: {simdMs:F2} ms  ({simdPer:F4} ms/op)  checksum={checksum1}");
        Debug.LogWarning(
            "Benchmark vs Newtonsoft skipped: package com.unity.nuget.newtonsoft-json is not installed.\n" +
            "Install it (Package Manager or OpenUPM) to enable the comparison. API demos above still work.");
#endif
    }

    static void WarmSimd(string json)
    {
        using (var p = new Parser())
        using (var d = p.Parse(json))
        using (var r = d.GetRoot())
        {
            if (r.IsObject) { var c = r.ObjectCount(); }
            else if (r.IsArray) { var c = r.ArrayCount(); }
        }
    }

    static long BenchSimd(string json)
    {
        long sum = 0;
        using (var p = new Parser())
        using (var d = p.Parse(json))
        using (var r = d.GetRoot())
        {
            if (r.IsObject)
            {
                int n = r.ObjectCount();
                sum += n;
                using (var e = r.FindField("id")) if (e != null) sum += e.GetInt64();
            }
            else if (r.IsArray)
            {
                int n = r.ArrayCount();
                sum += n;
                if (n > 0)
                {
                    using (var e = r.ArrayAt(0))
                        if (e.IsNumber) sum += (long)e.GetDouble();
                }
            }
        }
        return sum;
    }

#if HAS_NEWTONSOFT
    static void WarmNewtonsoft(string json)
    {
        var tok = JToken.Parse(json);
        _ = tok.Type;
    }

    static long BenchNewtonsoft(string json)
    {
        long sum = 0;
        var tok = JToken.Parse(json);
        if (tok is JObject obj)
        {
            sum += obj.Count;
            if (obj.TryGetValue("id", out var id)) sum += id.Value<long>();
        }
        else if (tok is JArray arr)
        {
            sum += arr.Count;
            if (arr.Count > 0 && arr[0].Type == JTokenType.Integer)
                sum += arr[0].Value<long>();
            else if (arr.Count > 0 && arr[0].Type == JTokenType.Float)
                sum += (long)arr[0].Value<double>();
        }
        return sum;
    }
#endif

    static string BuildSmallJson()
    {
        return @"{""id"":12345,""name"":""player"",""score"":98.5,""active"":true,""tags"":[""a"",""b"",""c""]}";
    }

    static string BuildLargeJson()
    {
        var sb = new StringBuilder(64 * 1024);
        sb.Append("{\"id\":1,\"items\":[");
        for (int i = 0; i < 500; i++)
        {
            if (i > 0) sb.Append(',');
            sb.Append("{\"i\":").Append(i)
              .Append(",\"n\":\"item_").Append(i).Append("\"")
              .Append(",\"v\":").Append(i * 1.5)
              .Append(",\"ok\":").Append(i % 2 == 0 ? "true" : "false")
              .Append('}');
        }
        sb.Append("]}");
        return sb.ToString();
    }
}