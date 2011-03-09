using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.Windows.Forms;
using Google.NaClVsx.Installation;
using Microsoft.Win32;


namespace Google.NaClVsx
{
  [RunInstaller(true)]
  public partial class InstallHelp : Installer
  {
    private static readonly string kVsName = "devenv";

    private static readonly string[] kVsRegKeys = new string[] {
        @"HKEY_CURRENT_USER\Software\Microsoft\VisualStudio\9.0\Setup\VS",
        @"HKEY_LOCAL_MACHINE\Software\Microsoft\VisualStudio\9.0\Setup\VS",
    };

    private static readonly string kVsRegValue = "EnvironmentPath";

    private static readonly int kTimeout = 100;


    public InstallHelp()
    {
      InitializeComponent();
      progress_.Cancelled += new EventHandler(OnProgressCancelled);
      worker_.RunWorkerCompleted += new RunWorkerCompletedEventHandler(WorkerCompleted);
    }

    void OnProgressCancelled(object sender, EventArgs e)
    {
      worker_.CancelAsync();
    }

    void WorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
    {
      if (e.Error != null) {
        MessageBox.Show(progress_, e.Error.Message, "Install Error");
        progress_.Cancel();
      } else {
        progress_.DialogResult = DialogResult.OK;
        progress_.Close();
      }
    }

    public override void  Install(IDictionary stateSaver) {
      var vsProcesses = new List<Process>(Process.GetProcessesByName(kVsName));
      if (vsProcesses.Count > 0) {
        progress_.Text = "Preparing to install";
        progress_.Message = "Closing Visual Studio...";
        var onLoad = new EventHandler(OnProgressLoadInstall);
        progress_.Load += onLoad;

        var workerfn = new DoWorkEventHandler(KillVsWorker);
        worker_.DoWork += workerfn;

        var result = progress_.ShowDialog();

        worker_.DoWork -= workerfn;
        progress_.Load -= onLoad;

        // If we canceled, we need to let the installer know.
        if (result == DialogResult.Cancel) {
          throw new InstallException("Operation cancelled");
        } 
      }

      base.Install(stateSaver);
    }

    private void OnProgressLoadInstall(object sender, EventArgs e) {
      var workerfn = new DoWorkEventHandler(KillVsWorker);
      worker_.DoWork += workerfn;

      var result = MessageBox.Show(
          null,
          "Installation cannot proceed until Visual Studio is closed.\n" +
          "Close Visual Studio automatically?\n" +
          "(Select 'Yes' to close Visual Studio automatically. Select 'No' to close it yourself. Select 'Cancel' to abort the install process.",
          "Installer",
          MessageBoxButtons.YesNoCancel,
          MessageBoxIcon.Question,
          MessageBoxDefaultButton.Button1,
          MessageBoxOptions.ServiceNotification);
      if (result != DialogResult.Cancel) {
          worker_.RunWorkerAsync(result);
      } else {
        progress_.DialogResult = result;
        progress_.Close();
      }
    }

    public override void Commit(IDictionary savedState)
    {
      var workerfn = new DoWorkEventHandler(RunDevenvSetupWorker);
      worker_.DoWork += workerfn;
      progress_.Text = "Finalizing";
      progress_.Message = "Registering package with Visual Studio...";

      var onLoad = new EventHandler(OnProgressLoadCommit);
      progress_.Load += onLoad;
      var result = progress_.ShowDialog();
      worker_.DoWork -= workerfn;

      if (result == DialogResult.Cancel) {
        MessageBox.Show(
            null,
            "Setup was unable to automatically register " +
            "components with Visual Studio.\n" +
            "To register the Native Client support components manually, " +
            "run 'devenv.exe /setup' from a Visual Studio command prompt.",
            "Registration Failure",
            MessageBoxButtons.OK,
            MessageBoxIcon.Warning,
            MessageBoxDefaultButton.Button1,
            MessageBoxOptions.ServiceNotification);
      }

    }

    private void OnProgressLoadCommit(object sender, EventArgs e) {
      worker_.RunWorkerAsync();
    }

    void KillVsWorker(object sender, DoWorkEventArgs e)
    {
      BackgroundWorker worker = sender as BackgroundWorker;
      DialogResult shouldKill = (DialogResult)e.Argument;

      var vsProcesses = new List<Process>(Process.GetProcessesByName(kVsName));
      foreach (var vsProcess in vsProcesses)
      {
        while (!vsProcess.HasExited && !worker.CancellationPending)
        {
          if (shouldKill == DialogResult.Yes) {
            vsProcess.CloseMainWindow();
          }
          vsProcess.WaitForExit(kTimeout);
        }
      }
    }

    private void RunDevenvSetupWorker(object sender, DoWorkEventArgs e)
    {
      BackgroundWorker worker = sender as BackgroundWorker;
      string devenv = null;

      foreach (var vsRegKey in kVsRegKeys) {
        devenv = (string)Registry.GetValue(vsRegKey, kVsRegValue, null);
        if (devenv != null) break;
      }

      if (devenv == null) {
        throw new InstallException("Could not find devenv.exe");
      }
      var vsProcess = Process.Start(devenv, "/setup");
      while (!vsProcess.HasExited && !worker.CancellationPending)
      {
        vsProcess.WaitForExit(kTimeout);
      }
    }


    private ProgressForm progress_ = new ProgressForm();
  }
}
