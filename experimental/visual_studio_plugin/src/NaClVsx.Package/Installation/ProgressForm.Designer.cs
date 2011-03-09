namespace Google.NaClVsx.Installation
{
  partial class ProgressForm
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ProgressForm));
      this.progress_ = new System.Windows.Forms.ProgressBar();
      this.message_ = new System.Windows.Forms.Label();
      this.cancel_ = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // progress_
      // 
      this.progress_.Location = new System.Drawing.Point(12, 25);
      this.progress_.Name = "progress_";
      this.progress_.Size = new System.Drawing.Size(300, 23);
      this.progress_.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
      this.progress_.TabIndex = 0;
      // 
      // message_
      // 
      this.message_.AutoSize = true;
      this.message_.Location = new System.Drawing.Point(12, 9);
      this.message_.Name = "message_";
      this.message_.Size = new System.Drawing.Size(35, 13);
      this.message_.TabIndex = 1;
      this.message_.Text = "label1";
      // 
      // cancel_
      // 
      this.cancel_.Location = new System.Drawing.Point(125, 54);
      this.cancel_.Name = "cancel_";
      this.cancel_.Size = new System.Drawing.Size(75, 23);
      this.cancel_.TabIndex = 2;
      this.cancel_.Text = "&Cancel";
      this.cancel_.UseVisualStyleBackColor = true;
      this.cancel_.Click += new System.EventHandler(this.cancel__Click);
      // 
      // ProgressForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(324, 83);
      this.ControlBox = false;
      this.Controls.Add(this.cancel_);
      this.Controls.Add(this.message_);
      this.Controls.Add(this.progress_);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.Name = "ProgressForm";
      this.Text = "Installer";
      this.TopMost = true;
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.ProgressBar progress_;
    private System.Windows.Forms.Label message_;
    private System.Windows.Forms.Button cancel_;
  }
}