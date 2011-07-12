using System;
using System.IO;
using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NaClVsx.Package_UnitTestProject
{
  /// <summary>
  /// Contains various utilities that are useful for multiple tests.
  /// </summary>
  class NaClPackageTestUtils
  {
    /// <summary>
    /// Creates a new NaClSymbolProvider and parses our loop.nexe sample
    /// binary.
    /// </summary>
    /// <returns>The pre-loaded symbol provider.</returns>
    public static NaClSymbolProvider LoadLoopNexe() {
      var symbolProvider = new NaClSymbolProvider(null);
      string status;
      bool ok = symbolProvider.LoadModule(
          Path.Combine(GetVsxRootDir(),
                       kNexePath),
          kBaseAddr, out status);
      Assert.IsTrue(ok, "LoadModule failed");
      return symbolProvider;
    }

    /// <summary>
    /// Finds the SDK root, using the system environment.  Will fail to assert
    /// if root cannot be located.
    /// </summary>
    /// <returns>The location of the VSX root directory.</returns>
    public static string GetVsxRootDir() {
      if (null == vsxRootDir) {
        vsxRootDir = Environment.GetEnvironmentVariable("NACL_VSX_ROOT");
      }
      Assert.IsNotNull(vsxRootDir,
                       "Could not get NACL_VSX_ROOT from environment.");
      return vsxRootDir;
    }

    /// <summary>
    /// 
    /// </summary>
    //public static readonly string kCodePath = @"src\loop\loop.cc";

    /// <summary>
    /// The base address at which symbol information should start in nexe code.
    /// </summary>
    public static readonly ulong kBaseAddr = 0x00000ffc00000000;

    /// <summary>
    /// The location realtive to the  at which the loop nexe is to be found.
    /// </summary>
    private static readonly string kNexePath = @"src\loop\loop.nexe";

    /// <summary>
    /// The location of the visual studio plugin code.
    /// </summary>
    private static string vsxRootDir;
  }
}
