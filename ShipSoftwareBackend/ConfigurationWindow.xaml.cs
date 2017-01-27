using System.Windows;
using System.Windows.Controls;

namespace ShipSoftwareBackend
{
    /// <summary>
    /// Interaction logic for UpdateSQLcredsWindow.xaml
    /// </summary>
    public partial class ConfigurationWindow : Window
    {
        Button btnStart;
        Configuration config;
        bool disableButton;

        public ConfigurationWindow(Button button, ref Configuration config, bool disableButton)
        {
            InitializeComponent();
            btnStart = button;
            this.config = config;
            this.disableButton = disableButton;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // SQL configs
            txtDatabase.Text = config.SQL_DATABASE;
            txtUsername.Text = config.SQL_USERNAME;
            txtPassword.Password = config.SQL_PASSWORD;
            txtHostname.Text = config.SQL_HOSTNAME;
            cmbDatabaseSystem.SelectedIndex = cmbDatabaseSystem.Items.IndexOf((ComboBoxItem)this.cmbDatabaseSystem.FindName(config.SQL_DATABASE_SYSTEM));
            // API key
            txtApiKey.Text = config.API_KEY;
            // Log size
            txtLogSize.Text = config.LOG_SIZE.ToString();
        }

        private bool AskDialog(string message)
        {
            MessageBoxButton btnMessageBox = MessageBoxButton.YesNo;
            MessageBoxResult msgBoxResult = MessageBox.Show(message, "Warning", btnMessageBox);
            switch (msgBoxResult)
            {
                case MessageBoxResult.Yes:
                    return false;
                case MessageBoxResult.No:
                    return true;
            }

            return false;
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            // SQL configuration
            string database = txtDatabase.Text.Trim();
            string username = txtUsername.Text.Trim();
            string password = txtPassword.Password.Trim();
            string hostname = txtHostname.Text.Trim();
            string database_system;
            if (cmbDatabaseSystem.SelectedIndex == -1)
            {
                MessageBox.Show("No database system selected!", "Error");
                return;
            }
            else
            {
                database_system = ((ComboBoxItem)cmbDatabaseSystem.SelectedItem).Name.ToString();
            }
            // API key
            string API_key = txtApiKey.Text.Trim();
            // Log size
            int log_size;

            try
            {
                log_size = int.Parse(txtLogSize.Text.ToString());
                if (log_size < 0)
                {
                    MessageBox.Show("Log size must be a positive number.");
                    return;
                }
            }
            catch
            {
                MessageBox.Show("Invalid log size.");
                return;
            }

            int retVal = config.ValidateConfiguration(database, username, password, hostname, database_system, API_key, log_size);
            switch (retVal)
            {
                case 1:
                    MessageBox.Show("SQL server database name is not valid!", "Error");
                    if (disableButton)
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = "Update SQL configuration to start background service.";
                    }
                    return;
                case 2:
                    MessageBox.Show("SQL server hostname is not valid!", "Error");
                    if (disableButton)
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = "Update SQL configuration to start background service.";
                    }
                    return;
                case 3:
                    MessageBox.Show("API key is too short!", "Error");
                    if (disableButton)
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = "Update your API key to start background service.";
                    }
                    return;
                case 4:
                    MessageBox.Show("API key is too long!");
                    if (disableButton)
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = "Update your API key to start background service.";
                    }
                    return;
                case 5:
                    if (AskDialog("Are you sure you want to use empty username and password?")) {
                        return;
                    }
                    break;
                case 6:
                    if (AskDialog("Are you sure you want to use empty username?")) {
                        return;
                    }
                    break;
                case 7:
                    if (AskDialog("Are you sure you want to use empty password?")) {
                        return;
                    }
                    break;
                case 8:
                    MessageBox.Show("Invalid database system!", "Error");
                    if (disableButton)
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = "Choose database system to start background service.";
                    }
                    return;
            }

            if (config.SaveConfiguration(database, username, password, hostname, database_system, API_key, log_size))
            {
                MessageBox.Show("Configuration updated.", "Ok");
                DialogResult = true;
                btnStart.IsEnabled = true;
                btnStart.ToolTip = null;
                Close();
            }
            else
            {
                MessageBox.Show("Could not save configuration!", "Error");
                return;
            }
        }
    }
}
