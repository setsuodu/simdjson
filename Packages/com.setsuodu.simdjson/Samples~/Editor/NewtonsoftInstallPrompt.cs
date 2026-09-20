using UnityEditor;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;
using UnityEngine;

[InitializeOnLoad]
public static class NewtonsoftInstallPrompt
{
    const string PackageId = "com.unity.nuget.newtonsoft-json";

    // 仅本次 Domain Reload，不写磁盘、不用 EditorPrefs
    static bool _shownThisReload;
    static AddRequest _addRequest;

    static NewtonsoftInstallPrompt()
    {
        EditorApplication.delayCall += TryPrompt;
    }

    static void TryPrompt()
    {
        if (_shownThisReload)
            return;
        _shownThisReload = true;

        if (IsPackageInProject(PackageId))
            return;

        bool install = EditorUtility.DisplayDialog(
            "SimdJSON Sample",
            "Benchmark 需要安装：\n\n  " + PackageId + "\n\nAPI 示例不装也能跑。是否现在安装？",
            "安装",
            "稍后");

        if (!install)
            return;

        Debug.Log("[SimdJSON] Installing " + PackageId + "...");
        _addRequest = Client.Add(PackageId);
        EditorApplication.update += OnProgress;
    }

    static void OnProgress()
    {
        if (_addRequest == null || !_addRequest.IsCompleted)
            return;

        EditorApplication.update -= OnProgress;

        if (_addRequest.Status == StatusCode.Success)
            Debug.Log("[SimdJSON] Installed: " + _addRequest.Result.packageId);
        else
            Debug.LogError("[SimdJSON] Install failed: " + _addRequest.Error.message);

        _addRequest = null;
    }

    static bool IsPackageInProject(string packageId)
    {
        var list = UnityEditor.PackageManager.PackageInfo.GetAllRegisteredPackages();
        if (list == null) return false;
        foreach (var p in list)
            if (p.name == packageId)
                return true;
        return false;
    }
}