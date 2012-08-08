// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;

  using EnvDTE;
  using Microsoft.VisualStudio.VCProjectEngine;

  /// <summary>
  /// This class contains the functionality related to the web server which hosts the web page
  /// during debugging.
  /// </summary>
  public class WebServer : IDisposable
  {
    /// <summary>
    /// The web server port to default to if the user does not specify one.
    /// </summary>
    private const int DefaultWebServerPort = 5103;

    /// <summary>
    /// Holds the main web server process.
    /// </summary>
    private System.Diagnostics.Process webServer_;

    /// <summary>
    /// Captures output from the web server.
    /// </summary>
    private OutputWindowPane webServerOutputPane_;

    /// <summary>
    /// Keeps track of if dispose has been called.
    /// </summary>
    private bool disposed_ = false;

    /// <summary>
    /// Constructs the WebServer, starts the web server process.
    /// </summary>
    /// <param name="outputWindowPane">Existing output pane to send web server output to.</param>
    /// <param name="properties">PropertyManager that is set to a valid project/platform.</param>
    public WebServer(OutputWindowPane outputWindowPane, PropertyManager properties)
    {
      if (outputWindowPane == null)
      {
        throw new ArgumentNullException("outputWindowPane");
      }

      if (properties == null)
      {
        throw new ArgumentNullException("properties");
      }

      webServerOutputPane_ = outputWindowPane;

      // Read port from properties, if invalid port then set to default value.
      int webServerPort;
      if (!int.TryParse(properties.WebServerPort, out webServerPort))
      {
        webServerPort = DefaultWebServerPort;
      }

      string webServerExecutable = "python.exe";
      string webServerArguments = string.Format(
          "{0}\\examples\\httpd.py --no_dir_check {1}", properties.SDKRootDirectory, webServerPort);

      // Start the web server process.
      try
      {
        webServer_ = new System.Diagnostics.Process();
        webServer_.StartInfo.CreateNoWindow = true;
        webServer_.StartInfo.UseShellExecute = false;
        webServer_.StartInfo.RedirectStandardOutput = true;
        webServer_.StartInfo.RedirectStandardError = true;
        webServer_.StartInfo.FileName = webServerExecutable;
        webServer_.StartInfo.Arguments = webServerArguments;
        webServer_.StartInfo.WorkingDirectory = properties.ProjectDirectory;
        webServer_.OutputDataReceived += WebServerMessageReceive;
        webServer_.ErrorDataReceived += WebServerMessageReceive;
        webServer_.Start();
        webServer_.BeginOutputReadLine();
        webServer_.BeginErrorReadLine();
      }
      catch (Exception e)
      {
        webServerOutputPane_.OutputString(Strings.WebServerStartFail + "\n");
        webServerOutputPane_.OutputString("Exception: " + e.Message + "\n");
        webServerOutputPane_.Activate();
      }

      webServerOutputPane_.Clear();
      webServerOutputPane_.OutputString(Strings.WebServerStartMessage + "\n");
      webServerOutputPane_.Activate();
    }

    /// <summary>
    /// Finalizer. Should clean up unmanaged resources. Should not be overriden in derived classes.
    /// </summary>
    ~WebServer()
    {
      Dispose(false);
    }

    /// <summary>
    /// Disposes the object when called by user code (not directly by garbage collector).
    /// </summary>
    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the object. If disposing is false then this has been called by garbage collection,
    /// and we shouldn't reference managed objects.
    /// </summary>
    /// <param name="disposing">True if user called Dispose, false if garbage collection.</param>
    protected virtual void Dispose(bool disposing)
    {
      if (!disposed_ && disposing)
      {
        // Managed resource clean up.
        Utility.EnsureProcessKill(ref webServer_);
        webServerOutputPane_.OutputString(Strings.WebServerStopMessage);
        webServerOutputPane_.Activate();
      }

      disposed_ = true;
    }

    /// <summary>
    /// Receives output from the web server process to display in the Visual Studio UI.
    /// </summary>
    /// <param name="sender">The parameter is not used.</param>
    /// <param name="e">Contains the data to display.</param>
    private void WebServerMessageReceive(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
      webServerOutputPane_.OutputString(e.Data + "\n");
    }
  }
}
