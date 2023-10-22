using System.Collections.Generic;
using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;
using System.Drawing;

using FastColoredTextBoxNS;
using System.IO;

namespace DebugExecutor
{
    public partial class Form1 : Form
    {
        FastColoredTextBox textbox;
        AutocompleteMenu popupMenu;

        public Form1()
        {
            // create
            textbox = new FastColoredTextBox();
            popupMenu = new AutocompleteMenu(textbox);

            // setup dock
            textbox.Dock = DockStyle.Fill;

            // setup language
            textbox.Language = Language.Luau;

            // setup colours
            textbox.BackColor = Color.FromArgb(30, 26, 10);
            textbox.IndentBackColor = Color.FromArgb(30, 26, 10);
            textbox.LineNumberColor = Color.White;
            textbox.ForeColor = Color.White;

            // setup font
            textbox.Font = new Font("Arial", 12);

            // setup autocomplete
            textbox.KeyDown += (s, e) =>
            {
                if (e.KeyCode == Keys.Enter)
                {
                    e.Handled = true;
                    textbox.InsertText("\n");
                }
                else
                {
                    popupMenu.Show(true);
                }
            };

            // setup autocomplete
            popupMenu.MinFragmentLength = 2;
            popupMenu.Opacity = 0.5;
            popupMenu.AllowTabKey = true;

            var randomWords = new List<string>
            {
                "for", "do", "function", "end", "in", "pairs", "ipairs", // default lua stuff

                "print", "warn", "error", // logging stuff

                "identity", "printidentity", "version", "getidentity", // identity stuff

                "createscript", "loadstring", // script chunk stuff

                "wait", // wait & resume stuff

                "readfile", "listfiles", "writefile", // file stuff
                "makefolder", "appendfile", "isfile", "isfolder",
                "delfile", "delfolder", "dofile",

                "rconsoleclear", "rconsolecreate", "rconsoledestroy",
                "rconsoleprint", "rconsoleinput", "rconsoletitle",

                "time", "setclipboard", "getfps", "setfps", "isactive" // misc stuff
            };

            popupMenu.Items.SetAutocompleteItems(randomWords);
            popupMenu.Items.MaximumSize = new System.Drawing.Size(200, 300);
            popupMenu.Items.Width = 200;

            // add to form
            Controls.Add(textbox);

            // setup client path/pipes
            FileIO.SetupClientPath();

            // setup form
            InitializeComponent();
        }

        private void Textbox_KeyDown(object sender, KeyEventArgs e)
        {
            throw new NotImplementedException();
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
        private void execute(object sender, EventArgs e)
        {
            if (!BetronaPipes.Execute(textbox.Text))
            {
                if (BetronaPipes.InjectDLL() && counter >= 3)
                {
                    counter++;
                    execute(sender, e);
                }
                else
                {
                    counter = 0;

                    MessageBox.Show($"Fatal injection error (Try inject first)", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void clear(object sender, EventArgs e)
        {
            textbox.Clear();
        }

        private void openFile(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog()
            {
                Title = "Open File",
                CheckFileExists = true,
            };

            if (dialog.ShowDialog() == DialogResult.OK)
                textbox.Text = File.ReadAllText(dialog.FileName);
        }

        private void executeFile(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog()
            {
                Title = "Execute File",
                CheckFileExists = true,
            };

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                if (!BetronaPipes.Execute(File.ReadAllText(dialog.FileName)))
                {
                    MessageBox.Show($"Fatal injection error (Try inject first)", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void saveFile(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog()
            {
                Title = "Save File",
                CheckFileExists = true
            };

            if (dialog.ShowDialog() == DialogResult.OK)
            {

            }
        }
    }
}
