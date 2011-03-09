using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;

namespace MsiHelp
{
  public enum MainFormResultCode {
    Unknown = -1,
    Ok = 0,
    UserCancelled,
    BadArgs,
    Error,
  }

  public partial class MainForm : Form
  {
    private static readonly string kVsName = "devenv";

    public MainForm()
    {
      InitializeComponent();
      ResultCode = MainFormResultCode.Unknown;
      this.Visible = false;
    }

    public MainFormResultCode ResultCode { get; set; }
    public Exception Error { get; set; }

    protected override void OnLoad(EventArgs e)
    {
      this.Visible = false;
      string[] args = Environment.GetCommandLineArgs();
      if (args.Length != 2) {
        MessageBox.Show(this, "Oops: wrong number of arguments", "MsiHelp", MessageBoxButtons.OK, MessageBoxIcon.Error);
        ResultCode = MainFormResultCode.BadArgs;
        Close();
      }

      if (args[1] == "test") {
        this.Text = "Test Installer";
        message_.Text = "Wasting time...";
        worker_.DoWork += new DoWorkEventHandler(TestWorker);
      } else if (args[1] == "killvs") {
        this.Visible = true;
        Text = "Close Visual Studio";
        message_.Text = "Waiting for Visual Studio to close...";
        worker_.DoWork += new DoWorkEventHandler(KillVsWorker);
      } else if (args[1] == "setup") {
        this.Visible = true;
        Text = "Registering Packages";
        message_.Text = "Registering packages with Visual Studio...";
        worker_.DoWork += new DoWorkEventHandler(DevEnvSetup);
      }

      worker_.RunWorkerAsync();
      base.OnLoad(e);
    }

    void DevEnvSetup(object sender, DoWorkEventArgs e)
    {
//      string vsdir = Registry.GetValue("")
    }

    void KillVsWorker(object sender, DoWorkEventArgs e)
    {
      var vsProcesses = new List<Process>(Process.GetProcessesByName(kVsName));

      if (vsProcesses.Count == 0) {
        Finish(MainFormResultCode.Ok);
      }

      DialogResult autoClose = (DialogResult) Invoke(new InvokeDialogHandler( () => AutoCloseMessage() ));

      try {
        switch (autoClose) {
          case DialogResult.Yes:
            Invoke(
                new InvokeHandler(
                    () => message_.Text = "Closing Visual Studio..."));
            foreach (var vsProcess in vsProcesses) {
              vsProcess.CloseMainWindow();
            }
            break;
          case DialogResult.No:
            break;
          case DialogResult.Cancel:
            Finish(MainFormResultCode.UserCancelled);
            break;
        }

        List<Process> deaders = new List<Process>();
        while (vsProcesses.Count > 0) {
          foreach (var vsProcess in vsProcesses) {
            if (vsProcess.HasExited) {
              deaders.Add(vsProcess);
            }
          }
          foreach (var process in deaders) {
            vsProcesses.Remove(process);
            process.Dispose();
          }
          deaders.Clear();
        }
      } catch (Exception ex) {
        Error = ex;
        Finish(MainFormResultCode.Error);
        return;
      }
      Finish(MainFormResultCode.Ok);
    }

    private DialogResult AutoCloseMessage() {
      return MessageBox.Show(
          this,
          "Installation cannot proceed until Visual Studio is closed.\n" +
          "Close Visual Studio automatically?\n" +
          "(Select 'Yes' to close Visual Studio automatically. Select 'No' to close it yourself. Select 'Cancel' to abort the install process.",
          "Installer",
          MessageBoxButtons.YesNoCancel,
          MessageBoxIcon.Question);
    }

    private delegate void InvokeHandler();

    private delegate DialogResult InvokeDialogHandler();
    private void OnFinished(MainFormResultCode code) {
      ResultCode = code;
      Close();
    }

    private void Finish(MainFormResultCode code) {
      this.Invoke(new InvokeHandler(() => OnFinished(code)));
    }

    void TestWorker(object sender, DoWorkEventArgs e)
    {
      System.Threading.Thread.Sleep(5000);
      Finish(MainFormResultCode.Ok);
    }

    private void button1_Click(object sender, EventArgs e)
    {
      OnFinished(MainFormResultCode.UserCancelled);
    }
  }
}
