// See https://aka.ms/new-console-template for more information
using Test;


#region TextHost Callback Implement

TextHostDll.OnOutputText? _output;
TextHostDll.ProcessCallback? _callback;
TextHostDll.OnCreateThread? _createThread;
TextHostDll.OnRemoveThread? _removeThread;


void CreateThreadHandle(
    long threadId,
    uint processId,
    ulong address,
    ulong context,
    ulong subContext,
    string name,
    string hookcode)
{
}

void OutputHandle(long threadId, string opData, uint length)
{
    Console.WriteLine(opData);
}

void RemoveThreadHandle(long threadId) { }

/// <summary>
/// Called when console thread created
/// </summary>
void OnConnectCallBackHandle(uint processId)
{
}
#endregion


Console.WriteLine("Hello, World!");

_createThread = CreateThreadHandle;
_output = OutputHandle;
_removeThread = RemoveThreadHandle;
_callback = OnConnectCallBackHandle;

TextHostDll.TextHostInit(_callback, _ => { }, _createThread, _removeThread, _output);

var a = System.Diagnostics.Process.GetProcessesByName("ぜったい征服☆学園結社パニャニャンダー!!")[0].Id;
//TextHostDll.InjectProcess((uint)a);

//System.Diagnostics.Process.Start(@"C:\Users\kimika\source\repos\texthost\Test\bin\Debug\net8.0\win-x64\libs\x86\Injector.exe", @$"{a} C:\Users\kimika\source\repos\texthost\Test\bin\Debug\net8.0\win-x64\libs\x86\texthook.dll");

//TextHostDll.DetachProcess((uint)a);

Console.WriteLine($"attach to pid {a}");

Console.ReadKey();

