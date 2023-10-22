using System.Diagnostics;
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Windows.Forms;

public class BetronaPipes
{
    public static bool Execute(string script)
    {
        try
        {
            FileIO.WriteFile("script.tmp", script);
        }
        catch
        {
            return false;
        }

        return true;
    }

    #region Access

    const int PROCESS_CREATE_THREAD = 0x2;
    const int PROCESS_QUERY_INFORMATION = 0x400;
    const int PROCESS_VM_OPERATION = 0x8;
    const int PROCESS_VM_WRITE = 0x20;
    const int PROCESS_VM_READ = 0x10;

    const uint MEM_COMMIT = 0x1000;
    const uint MEM_RESERVE = 0x2000;
    const uint PAGE_READWRITE = 4;

    #endregion

    public static bool InjectDLL()
    {
        FileIO.GetClientPath();

        Process process = Process.GetProcessesByName("Minecraft.Windows")[0];

        if (process == null)
            return false;

        string path = new FileInfo("MCBELight.dll").FullName;

        FileInfo fileInfo = new FileInfo(path);
        FileSecurity accessControl = fileInfo.GetAccessControl();
        accessControl.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier("S-1-15-2-1"), FileSystemRights.FullControl, InheritanceFlags.None, PropagationFlags.NoPropagateInherit, AccessControlType.Allow));
        fileInfo.SetAccessControl(accessControl);

        IntPtr procHandle = kernel32.OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, process.Id);
        IntPtr loadLibraryA = kernel32.GetProcAddress(kernel32.GetModuleHandle("kernel32.dll"), "LoadLibraryA");
        IntPtr allocMemAdr = kernel32.VirtualAllocEx(procHandle, IntPtr.Zero, (uint)((path.Length + 1) * Marshal.SizeOf(typeof(char))), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (!kernel32.WriteProcessMemory(procHandle, allocMemAdr, Encoding.Default.GetBytes(path), (uint)((path.Length + 1) * Marshal.SizeOf(typeof(char))), out _))
            return false;

        if (kernel32.CreateRemoteThread(procHandle, IntPtr.Zero, 0, loadLibraryA, allocMemAdr, 0, IntPtr.Zero) != IntPtr.Zero)
        {
            //return false;
        }

        return true;
    }
}