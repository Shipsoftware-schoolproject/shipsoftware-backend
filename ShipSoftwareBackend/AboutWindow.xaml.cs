using System.Windows;
using System.Diagnostics;
using System.Windows.Navigation;

namespace ShipSoftwareBackend
{
    /// <summary>
    /// Interaction logic for AboutWindow.xaml
    /// </summary>
    public partial class AboutWindow : Window
    {
        public AboutWindow(string programVer)
        {
            InitializeComponent();
            lblVersion.Content = "Version: " + programVer;
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
        }
    }
}
