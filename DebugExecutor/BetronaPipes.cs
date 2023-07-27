using System.Diagnostics;
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;

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

    public static bool InjectDLL()
    {
        FileIO.GetClientPath();

        Process process = Process.GetProcessesByName("Minecraft.Windows")[0];

        string path = "MCBELight.dll";

        FileInfo fileInfo = new FileInfo(path);
        FileSecurity accessControl = fileInfo.GetAccessControl();
        accessControl.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier("S-1-15-2-1"), FileSystemRights.FullControl, InheritanceFlags.None, PropagationFlags.NoPropagateInherit, AccessControlType.Allow));
        fileInfo.SetAccessControl(accessControl);

        IntPtr hProcess = kernel32.OpenProcess(1082, false, process.Id);
        IntPtr procAddress = kernel32.GetProcAddress(kernel32.GetModuleHandle("kernel32.dll"), "LoadLibraryA");
        IntPtr intPtr = kernel32.VirtualAllocEx(hProcess, IntPtr.Zero, (uint)((path.Length + 1) * Marshal.SizeOf(typeof(char))), 12288U, 4U);

        kernel32.WriteProcessMemory(hProcess, intPtr, Encoding.Default.GetBytes(path), (uint)((path.Length + 1) * Marshal.SizeOf(typeof(char))), out _);
        kernel32.CreateRemoteThread(hProcess, IntPtr.Zero, 0U, procAddress, intPtr, 0U, IntPtr.Zero);

        return true;
    }
}