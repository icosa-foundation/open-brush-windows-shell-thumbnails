using System;
using SharpShell.ServerRegistration;
using OpenBrush.ShellExtensions;

namespace OpenBrush.ShellExtensions.Setup
{
    internal static class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            var server = new ThumbnailHandlerTilt();
            var regType = Environment.Is64BitOperatingSystem
                ? RegistrationType.OS64Bit
                : RegistrationType.OS32Bit;

            if (args.Length > 0 && args[0].Equals("/uninstall", StringComparison.OrdinalIgnoreCase))
            {
                ServerRegistrationManager.UninstallServer(server, regType);
            }
            else
            {
                ServerRegistrationManager.InstallServer(server, regType, true);
            }
        }
    }
}
