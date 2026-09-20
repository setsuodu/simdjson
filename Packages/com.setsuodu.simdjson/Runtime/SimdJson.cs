using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

namespace SimdJson
{
    /// <summary>
    /// Error codes returned by the native simdjson library.
    /// </summary>
    public enum SimdJsonError
    {
        Ok = 0,
        Capacity = 1,
        MemAlloc = 2,
        TapeError = 3,
        DepthError = 4,
        StringError = 5,
        TAtomError = 6,
        FAtomError = 7,
        NAtomError = 8,
        NumberError = 9,
        Utf8Error = 10,
        Uninitialized = 11,
        Empty = 12,
        UnescapedChars = 13,
        UnexpectedError = 14,
        ParserInUse = 15,
        InsufficientPadding = 16,
        IncompleteArrayOrObject = 17,
        ScalarDocumentAsValue = 18,
        OutOfOrderIteration = 19,
        ObjectIterError = 20,
        FindError = 21,
        NoSuchField = 22,
        IndexOutOfBounds = 23,
        IncorrectType = 24,
        NumberOutOfRange = 25,
        Invalid = 26
    }

    /// <summary>
    /// JSON value type.
    /// </summary>
    public enum SimdJsonType
    {
        Null = 0,
        Bool = 1,
        Number = 2,
        String = 3,
        Array = 4,
        Object = 5
    }

    /// <summary>
    /// Exception thrown when a simdjson operation fails.
    /// </summary>
    public class SimdJsonException : Exception
    {
        public SimdJsonError ErrorCode { get; }

        public SimdJsonException(SimdJsonError code, string message = null)
            : base(message ?? $"SimdJSON error: {code}")
        {
            ErrorCode = code;
        }
    }

    /// <summary>
    /// High-performance SIMD JSON parser powered by the native simdjson library.
    /// Thread-safety: create one parser per thread for best performance.
    /// </summary>
    public sealed class Parser : IDisposable
    {
        private IntPtr _handle;
        private bool _disposed;

        public Parser()
        {
            _handle = Native.SimdJson_CreateParser();
            if (_handle == IntPtr.Zero)
                throw new SimdJsonException(SimdJsonError.MemAlloc, "Failed to create parser");
        }

        /// <summary>
        /// Parse a JSON string and return a Document. Caller must dispose the Document.
        /// </summary>
        public Document Parse(string json)
        {
            if (json == null) throw new ArgumentNullException(nameof(json));
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            return Parse(bytes);
        }

        /// <summary>
        /// Parse UTF-8 JSON bytes.
        /// </summary>
        public Document Parse(byte[] utf8Json)
        {
            if (utf8Json == null) throw new ArgumentNullException(nameof(utf8Json));
            return Parse(utf8Json, 0, utf8Json.Length);
        }

        /// <summary>
        /// Parse a slice of UTF-8 JSON bytes.
        /// </summary>
        public Document Parse(byte[] utf8Json, int offset, int length)
        {
            EnsureNotDisposed();
            if (utf8Json == null) throw new ArgumentNullException(nameof(utf8Json));
            if (offset < 0 || length < 0 || offset + length > utf8Json.Length)
                throw new ArgumentOutOfRangeException();

            IntPtr docHandle;
            SimdJsonError err;
            unsafe
            {
                fixed (byte* ptr = utf8Json)
                {
                    err = Native.SimdJson_Parse(_handle, (IntPtr)(ptr + offset), (UIntPtr)length, out docHandle);
                }
            }
            if (err != SimdJsonError.Ok)
                throw new SimdJsonException(err, "Parse failed");
            return new Document(docHandle);
        }

        /// <summary>
        /// Validate JSON without building a full document tree (fast path).
        /// </summary>
        public static bool TryValidate(string json, out SimdJsonError error)
        {
            if (json == null)
            {
                error = SimdJsonError.Invalid;
                return false;
            }
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            return TryValidate(bytes, out error);
        }

        public static bool TryValidate(byte[] utf8Json, out SimdJsonError error)
        {
            if (utf8Json == null)
            {
                error = SimdJsonError.Invalid;
                return false;
            }
            unsafe
            {
                fixed (byte* ptr = utf8Json)
                {
                    error = Native.SimdJson_Validate((IntPtr)ptr, (UIntPtr)utf8Json.Length);
                }
            }
            return error == SimdJsonError.Ok;
        }

        /// <summary>
        /// Minify JSON (remove whitespace). Returns the minified string.
        /// </summary>
        public static string Minify(string json)
        {
            if (json == null) throw new ArgumentNullException(nameof(json));
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            // allocate generous buffer
            byte[] outBuf = new byte[bytes.Length + 64];
            UIntPtr outLen;
            SimdJsonError err;
            unsafe
            {
                fixed (byte* inPtr = bytes)
                fixed (byte* outPtr = outBuf)
                {
                    err = Native.SimdJson_Minify((IntPtr)inPtr, (UIntPtr)bytes.Length,
                        (IntPtr)outPtr, (UIntPtr)outBuf.Length, out outLen);
                }
            }
            if (err != SimdJsonError.Ok)
                throw new SimdJsonException(err, "Minify failed");
            return Encoding.UTF8.GetString(outBuf, 0, (int)outLen);
        }

        /// <summary>
        /// Native library version string.
        /// </summary>
        public static string Version => Native.SimdJson_Version();

        public void Dispose()
        {
            if (!_disposed && _handle != IntPtr.Zero)
            {
                Native.SimdJson_DestroyParser(_handle);
                _handle = IntPtr.Zero;
                _disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~Parser() => Dispose();

        private void EnsureNotDisposed()
        {
            if (_disposed || _handle == IntPtr.Zero)
                throw new ObjectDisposedException(nameof(Parser));
        }
    }

    /// <summary>
    /// Owns a parsed JSON document. Must outlive any Element obtained from it.
    /// On-demand: elements are single-pass; iterate carefully.
    /// </summary>
    public sealed class Document : IDisposable
    {
        private IntPtr _handle;
        private bool _disposed;

        internal Document(IntPtr handle)
        {
            _handle = handle;
        }

        /// <summary>
        /// Get the root element of the document.
        /// </summary>
        public Element GetRoot()
        {
            EnsureNotDisposed();
            IntPtr elem;
            var err = Native.SimdJson_GetRoot(_handle, out elem);
            if (err != SimdJsonError.Ok)
                throw new SimdJsonException(err);
            return new Element(elem);
        }

        public void Dispose()
        {
            if (!_disposed && _handle != IntPtr.Zero)
            {
                Native.SimdJson_DestroyDocument(_handle);
                _handle = IntPtr.Zero;
                _disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~Document() => Dispose();

        private void EnsureNotDisposed()
        {
            if (_disposed || _handle == IntPtr.Zero)
                throw new ObjectDisposedException(nameof(Document));
        }
    }

    /// <summary>
    /// Represents a JSON value (null, bool, number, string, array, object).
    /// Dispose when finished to free native resources.
    /// Note: due to on-demand design, some operations consume the element.
    /// </summary>
    public sealed class Element : IDisposable
    {
        private IntPtr _handle;
        private bool _disposed;

        internal Element(IntPtr handle)
        {
            _handle = handle;
        }

        public SimdJsonType Type
        {
            get
            {
                EnsureNotDisposed();
                return Native.SimdJson_GetType(_handle);
            }
        }

        public bool IsNull => Type == SimdJsonType.Null;
        public bool IsBool => Type == SimdJsonType.Bool;
        public bool IsNumber => Type == SimdJsonType.Number;
        public bool IsString => Type == SimdJsonType.String;
        public bool IsArray => Type == SimdJsonType.Array;
        public bool IsObject => Type == SimdJsonType.Object;

        public bool GetBool()
        {
            EnsureNotDisposed();
            int v;
            var err = Native.SimdJson_GetBool(_handle, out v);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return v != 0;
        }

        public long GetInt64()
        {
            EnsureNotDisposed();
            long v;
            var err = Native.SimdJson_GetInt64(_handle, out v);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return v;
        }

        public ulong GetUInt64()
        {
            EnsureNotDisposed();
            ulong v;
            var err = Native.SimdJson_GetUInt64(_handle, out v);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return v;
        }

        public double GetDouble()
        {
            EnsureNotDisposed();
            double v;
            var err = Native.SimdJson_GetDouble(_handle, out v);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return v;
        }

        public string GetString()
        {
            EnsureNotDisposed();
            // first query length
            UIntPtr len;
            var err = Native.SimdJson_GetString(_handle, IntPtr.Zero, UIntPtr.Zero, out len);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            if ((int)len == 0) return string.Empty;

            byte[] buf = new byte[(int)len + 1];
            unsafe
            {
                fixed (byte* p = buf)
                {
                    err = Native.SimdJson_GetString(_handle, (IntPtr)p, (UIntPtr)buf.Length, out len);
                }
            }
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return Encoding.UTF8.GetString(buf, 0, (int)len);
        }

        // ---- Object helpers ----

        public int ObjectCount()
        {
            EnsureNotDisposed();
            UIntPtr count;
            var err = Native.SimdJson_ObjectCount(_handle, out count);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return (int)count;
        }

        /// <summary>
        /// Find a field by key. Returns null if not found (or throws depending on mode).
        /// </summary>
        public Element FindField(string key)
        {
            if (key == null) throw new ArgumentNullException(nameof(key));
            EnsureNotDisposed();
            byte[] keyBytes = Encoding.UTF8.GetBytes(key);
            IntPtr outVal;
            SimdJsonError err;
            unsafe
            {
                fixed (byte* kp = keyBytes)
                {
                    err = Native.SimdJson_ObjectFindField(_handle, (IntPtr)kp, (UIntPtr)keyBytes.Length, out outVal);
                }
            }
            if (err == SimdJsonError.NoSuchField) return null;
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return new Element(outVal);
        }

        /// <summary>
        /// Get object field at index (0-based). Also returns the key.
        /// </summary>
        public bool TryGetObjectAt(int index, out string key, out Element value)
        {
            EnsureNotDisposed();
            key = null;
            value = null;
            byte[] keyBuf = new byte[256];
            UIntPtr keyLen;
            IntPtr outVal;
            SimdJsonError err;
            unsafe
            {
                fixed (byte* kp = keyBuf)
                {
                    err = Native.SimdJson_ObjectAt(_handle, (UIntPtr)index,
                        (IntPtr)kp, (UIntPtr)keyBuf.Length, out keyLen, out outVal);
                }
            }
            if (err == SimdJsonError.IndexOutOfBounds) return false;
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            key = Encoding.UTF8.GetString(keyBuf, 0, (int)keyLen);
            value = new Element(outVal);
            return true;
        }

        // ---- Array helpers ----

        public int ArrayCount()
        {
            EnsureNotDisposed();
            UIntPtr count;
            var err = Native.SimdJson_ArrayCount(_handle, out count);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return (int)count;
        }

        public Element ArrayAt(int index)
        {
            EnsureNotDisposed();
            IntPtr outVal;
            var err = Native.SimdJson_ArrayAt(_handle, (UIntPtr)index, out outVal);
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            return new Element(outVal);
        }

        public bool TryArrayAt(int index, out Element value)
        {
            EnsureNotDisposed();
            value = null;
            IntPtr outVal;
            var err = Native.SimdJson_ArrayAt(_handle, (UIntPtr)index, out outVal);
            if (err == SimdJsonError.IndexOutOfBounds) return false;
            if (err != SimdJsonError.Ok) throw new SimdJsonException(err);
            value = new Element(outVal);
            return true;
        }

        // Convenience typed getters with path-like access for simple cases
        public Element this[string key] => FindField(key) ?? throw new SimdJsonException(SimdJsonError.NoSuchField, $"Key not found: {key}");
        public Element this[int index] => ArrayAt(index);

        public void Dispose()
        {
            if (!_disposed && _handle != IntPtr.Zero)
            {
                Native.SimdJson_DestroyElement(_handle);
                _handle = IntPtr.Zero;
                _disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~Element() => Dispose();

        private void EnsureNotDisposed()
        {
            if (_disposed || _handle == IntPtr.Zero)
                throw new ObjectDisposedException(nameof(Element));
        }
    }

    /// <summary>
    /// P/Invoke bindings to the native simdjson_unity library.
    /// </summary>
    internal static class Native
    {
#if UNITY_IOS && !UNITY_EDITOR
        const string LibName = "__Internal";
#elif UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
        const string LibName = "simdjson_unity";
#elif UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX
        const string LibName = "simdjson_unity";
#elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
        const string LibName = "simdjson_unity";
#elif UNITY_ANDROID
        const string LibName = "simdjson_unity";
#else
        const string LibName = "simdjson_unity";
#endif

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SimdJson_CreateParser();

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SimdJson_DestroyParser(IntPtr parser);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_Parse(IntPtr parser, IntPtr json, UIntPtr length, out IntPtr outDoc);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SimdJson_DestroyDocument(IntPtr doc);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetRoot(IntPtr doc, out IntPtr outElem);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SimdJson_DestroyElement(IntPtr elem);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonType SimdJson_GetType(IntPtr elem);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetBool(IntPtr elem, out int outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetInt64(IntPtr elem, out long outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetUInt64(IntPtr elem, out ulong outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetDouble(IntPtr elem, out double outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_GetString(IntPtr elem, IntPtr buffer, UIntPtr capacity, out UIntPtr outLength);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_ObjectCount(IntPtr elem, out UIntPtr outCount);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_ObjectFindField(IntPtr obj, IntPtr key, UIntPtr keyLen, out IntPtr outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_ObjectAt(IntPtr obj, UIntPtr index, IntPtr keyBuffer, UIntPtr keyCapacity, out UIntPtr outKeyLen, out IntPtr outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_ArrayCount(IntPtr elem, out UIntPtr outCount);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_ArrayAt(IntPtr arr, UIntPtr index, out IntPtr outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_Minify(IntPtr json, UIntPtr length, IntPtr outBuffer, UIntPtr outCapacity, out UIntPtr outLength);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimdJsonError SimdJson_Validate(IntPtr json, UIntPtr length);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern string SimdJson_Version();
    }
}
