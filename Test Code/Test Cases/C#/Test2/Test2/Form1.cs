using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Test2
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            serialPort1.Open();
        }

        private void sendVals_Click(object sender, EventArgs e)
        {
            //Send the circumferences to the neck, chest, waist and hips
            string m1 = "<" + neckTbox.Text + "," + chestTbox.Text + "," + waistTbox.Text + "," + hipsTbox.Text + ">";
            serialPort1.Write(m1);
        }
    }
}
