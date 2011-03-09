namespace MsiHelp
{
  partial class MainForm
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
      this.worker_ = new System.ComponentModel.BackgroundWorker();
      this.message_ = new System.Windows.Forms.Label();
      this.progress_ = new System.Windows.Forms.ProgressBar();
      this.button1 = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // message_
      // 
      this.message_.AutoSize = true;
      this.message_.Location = new System.Drawing.Point(12, 9);
      this.message_.Name = "message_";
      this.message_.Size = new System.Drawing.Size(35, 13);
      this.message_.TabIndex = 0;
      this.message_.Text = "label1";
      // 
      // progress_
      // 
      this.progress_.Location = new System.Drawing.Point(15, 25);
      this.progress_.Name = "progress_";
      this.progress_.Size = new System.Drawing.Size(328, 23);
      this.progress_.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
      this.progress_.TabIndex = 1;
      // 
      // button1
      // 
      this.button1.DialogResult = System.Windows.Forms.DialogResult.Cancel;
      this.button1.Location = new System.Drawing.Point(140, 54);
      this.button1.Name = "button1";
      this.button1.Size = new System.Drawing.Size(75, 23);
      this.button1.TabIndex = 2;
      this.button1.Text = "&Cancel";
      this.button1.UseVisualStyleBackColor = true;
      this.button1.Click += new System.EventHandler(this.button1_Click);
      // 
      // MainForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.CancelButton = this.button1;
      this.ClientSize = new System.Drawing.Size(355, 82);
      this.ControlBox = false;
      this.Controls.Add(this.button1);
      this.Controls.Add(this.progress_);
      this.Controls.Add(this.message_);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Name = "MainForm";
      this.ShowIcon = false;
      this.ShowInTaskbar = false;
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
      this.Text = "Installer";
      this.TopMost = true;
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.ComponentModel.BackgroundWorker worker_;
    private System.Windows.Forms.Label message_;
    private System.Windows.Forms.ProgressBar progress_;
    private System.Windows.Forms.Button button1;
  }
}