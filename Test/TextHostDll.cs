using System.Runtime.InteropServices;

namespace Test;

internal static class TextHostDll
{
    private const string TextHostRelativePath = @"texthost.dll";

    #region Callback Delegate
    internal delegate void ProcessCallback(uint processId);

    internal delegate void OnCreateThread(
        long threadId,
        uint processId,
        ulong address,
        ulong context,
        ulong subContext,
        [MarshalAs(UnmanagedType.LPWStr)] string name,
        [MarshalAs(UnmanagedType.LPWStr)] string hookCode);

    internal delegate void OnRemoveThread(long threadId);

    internal delegate void OnOutputText(long threadId, [MarshalAs(UnmanagedType.LPWStr)] string text, uint length);
    #endregion

    [DllImport(TextHostRelativePath)]
    internal static extern int TextHostInit(
        ProcessCallback onConnect,
        ProcessCallback onDisconnect,
        OnCreateThread onCreateThread,
        OnRemoveThread onRemoveThread,
        OnOutputText onOutputText
        );

    [DllImport(TextHostRelativePath)]
    internal static extern int InsertHook(
        uint processId,
        [MarshalAs(UnmanagedType.LPWStr)] string hookCode
        );

    [DllImport(TextHostRelativePath)]
    internal static extern int RemoveHook(uint processId, ulong address);

    [DllImport(TextHostRelativePath)]
    internal static extern int InjectProcess(uint processId);

    [DllImport(TextHostRelativePath)]
    internal static extern int DetachProcess(uint processId);

    [DllImport(TextHostRelativePath)]
    internal static extern int SearchForText(
        uint processId,
        [MarshalAs(UnmanagedType.LPWStr)] string text,
        int codepage
    );

    /// <summary>
    /// Weird after a period of time, we don't use it
    /// </summary>
    [DllImport(TextHostRelativePath)]
    internal static extern int AddClipboardThread(IntPtr handle);
}
