using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Google.NaClVsx.Installation
{
  public partial class ProgressForm : Form
  {
    public ProgressForm()
    {
      InitializeComponent();
    }

    public string Message {
      get { return message_.Text; }
      set { message_.Text = value; }
    }

    public void Cancel() {
      if (Cancelled != null) {
        Cancelled(this, null);
      }
      this.DialogResult = DialogResult.Cancel;
      Close();
    }

    public event EventHandler Cancelled;

    private void cancel__Click(object sender, EventArgs e)
    {
      Cancel();
    }


  }
}
