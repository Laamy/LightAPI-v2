using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;

namespace DebugExecutor
{
    public partial class Form1 : Form
    {
        FastColoredTextBoxNS.FastColoredTextBox textbox;

        public Form1()
        {
            textbox = new FastColoredTextBoxNS.FastColoredTextBox();
            textbox.Dock = DockStyle.Fill;
            textbox.Language = FastColoredTextBoxNS.Language.Lua;
            Controls.Add(textbox);

            FileIO.SetupClientPath();

            InitializeComponent();
        }

        private void ExitAppBtn(object sender, System.EventArgs e)
            => Application.Exit();

        private void KillMinecraftBtn(object sender, System.EventArgs e)
            => Process.GetProcessesByName("Minecraft.Windows")[0].Kill();

        private void InjectBtn(object sender, System.EventArgs e)
        {
            if (!BetronaPipes.InjectDLL())
            {
                MessageBox.Show($"Failed to inject {Text} (Already injected?)", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        int counter = 0;
        private void executeToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            if (!BetronaPipes.Execute(textbox.Text))
            {
                if (BetronaPipes.InjectDLL() && counter >= 3)
                {
                    Thread.Sleep(50);
                    counter++;
                    executeToolStripMenuItem_Click(sender, e);
                }
                else
                {
                    counter = 0;

                    MessageBox.Show($"Fatal injection error (Try inject first)", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }
    }
}
