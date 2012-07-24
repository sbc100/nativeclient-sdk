// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Collections.Generic;
  using System.Linq;
  using System.Management;

  using EnvDTE;
  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;
  using Microsoft.VisualStudio.VCProjectEngine;

  /// <summary>
  /// This class contains utilities for running tests.
  /// </summary>
  public static class TestUtilities
  {
    /// <summary>
    /// This starts an instance of Visual Studio and get it's DTE object.
    /// </summary>
    /// <returns>DTE of the started instance.</returns>
    public static DTE2 StartVisualStudioInstance()
    {
      // Set up filter to handle threading events and automatically retry calls
      // to dte which fail because dte is busy.
      ComMessageFilter.Register();

      Type visualStudioType = Type.GetTypeFromProgID("VisualStudio.DTE.10.0");
      DTE2 visualStudio = Activator.CreateInstance(visualStudioType) as DTE2;
      if (visualStudio == null)
      {
        throw new Exception("Visual Studio failed to start");
      }

      visualStudio.MainWindow.Visible = true;
      return visualStudio;
    }

    /// <summary>
    /// This properly cleans up after StartVisualStudioInstance().
    /// </summary>
    /// <param name="dte">Dte instance returned by StartVisualStudioInstance().</param>
    public static void CleanUpVisualStudioInstance(DTE2 dte)
    {
      if (dte != null)
      {
        if (dte.Solution != null)
        {
          dte.Solution.Close();
        }

        dte.Quit();
      }

      // Stop the message filter.
      ComMessageFilter.Revoke();
    }

    /// <summary>
    /// This returns the text contained in the given output window pane.
    /// </summary>
    /// <param name="pane">Pane to get text from.</param>
    /// <returns>Text in the window.</returns>
    public static string GetPaneText(OutputWindowPane pane)
    {
      TextSelection selection = pane.TextDocument.Selection;
      selection.StartOfDocument(false);
      selection.EndOfDocument(true);
      return selection.Text;
    }

    /// <summary>
    /// This starts a python process that just sleeps waiting to be killed.
    /// It can be used with DoesProcessExist() to verify that a process started/exited.
    /// </summary>
    /// <param name="identifierString">
    /// A unique string to identify the process via its command line arguments.
    /// </param>
    /// <param name="timeout">Time in seconds to wait before process exits on its own.</param>
    /// <returns>The process object that was started.</returns>
    public static System.Diagnostics.Process StartProcessForKilling(
        string identifierString, int timeout)
    {
      string args = string.Format(
          "-c \"import time; time.sleep({0}); print '{1}'\"",
          timeout,
          identifierString);
      System.Diagnostics.Process proc = new System.Diagnostics.Process();
      proc.StartInfo.CreateNoWindow = true;
      proc.StartInfo.UseShellExecute = false;
      proc.StartInfo.FileName = "python.exe";
      proc.StartInfo.Arguments = args;
      proc.Start();
      return proc;
    }

    /// <summary>
    /// This returns true if there is a running process that has command line arguments
    /// containing the given Strings.  The search is case-insensitive.
    /// </summary>
    /// <param name="processName">Name of the process executable.</param>
    /// <param name="commandLineIdentifiers">Strings to check for.</param>
    /// <returns>True if some process has the Strings in its command line arguments.</returns>
    public static bool DoesProcessExist(string processName, params string[] commandLineIdentifiers)
    {
      List<string> results = new List<string>();
      string query =
          string.Format("select CommandLine from Win32_Process where Name='{0}'", processName);
      using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(query))
      {
        using (ManagementObjectCollection result = searcher.Get())
        {
          foreach (ManagementObject process in result)
          {
            string commandLine = process["CommandLine"] as string;
            if (string.IsNullOrEmpty(commandLine))
            {
              break;
            }

            // Check if the command line contains each of the required identifiers.
            if (commandLineIdentifiers.All(i => commandLine.Contains(i)))
            {
              return true;
            }
          }
        }
      }

      return false;
    }

    /// <summary>
    /// Sets the active configuration for the solution by specifying the configuration name
    /// and platform name. A solution configuration containing a project configuration that has
    /// the config and platform names specified for the specified project is selected.
    /// </summary>
    /// <param name="dte">The main visual studio object.</param>
    /// <param name="projectUniqueName">UniqueName of the project to match.</param>
    /// <param name="configurationName">Ex: "Debug" or "Release".</param>
    /// <param name="platformName">Ex: "Win32" or "NaCl" or "PPAPI".</param>
    public static void SetSolutionConfiguration(
        DTE2 dte,
        string projectUniqueName,
        string configurationName,
        string platformName)
    {
      foreach (EnvDTE.SolutionConfiguration config in
          dte.Solution.SolutionBuild.SolutionConfigurations)
      {
        EnvDTE.SolutionContext context = null;
        try
        {
          context = config.SolutionContexts.Item(projectUniqueName);
        }
        catch (ArgumentException)
        {
          throw new Exception(
              string.Format("Project unique name not found in solution: {0}", projectUniqueName));
        }

        if (context == null)
        {
          throw new Exception("Failed to get solution context");
        }

        if (context.PlatformName == platformName && context.ConfigurationName == configurationName)
        {
          config.Activate();
          return;
        }
      }

      throw new Exception(string.Format(
          "Matching configuration not found for {0}: {1}|{2}",
          projectUniqueName,
          platformName,
          configurationName));
    }

    /// <summary>
    /// Returns a VCConfiguration object with a matching configuration name and platform type.
    /// </summary>
    /// <param name="project">Project to get the configuration from.</param>
    /// <param name="name">Name of configuration (e.g. 'Debug').</param>
    /// <param name="platform">Name of the platform (e.g. 'NaCl').</param>
    /// <returns>A matching VCConfiguration object.</returns>
    public static VCConfiguration GetVCConfiguration(Project project, string name, string platform)
    {
      VCProject vcproj = (VCProject)project.Object;
      IVCCollection configs = vcproj.Configurations;

      foreach (VCConfiguration config in configs)
      {
        if (config.ConfigurationName == name && config.Platform.Name == platform)
        {
          return config;
        }
      }

      throw new Exception(
          string.Format("Project does not have configuration: {0}|{1}", platform, name));
    }

    /// <summary>
    /// Tests that a given property has a specific value in a certain VCConfiguration
    /// </summary>
    /// <param name="configuration">Gives the platform and configuration type</param>
    /// <param name="pageName">Property page name where property resides.</param>
    /// <param name="propertyName">Name of the property to check.</param>
    /// <param name="expectedValue">Expected value of the property.</param>
    /// <param name="ignoreCase">Ignore case when comparing the expected and actual values.</param>
    public static void AssertPropertyEquals(
        VCConfiguration configuration,
        string pageName,
        string propertyName,
        string expectedValue,
        bool ignoreCase)
    {
      IVCRulePropertyStorage rule = configuration.Rules.Item(pageName);
      string callInfo = string.Format(
          "Page: {0}, Property: {1}, Configuration: {2}",
          pageName,
          propertyName,
          configuration.ConfigurationName);

      Assert.AreEqual(
          expectedValue,
          rule.GetUnevaluatedPropertyValue(propertyName),
          ignoreCase,
          callInfo);
    }

    /// <summary>
    /// Tests that a given property contains a specific string in a certain VCConfiguration
    /// </summary>
    /// <param name="configuration">Gives the platform and configuration type</param>
    /// <param name="pageName">Property page name where property resides.</param>
    /// <param name="propertyName">Name of the property to check.</param>
    /// <param name="expectedValue">Expected string to contain.</param>
    /// <param name="ignoreCase">Ignore case when comparing the expected and actual values.</param>
    public static void AssertPropertyContains(
        VCConfiguration configuration,
        string pageName,
        string propertyName,
        string expectedValue,
        bool ignoreCase)
    {
      StringComparison caseSensitive = ignoreCase ?
          StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;

      IVCRulePropertyStorage rule = configuration.Rules.Item(pageName);
      string propertyValue = rule.GetUnevaluatedPropertyValue(propertyName);

      string message = string.Format(
          "{0} should be contained in {1}. Page: {2}, Property: {3}, Configuration: {4}",
          expectedValue,
          propertyValue,
          pageName,
          propertyName,
          configuration.ConfigurationName);

      Assert.IsTrue(propertyValue.Contains(expectedValue, caseSensitive), message);
    }

    /// <summary>
    /// Extends the string class to allow checking if a string contains another string
    /// allowing a comparison type (such as case-insensitivity).
    /// </summary>
    /// <param name="source">Base string to search.</param>
    /// <param name="toCheck">String to check if contained within base string.</param>
    /// <param name="comparison">Comparison type.</param>
    /// <returns>True if toCheck is contained in source.</returns>
    public static bool Contains(this string source, string toCheck, StringComparison comparison)
    {
      return source.IndexOf(toCheck, comparison) != -1;
    }
  }
}
