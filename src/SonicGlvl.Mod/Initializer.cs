using System;
using System.IO;
using System.Reflection;
using System.Security;
using System.Security.Permissions;
using GhostRecorder;

namespace SonicGLvl
{
    /// <summary>
    /// This class is simply a proxy for calling Init again from another AppDomain.
    /// </summary>
    public class InitProxy : MarshalByRefObject
    {
        public void Run()
        {
            Initializer.Initialize();
        }

        public void OnFrame()
        {
            Mod.OnFrame();
        }
    }

    public class Initializer
    {
        /// <summary>
        /// Contains our child AppDomain used for init-ing mods in their own separate worlds.
        /// </summary>
        static AppDomain _childDomain;

        static dynamic _proxyObject;

        /// <summary>
        /// This file and/or Initializer.cs contains the DLL Template for Reloaded Mod Loader mods.
        /// If you are looking for user code, please see Program.cs
        /// </summary>
        [DllExport]
        public static void Init()
        {
            // Try restarting in another AppDomain if possible.
            try
            {
                // Give the new AppDomain full permissions.
                PermissionSet permissionSet = new PermissionSet(PermissionState.Unrestricted);
                permissionSet.AddPermission(new SecurityPermission(SecurityPermissionFlag.AllFlags));

                // The ApplicationBase of the new domain should be the directory containing the current DLL.
                AppDomainSetup appDomainSetup = new AppDomainSetup()
                {
                    ApplicationBase = Path.GetDirectoryName(typeof(InitProxy).Assembly.Location)
                };
                _childDomain = AppDomain.CreateDomain("SonicGlvl", null, appDomainSetup, permissionSet);
                _childDomain.UnhandledException += ChildDomain_UnhandledException;

                // Now make the new AppDomain load our code using our proxy.
                Type proxyType = typeof(InitProxy);
                _proxyObject = _childDomain.CreateInstanceFrom(proxyType.Assembly.Location, proxyType.FullName).Unwrap(); // Our AssemblyResolve will pick the missing DLL out.
                _proxyObject.Run();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                Initialize();
            }
        }

        [DllExport]
        public static void OnFrame()
        {
            _proxyObject.OnFrame();
        }

        /// <summary>
        /// Throws exceptions in the default AppDomain when/if the application crashes.
        /// VS may otherwise fail to get the stack trace.
        /// </summary>
        public static void ChildDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            throw (Exception) e.ExceptionObject;
        }

        /// <summary>
        /// This file and/or Initializer.cs contains the DLL Template for Reloaded Mod Loader mods.
        /// If you are looking for user code, please see Program.cs
        /// </summary>
        public static void Initialize()
        {
            AppDomain.CurrentDomain.AssemblyResolve += LocalAssemblyFinder.ResolveAssembly;
            InitializeInternal();
        }

        /// <summary>
        /// This file contains the main entry code executed as part of the DLL template for Reloaded Mod Loader
        /// mods. It is very important that the entry method contains only AppDomain.CurrentDomain.AssemblyResolve
        /// due to otherwise possible problems with static initialization of Program.
        /// </summary>
        public static void InitializeInternal()
        {
            // Call Init
            try
            {
                Mod.Init();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }

    /// <summary>
    /// You know how when you inject a DLL into a process it will fail to find any of the libraries that 
    /// go along with it? I do too, worry not, this will help you.
    /// </summary>
    public static class LocalAssemblyFinder
    {
        /// <summary>
        /// Finds and retrieves an Assembly/Module/DLL from the libraries folder in the case it is not
        /// yet loaded or the mod fails to find the assembly.
        /// </summary>
        public static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            // Get mod folder.
            string modFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            // Get the path to the mod's libraries folder.
            string localLibrary = modFolder + "\\Libraries\\";
            string thisFolderLibrary = modFolder + "\\";
            string dllName = new AssemblyName(args.Name).Name + ".dll";

            // Append the assembly name.
            localLibrary += dllName;
            thisFolderLibrary += dllName;

            // Try loading from the current folder.
            if (File.Exists(thisFolderLibrary))
                return Assembly.LoadFrom(thisFolderLibrary);

            // Try loading from local library folder.
            if (File.Exists(localLibrary))
                return Assembly.LoadFrom(localLibrary);

            // Panic mode! Search all subdirectories!
            // Obtain potential libraries.
            string[] libraries = Directory.GetFiles(modFolder, args.Name, SearchOption.AllDirectories);

            // If one is found, select first.
            return libraries.Length > 0 ? Assembly.LoadFrom(libraries[0]) : null;
        }

    }
}
