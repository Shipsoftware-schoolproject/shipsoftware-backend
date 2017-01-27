using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data.SqlClient;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Xml;
using MySql.Data.MySqlClient;

namespace ShipSoftwareBackend
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static string programVer = "0.1.0";
        Thread apiThread;
        volatile bool threadRun = true;
        // Text colors
        SolidColorBrush colorOK = new SolidColorBrush(Colors.Green);
        SolidColorBrush colorERROR = new SolidColorBrush(Colors.Red);
        Configuration config = new Configuration();

        public MainWindow()
        {
            InitializeComponent();
            // Add CollectionChanged notify to lstErrors
            ((INotifyCollectionChanged)lstErrors.Items).CollectionChanged += lstErrors_CollectionChanged;
            apiThread = new Thread(APICallThread);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // load SQL credentials
            int retVal = config.LoadConfiguration();
            switch (retVal)
            {
                case -1:
                    if (ErrorDialog("Could not load configuration file.\n\nDo you want to update your configuration now?", "You must update your configuration to use the program.", true, "Update your configuration to start background service."))
                    {
                        Log("Could not load configuration.");
                    }
                    break;
                case -2:
                    if (ErrorDialog("Configuration file is corrupted.\n\nDo you want to update your configuration now?", "You must update your configuration to use the program.", true, "Update your configuration to start background service."))
                    {
                        Log("Configuration is corrupted.");
                    }
                    break;
                case 1:
                    if (ErrorDialog("SQL server database name is not valid.\n\nDo you want to update it now?", "You must provide valid SQL server database name to use the program.", true, "Update SQL configuration to start background service."))
                    {
                        Log("Invalid SQL server hostname.");
                    }
                    break;
                case 2:
                    if (ErrorDialog("SQL server hostname is not valid.\n\nDo you want to update it now?", "You must provide valid SQL server hostname to use the program.", true, "Update SQL configuration to start background service."))
                    {
                        Log("Invalid SQL server hostname.");
                    }
                    break;
                case 3:
                    if (ErrorDialog("API key is too short.\n\nDo you want to update your API key now?", "You must provide valid API key to use the program.", true, "Update your API key to start background service."))
                    {
                        Log("Invalid API key: too short.");
                    }
                    break;
                case 4:
                    if (ErrorDialog("API key is too long.\n\nDo you want to update your API key now?", "You must provide valid API key to use the program.", true, "Update your API key to start background service."))
                    {
                        Log("Invalid API key: too long.");
                    }
                    break;
                case 5:
                    if (ErrorDialog("You're using empty username and password for SQL server.\n\nDo you want to change them now?", "", false, null))
                    {
                        Log("Warning: using empty username and password.");
                    }
                    break;
                case 6:
                    if (ErrorDialog("You're using empty username for SQL server.\n\nDo you want to change it now?", "", false, null))
                    {
                        Log("Warning: using empty username.");
                    }
                    break;
                case 7:
                    if (ErrorDialog("You're using empty password for SQL server.\n\nDo you want to change it now?", "", false, null))
                    {
                        Log("Warning: using empty password.");
                    }
                    break;
                case 8:
                    if (ErrorDialog("Invalid database system.\n\nDo you want to choose it now?", "You must choose the database system to use the program.", true, "Choose database system to start background service."))
                    {
                        Log("Invalid database system selected.");
                    }
                    break;
            }
        }

        // Dialog for LoadConfiguration errors
        bool ErrorDialog(string errorMessage, string cancelMessage, bool disable, string tooltip)
        {
            string caption;

            if (disable)
            {
                caption = "Error";
            }
            else
            {
                caption = "Warning";
            }

            MessageBoxButton btnMessageBox = MessageBoxButton.YesNo;
            MessageBoxResult msgBoxResult = MessageBox.Show(errorMessage, caption, btnMessageBox);
            switch (msgBoxResult)
            {
                case MessageBoxResult.Yes:
                    ConfigurationWindow configWindow = new ConfigurationWindow(btnStart, ref config, true);
                    if (configWindow.ShowDialog() == true)
                    {
                        btnStart.IsEnabled = true;
                        btnStart.ToolTip = null;
                        return false;
                    }
                    else
                    {
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = tooltip;
                        return true;
                    }
                case MessageBoxResult.No:
                    if (disable)
                    {
                        MessageBox.Show(cancelMessage, "Warning");
                        btnStart.IsEnabled = false;
                        btnStart.ToolTip = tooltip;
                    }
                    return true;
            }
            return true;
        }

        void UpdateUpdated(bool success)
        {
            SolidColorBrush color;
            string lblUpdatedText = "";

            if (success)
            {
                color = colorOK;
            }
            else
            {
                color = colorERROR;
                lblUpdatedText = " - FAILED";
            }

            Dispatcher.BeginInvoke(new Action(delegate ()
            {
                lblUpdated.Content = DateTime.Now.ToString() + lblUpdatedText;
                lblUpdated.Foreground = color;
            }));
        }

        void Log(string logMessage)
        {
            Dispatcher.BeginInvoke(new Action(delegate ()
            {
                lstErrors.Items.Insert(0, DateTime.Now.ToString() + " - " + logMessage);
            }));
        }

        void UpdateStatus(SolidColorBrush color, string text)
        {
            Dispatcher.BeginInvoke(new Action(delegate ()
            {
                lblRunning.Content = text;
                lblRunning.Foreground = color;
            }));
        }

        dynamic sqlCon()
        {
            if (config.SQL_DATABASE_SYSTEM == "mssql")
            {
                return new SqlConnection("Data source = " + config.SQL_HOSTNAME + "; Initial Catalog = " + config.SQL_DATABASE + "; User ID = " + config.SQL_USERNAME + "; Password = " + config.SQL_PASSWORD + ";");
            }
            else
            {
                return new MySqlConnection("Server=" + config.SQL_HOSTNAME + ";Database=" + config.SQL_DATABASE + ";Username=" + config.SQL_USERNAME + ";Password=" + config.SQL_PASSWORD + ";");
            }
        }

        dynamic sqlCommand(string query, dynamic connection)
        {
            if (config.SQL_DATABASE_SYSTEM == "mssql")
            {
                return new SqlCommand(query, connection);
            }
            else
            {
                return new MySqlCommand(query, connection);
            }
        }

        // get all ships MMSI's from database
        List<List<string>> GetShips()
        {
            List<List<string>> ships = new List<List<string>>();
            dynamic con = sqlCon();
            dynamic reader;
            dynamic query = sqlCommand("SELECT ShipID, MMSI FROM Ships", con);

            try
            {
                con.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
                return null;
            }

            try
            {
                reader = query.ExecuteReader();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
                return null;
            }

            while (reader.Read())
            {
                List<string> ship = new List<string> { reader["ShipID"].ToString(), reader["MMSI"].ToString() };
                ships.Add(ship);
            }

            con.Close();

            if (ships.Count < 1)
            {
                MessageBox.Show("Database does not contain any ships.", "Error");
                return null;
            }

            return ships;
        }

        // fetch data from API
        string GetData(bool debug, List<List<string>> ships)
        {
            if (debug)
            {
                // COLOMBO EXPRESS
                //return "<xml><command>get</command><result>ok</result><what>loc</what><found>1</found><entries><entry><class>i</class><name>COLOMBO EXPRESS</name><mmsi>211433000</mmsi><type>a</type><time>1478318422</time><lasttime>1478318422</lasttime><lat>50.710462</lat><lng>-0.967245</lng><course>108.1</course><speed>25.1872</speed><srccall>DIHC</srccall><dstcall>ais</dstcall><comment>UKSOU &gt; USNYC (ETA Nov05 09:00)</comment><imo>295244</imo><vesselclass>79</vesselclass><navstat>0</navstat><heading>109</heading><length>336</length><width>42</width><draught>12.6</draught><ref_front>21</ref_front><ref_left>21</ref_left><path>M6FGP</path></entry></entries></xml>";
                // WASA EXPRESS test with comment being only the destination
                //return "<xml><command>get</command><result>ok</result><what>loc</what><found>1</found><entries><entry><class>i</class><name>WASA EXPRESS</name><mmsi>230636000</mmsi><type>a</type><time>1477915166</time><lasttime>1477915166</lasttime><lat>63.10701</lat><lng>21.41677</lng><course>114</course><speed>23.3352</speed><srccall>OJQB</srccall><dstcall>ais</dstcall><comment>HELSINKI (ETA Nov10 10:30)</comment><imo>8000226</imo><vesselclass>106</vesselclass><navstat>0</navstat><heading>113</heading><length>141</length><width>23</width><draught>4.6</draught><ref_front>23</ref_front><ref_left>10</ref_left><path>OH6AA</path></entry></entries></xml>";
                // WASA Express
                //return "<xml><command>get</command><result>ok</result><what>loc</what><found>1</found><entries><entry><class>i</class><name>WASA EXPRESS</name><mmsi>230636000</mmsi><type>a</type><time>1477915166</time><lasttime>1477915166</lasttime><lat>63.10701</lat><lng>21.41677</lng><course>114</course><speed>23.3352</speed><srccall>OJQB</srccall><dstcall>ais</dstcall><comment>VAASA-UMEA-VAASA (ETA 20161031123000)</comment><imo>8000226</imo><vesselclass>106</vesselclass><navstat>0</navstat><heading>113</heading><length>141</length><width>23</width><draught>4.6</draught><ref_front>23</ref_front><ref_left>10</ref_left><path>OH6AA</path></entry></entries></xml>";
                // TAURUS (ship with no destination)
                //return "<xml><command>get</command><result>ok</result><what>loc</what><found>1</found><entries><entry><class>i</class><name>TAURUS</name><mmsi>230660000</mmsi><type>a</type><time>1478782285</time><lasttime>1478782285</lasttime><lat>63.8647614</lat><lng>23.0213261</lng><course>107.2</course><speed>2.778</speed><srccall>OJRL</srccall><dstcall>ais</dstcall><comment>HARBOUR TUG CH 12</comment><imo>9261190</imo><vesselclass>52</vesselclass><navstat>0</navstat><heading>296</heading><length>23</length><width>8</width><draught>4.0</draught><ref_front>10</ref_front><ref_left>6</ref_left><path>OH6AI</path></entry></entries></xml>";
                // Two ships, WASA EXPRESS & COLUBMO EXPRESS
                return "<xml><command>get</command><result>ok</result><what>loc</what><found>2</found><entries><entry><class>i</class><name>WASA EXPRESS</name><mmsi>230636000</mmsi><type>a</type><time>1479212440</time><lasttime>1479212440</lasttime><lat>63.0855466666667</lat><lng>21.5493666666667</lng><course>284</course><speed>2.5928</speed><srccall>OJQB</srccall><dstcall>ais</dstcall><comment>VAASA-UMEA-VAASA (ETA 20161115123000)</comment><imo>8000226</imo><vesselclass>106</vesselclass><navstat>0</navstat><heading>218</heading><length>141</length><width>23</width><draught>4.6</draught><ref_front>23</ref_front><ref_left>10</ref_left><path>OH6AA</path></entry><entry><class>i</class><name>COLOMBO EXPRESS</name><mmsi>211433000</mmsi><type>a</type><time>1479124931</time><lasttime>1479124931</lasttime><lat>39.043765</lat><lng>-74.001985</lng><course>202.3</course><speed>13.1492</speed><srccall>DIHC</srccall><dstcall>ais</dstcall><comment>UKSOU &gt; USNYC (ETA Nov05 09:00)</comment><imo>295244</imo><vesselclass>0</vesselclass><navstat>0</navstat><heading>204</heading><length>336</length><width>42</width><draught>0.0</draught><ref_front>0</ref_front><ref_left>0</ref_left><path>Spleen</path></entry></entries></xml>";
            }

            string shipsMMSI = null;
            WebClient client = new WebClient();
            Stream data = null;
            client.Headers.Add("user-agent", "shipsoftware-backend-schoolproject/" + programVer + " (+https://github.com/Shipsoftware-schoolproject/shipsoftware-backend)");

            for (int i = 0; i < ships.Count; i++)
            {
                if (shipsMMSI != null)
                {
                    shipsMMSI = shipsMMSI + "," + ships[i][1];
                }
                else
                {
                    shipsMMSI = ships[i][1];
                }
            }

            try
            {
                data = client.OpenRead(config.API_URL + "name=" + shipsMMSI + "&what=loc&apikey=" + config.API_KEY + "&format=xml");
            }
            catch (WebException)
            {
                data.Close();
                return null;
            }
            StreamReader reader = new StreamReader(data);
            string result = reader.ReadToEnd();

            data.Close();
            reader.Close();

            return result;
        }

        // parse ship route
        private string[] getRoute(string route)
        {
            string startingPoint = null;
            string endingPoint = null;

            // Needs more testing if there is other formats than the one listed below
            // Regex for starting and ending point in format of "START-END-START"
            if (Regex.IsMatch(route, ".*?-.*?"))
            {
                startingPoint = route.Substring(0, route.IndexOf('-'));
                endingPoint = route.Substring(route.IndexOf('-') + 1, route.IndexOf('-') - 1);
            }
            // Regex for starting and ending point in format of "START > END"
            else if (Regex.IsMatch(route, ".*? > .*?"))
            {
                startingPoint = route.Substring(0, route.IndexOf('>') - 1);
                endingPoint = route.Substring(route.IndexOf('>') + 2, ((route.IndexOf('(') - 1) - (route.IndexOf('>') + 2)));
            }
            // Otherwise assume that API reported only destination (ignore "destinations" which only have ETA or are actually comments (eg. radio channel to contact))
            else if (route[0] != '(' && route[route.Length - 1] == ')')
            {
                endingPoint = route.Substring(0, route.IndexOf('(') - 1);
            }

            string[] ret = { startingPoint, endingPoint };
            return ret;
        }

        private XmlDocument loadXML(string data)
        {
            XmlDocument xml = new XmlDocument();
            try
            {
                xml.LoadXml(data);
            }
            catch
            {
                xml = null;
            }

            return xml;
        }

        // backend thread which handles all API logic
        private void APICallThread()
        {
            int sleepTime = 300;
            int slept = sleepTime;

            while (true)
            {
                if(slept >= sleepTime) {
                    slept = 0;
                    UpdateStatus(colorOK, "Running (updating..)");
                    // get ships from database
                    List<List<string>> ships = GetShips();
                    if (ships == null)
                    {
                        threadRun = false;
                        ThreadShutdown();
                        break;
                    }
                    // debug "switch", set to false to get real data from the real API
                    string data = GetData(false, ships);

                    if (data == null)
                    {
                        Log("Error: API responded too slow response?");
                        UpdateUpdated(false);
                    }
                    else
                    {
                        XmlDocument xml = loadXML(data);
                        if (xml == null)
                        {
                            Log("Error: API returned malformed data..");
                            UpdateUpdated(false);
                        }

                        if (xml != null && xml.GetElementsByTagName("result")[0].InnerText.ToString() != "ok")
                        {
                            if (xml != null)
                            {
                                string errorMessage = xml.GetElementsByTagName("description")[0].InnerText.ToString();
                                Log("API error: \"" + errorMessage + "\"");
                                UpdateUpdated(false);

                                // invalid API key
                                if (errorMessage == "invalid API key")
                                {
                                    ThreadShutdown();
                                    Dispatcher.BeginInvoke(new Action(delegate ()
                                    {
                                        btnStart.IsEnabled = false;
                                        btnStart.ToolTip = "Update your API key.";
                                    }));
                                    threadRun = false;
                                    Log("Invalid API key! Check your API key.");
                                    MessageBox.Show("Invalid API key! Check your API key.");
                                }
                            }
                        }
                        else
                        {
                            int entries = int.Parse(xml.GetElementsByTagName("found")[0].InnerText.ToString());

                            if (entries <= 0)
                            {
                                Log("API query did not return any ships.");
                                UpdateUpdated(true);
                            }
                            else
                            {
                                dynamic con = sqlCon();

                                try
                                {
                                    con.Open();
                                }
                                catch
                                {
                                    Log("Error: failed to connect database.");
                                }

                                if (con == null && con.State.ToString() != "Open") {
                                        Log("Error: Database connection is not open");
                                }
                                else
                                {
                                    for (int i = 0; i < entries; i++)
                                    {
                                        string name = xml.GetElementsByTagName("name")[i].InnerText.ToString();
                                        int mmsi = int.Parse(xml.GetElementsByTagName("mmsi")[i].InnerText.ToString());
                                        string latitude = xml.GetElementsByTagName("lat")[i].InnerText.ToString();
                                        string longitude = xml.GetElementsByTagName("lng")[i].InnerText.ToString();
                                        string course = xml.GetElementsByTagName("course")[i].InnerText.ToString();
                                        string speed = xml.GetElementsByTagName("speed")[i].InnerText.ToString();
                                        string comment = xml.GetElementsByTagName("comment")[i].InnerText.ToString();
                                        string[] route = getRoute(comment);
                                        string ship_route_query = null;
                                        string ShipRoutesID = null;

                                        // Ships -table: UPDATE course
                                        using (dynamic query = sqlCommand("UPDATE Ships SET Course = " + course.ToString() + ", ShipSpeed = " + speed + " WHERE ShipID = " + ships[i][0], con))
                                        {
                                            try
                                            {
                                                query.ExecuteNonQuery();
                                            }
                                            catch
                                            {
                                                Log("Error: failed to UPDATE Course and Speed for " + name + ".");
                                            }
                                        }

                                        // GPS -table
                                        using (dynamic query = sqlCommand("INSERT INTO GPS (ShipID, North, East) VALUES (" + ships[i][0] + ", " + latitude + ", " + longitude + ")", con))
                                        {
                                            try
                                            {
                                                query.ExecuteNonQuery();
                                            }
                                            catch (Exception ex)
                                            {
                                                Log("Error: failed to INSERT GPS location to database (" + name + "). " + ex.Message);
                                            }
                                        }

                                        // ShipRoutes -table
                                        if (route[0] == null && route[1] != null)
                                        {
                                            ship_route_query = "SELECT ShipRoutesID FROM ShipRoutes WHERE StartingPortID = null AND EndingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE '" + route[1] + "%')";
                                        }
                                        else if (route[0] != null && route[1] == null)
                                        {
                                            ship_route_query = "SELECT ShipRoutesID FROM ShipRoutes WHERE StartingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE '" + route[0] + "%') AND EndingPortID = null";
                                        }
                                        else if (route[0] != null && route[1] != null)
                                        {
                                            ship_route_query = "SELECT ShipRoutesID FROM ShipRoutes WHERE StartingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE '" + route[0] + "%') AND EndingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE '" + route[1] + "%')";
                                        }
                                        else
                                        {
                                            Log("Warn: Ship \"" + name + " \" do not have start neither end port, skipping route update.");
                                        }

                                        if (ship_route_query != null)
                                        {
                                            bool update = true;

                                            using (dynamic query = sqlCommand(ship_route_query, con))
                                            {
                                                try
                                                {
                                                    using (dynamic reader = query.ExecuteReader())
                                                    {
                                                        if (reader.HasRows)
                                                        {
                                                            reader.Read();
                                                            ShipRoutesID = reader["ShipRoutesID"].ToString();
                                                        }
                                                        else
                                                        {
                                                            if (route[0] == null) route[0] = "null";
                                                            if (route[1] == null) route[1] = "null";
                                                            Log("Warn: no route \"" + route[0] + " - " + route[1] + "\" for ship \"" + name + "\"");
                                                            update = false;
                                                        }
                                                    }
                                                }
                                                catch
                                                {
                                                    Log("Error: route update query failed for ship " + name);
                                                }
                                            }

                                            if (update)
                                            {
                                                using (dynamic query = sqlCommand("UPDATE Ships SET ShipRoutesID = " + ShipRoutesID + " WHERE ShipID = " + ships[i][0], con))
                                                {
                                                    try
                                                    {
                                                        query.ExecuteNonQuery();
                                                    }
                                                    catch
                                                    {
                                                        Log("Error: failed to update route for ship \"" + name + "\"");
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    con.Close();

                                    Log("Ships updated succesfully.");
                                    UpdateUpdated(true);
                                }
                            }
                        }
                    }
                }

                // terminate thread
                if (!threadRun)
                {
                    ThreadShutdown();
                    break;
                }

                // update lblRunning text
                UpdateStatus(colorOK, "Running (next update in " + (sleepTime - slept) + " seconds)");

                // Sleep 1 second
                slept++;
                Thread.Sleep(1000);
            }
        }


        // gracefull exit
        void ExitProgram(bool terminate = false)
        {
            if (apiThread.IsAlive)
            {
                threadRun = false;
                ExitMessageWindow exitMsg = new ExitMessageWindow();
                exitMsg.Owner = this;
                exitMsg.Show();
                apiThread.Join();
                exitMsg.Close();
            }

            if (terminate)
            {
                Application.Current.Shutdown();
            }
        }

        // update UI texts and colors on thread exit
        private void ThreadShutdown()
        {
            Dispatcher.BeginInvoke(new Action(delegate ()
            {
                lblRunning.Foreground = colorERROR;
                lblUpdated.Foreground = colorERROR;
                lblRunning.Content = "Not running";
                btnStart.Content = "Start";
                btnStart.IsEnabled = true;
            }));
        }

        // bntStartCalling click event
        private void btnStart_Click(object sender, RoutedEventArgs e)
        {
            if (!apiThread.IsAlive)
            {
                threadRun = true;
                apiThread = new Thread(APICallThread);
                apiThread.Start();
                btnStart.Content = "Stop";
            }
            else
            {
                btnStart.IsEnabled = false;
                threadRun = false;
                lblRunning.Content = "Stopping";
                lblRunning.Foreground = colorERROR;
            }
        }

        // lstErrors CollectionChanged event
        private void lstErrors_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add)
            {
                if (lstErrors.Items.Count > config.LOG_SIZE)
                {
                    int lastIndex = lstErrors.Items.Count - 1;
                    lstErrors.Items.RemoveAt(lastIndex);
                }
            }
        }

        // clear lstErrors
        private void btnClearLog_Click(object sender, RoutedEventArgs e)
        {
            for (int i = 0; i < lstErrors.Items.Count; i++)
            {
                lstErrors.Items.RemoveAt(i);
            }
        }

        // Main window Closing event
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            ExitProgram();
        }

        // menu Configuration click event
        private void menuConfiguration_Click(object sender, RoutedEventArgs e)
        {
            ConfigurationWindow configWindow = new ConfigurationWindow(btnStart, ref config, false);
            configWindow.ShowDialog();
        }

        // menu Exit click event
        private void menuExit_Click(object sender, RoutedEventArgs e)
        {
            ExitProgram(true);
        }

        // menu About click event
        private void menuAbout_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Shipsoftware Backend - version " + programVer + "\n\n" +
                            "Made for school project at VAMK\n" +
                            "https://github.com/Shipsoftware-schoolproject\n" +
                            "Developed by Tomi Lähteenmäki, Jori Jalkanen, Henri Schreck.\n" +
                            "(c) 2016.", "About");
        }
    }
}
