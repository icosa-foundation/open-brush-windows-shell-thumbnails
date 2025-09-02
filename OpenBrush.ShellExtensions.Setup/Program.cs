using System;
using System.IO;
using SharpShell.ServerRegistration;
using Voxels.ShellExtensions;

namespace OpenBrush.ShellExtensions.Setup
{
    internal static class Program
    {
        static void Main(string[] args)
        {
            var serverPath = typeof(ThumbnailHandlerVox).Assembly.Location;
            var manager = new ServerRegistrationManager();

            try
            {
                if (args.Length > 0 && args[0].StartsWith("/u", StringComparison.OrdinalIgnoreCase))
                {
                    manager.UninstallServer(serverPath);
                }
                else
                {
                    manager.InstallServer(serverPath, RegistrationType.OS32Bit, RegistrationMode.PerMachine);
                }

                Console.WriteLine("Installed");
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                try
                {
                    File.AppendAllText("setup.log", ex.ToString());
                }
                catch
                {
                    // ignore failures to write log file
                }
            }

            Console.ReadLine();
        }
    }
}
