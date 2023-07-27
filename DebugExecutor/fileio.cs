using System.IO;
using System;

public class FileIO
{
    public static string GetRoamingStatePath()
    {
        return Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\..\Local\Packages\Microsoft.MinecraftUWP_8wekyb3d8bbwe\RoamingState\";
    }

    public static string GetClientPath()
    {
        return GetRoamingStatePath() + "BetronaScripter\\";
    }

    public static bool DoesClientPathExist(string path)
    {
        return File.Exists(GetClientPath() + path);
    }

    public static void CreatePath(string path)
    {
        Directory.CreateDirectory(GetClientPath() + path);
    }

    public static void DeletePath(string path)
    {
        File.Delete(path);
    }

    public static bool SetupClientPath()
    {
        if (!Directory.Exists(GetClientPath()))
        {
            Directory.CreateDirectory(GetClientPath());
            return true;
        }
        return false;
    }

    public static void WriteFile(string filePath, string content)
    {
        using (StreamWriter file = new StreamWriter(GetClientPath() + filePath))
        {
            file.Write(content);
        }
    }

    public static string ReadFile(string filePath)
    {
        string content = string.Empty;
        using (StreamReader file = new StreamReader(GetClientPath() + filePath))
        {
            string line;
            while ((line = file.ReadLine()) != null)
            {
                content += line;
            }
        }
        return content;
    }
}