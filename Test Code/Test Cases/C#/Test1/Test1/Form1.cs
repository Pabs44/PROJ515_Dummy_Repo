using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Test1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            serialPort1.Open();
        }

        private void sendB_Click(object sender, EventArgs e)
        {
            //Send the mm value to front chest
            string m1 = "B" + mmB.Text;
            serialPort1.Write(m1);
        }

        private void sendC_Click(object sender, EventArgs e)
        {
            //Send the mm value to front waist
            string m1 = "C" + mmC.Text;
            serialPort1.Write(m1);
        }

        private void sendD_Click(object sender, EventArgs e)
        {
            //Send the mm value to left chest
            string m1 = "D" + mmD.Text;
            serialPort1.Write(m1);
        }

        private void sendE_Click(object sender, EventArgs e)
        {
            //Send the mm value to left waist
            string m1 = "E" + mmE.Text;
            serialPort1.Write(m1);
        }

        private void sendF_Click(object sender, EventArgs e)
        {
            //Send the mm value to left hips
            string m1 = "F" + mmF.Text;
            serialPort1.Write(m1);
        }

        private void sendG_Click(object sender, EventArgs e)
        {
            //Send the mm value to right chest
            string m1 = "G" + mmG.Text;
            serialPort1.Write(m1);
        }

        private void sendH_Click(object sender, EventArgs e)
        {
            //Send the mm value to right waist
            string m1 = "H" + mmH.Text;
            serialPort1.Write(m1);
        }

        private void sendI_Click(object sender, EventArgs e)
        {
            //Send the mm value to right hips
            string m1 = "I" + mmI.Text;
            serialPort1.Write(m1);
        }

        private void sendJ_Click(object sender, EventArgs e)
        {
            //Send the mm value to back chest
            string m1 = "J" + mmJ.Text;
            serialPort1.Write(m1);
        }

        private void sendK_Click(object sender, EventArgs e)
        {
            //Send the mm value to back waist
            string m1 = "K" + mmK.Text;
            serialPort1.Write(m1);
        }

        private void sendL_Click(object sender, EventArgs e)
        {
            //Send the mm value to back hips
            string m1 = "L" + mmL.Text;
            serialPort1.Write(m1);
        }

        private void toMinB_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position front chest
            serialPort1.Write("b");
        }

        private void toMinC_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position front waist
            serialPort1.Write("c");
        }

        private void toMinD_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position left chest
            serialPort1.Write("d");
        }

        private void toMinE_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position left waist
            serialPort1.Write("e");
        }

        private void toMinF_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position left hip
            serialPort1.Write("f");
        }

        private void toMinG_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position right chest
            serialPort1.Write("g");
        }

        private void toMinH_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position right waist
            serialPort1.Write("g");
        }

        private void toMinI_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position right chest
            serialPort1.Write("i");
        }

        private void toMinK_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position back waist
            serialPort1.Write("k");
        }

        private void toMinJ_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position back chest
            serialPort1.Write("j");
        }

        private void toMinL_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position back hip
            serialPort1.Write("l");
        }

        private void resetTest_Click(object sender, EventArgs e)
        {
            //Send command to move to minimum position back chest
            serialPort1.Write("a");
        }

        private void sendTest_Click(object sender, EventArgs e)
        {
            //Send the mm value to back chest
            string m1 = "A" + mmTest.Text;
            serialPort1.Write(m1);
        }
    }
}
