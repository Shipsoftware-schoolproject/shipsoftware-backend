using System;
using System.IO;
using System.Xml;

namespace ShipSoftwareBackend
{
    public class Configuration
    {
        private string CONFIG_FILE = "configuration.xml";
        public string API_URL = "http://api.aprs.fi/api/get?";
        public int API_KEY_LENGTH = 21;
        private string api_key;
        private int log_size;
        private string sql_database;
        private string sql_username;
        private string sql_password;
        private string sql_hostname;

        public string API_KEY
        {
            private set { api_key = value; }
            get { return api_key; }
        }

        public int LOG_SIZE
        {
            private set { log_size = value; }
            get { return log_size; }
        }

        public string SQL_DATABASE
        {
            private set { sql_database = value; }
            get { return sql_database; }
        }

        public string SQL_USERNAME
        {
            private set { sql_username = value; }
            get { return sql_username; }
        }

        public string SQL_PASSWORD
        {
            private set { sql_password = value; }
            get { return sql_password; }
        }

        public string SQL_HOSTNAME
        {
            private set { sql_hostname = value; }
            get { return sql_hostname; }
        }

        public int ValidateConfiguration(string sql_database, string sql_username, string sql_password, string sql_hostname, string api_key, int log_size)
        {
            // Database name
            if (sql_database.Length < 1)
            {
                return 1;
            }
            // Hostname
            else if (sql_hostname.Length < 3)
            {
                return 2;
            }
            // API key
            else if (api_key.Length < API_KEY_LENGTH)
            {
                return 3;
            }
            else if (api_key.Length > API_KEY_LENGTH)
            {
                return 4;
            }
            // Username and password
            else if (sql_username == "" && sql_password == "")
            {
                return 5;
            }
            // Username
            else if (sql_username == "")
            {
                return 6;
            }
            // Password
            else if (sql_password == "")
            {
                return 7;
            }

            return 0;
        }

        public int LoadConfiguration()
        {
            XmlDocument xml = new XmlDocument();
            int retVal = 0;

            try
            {
                xml.Load(@CONFIG_FILE);
            }
            catch (Exception ex)
            {
                if (ex is FileNotFoundException)
                {
                    log_size = 20;
                    return -1;
                }
                else if (ex is XmlException)
                {
                    return -2;
                }
            }

            // Database name
            if (xml.GetElementsByTagName("database").Count == 1)
            {
                sql_database = xml.GetElementsByTagName("database")[0].InnerText.ToString().Trim();
            }
            else
            {
                retVal = -2;
            }

            // Username
            if (xml.GetElementsByTagName("username").Count == 1)
            {
                sql_username = xml.GetElementsByTagName("username")[0].InnerText.ToString().Trim();
            }
            else
            {
                retVal = -2;
            }

            // Password
            if (xml.GetElementsByTagName("password").Count == 1)
            {
                sql_password = xml.GetElementsByTagName("password")[0].InnerText.ToString().Trim();
            }
            else
            {
                retVal = -2;
            }

            // Hostname
            if (xml.GetElementsByTagName("hostname").Count == 1)
            {
                sql_hostname = xml.GetElementsByTagName("hostname")[0].InnerText.ToString().Trim();
            }
            else
            {
                retVal = -2;
            }

            // API key
            if (xml.GetElementsByTagName("apikey").Count == 1)
            {
                api_key = xml.GetElementsByTagName("apikey")[0].InnerText.ToString().Trim();
            }
            else
            {
                retVal = -2;
            }

            // Log size
            if (xml.GetElementsByTagName("log_size").Count == 1)
            {
                string tmp = xml.GetElementsByTagName("log_size")[0].InnerText.ToString().Trim();
                try
                {
                    log_size = int.Parse(tmp);
                }
                catch
                {
                    log_size = 20;
                }
                 
            }
            else
            {
                log_size = 20;
            }

            if (retVal != 0)
            {
                return -2;
            }

            return ValidateConfiguration(sql_database, sql_username, sql_password, sql_hostname, API_KEY, log_size);
        }

        public bool SaveConfiguration(string database, string username, string password, string hostname, string api_key, int log_size)
        {
            XmlDocument xml = new XmlDocument();
            XmlDeclaration declaration = xml.CreateXmlDeclaration("1.0", "UTF-8", null);

            XmlElement rootElement = xml.DocumentElement;
            xml.InsertBefore(declaration, rootElement);

            XmlElement root = xml.CreateElement("xml");
            xml.AppendChild(root);

            XmlElement elemDatabase = xml.CreateElement("database");
            elemDatabase.InnerText = database;
            xml.DocumentElement.AppendChild(elemDatabase);

            XmlElement elemUsername = xml.CreateElement("username");
            elemUsername.InnerText = username;
            xml.DocumentElement.AppendChild(elemUsername);

            XmlComment passwordComment;
            passwordComment = xml.CreateComment(" Very \"secure\" way to store password but hashing passwords was not part of this course ");
            xml.DocumentElement.AppendChild(passwordComment);

            XmlElement elemPassword = xml.CreateElement("password");
            elemPassword.InnerText = password;
            xml.DocumentElement.AppendChild(elemPassword);

            XmlElement elemHostname = xml.CreateElement("hostname");
            elemHostname.InnerText = hostname;
            xml.DocumentElement.AppendChild(elemHostname);

            XmlElement elemApiKey = xml.CreateElement("apikey");
            elemApiKey.InnerText = api_key;
            xml.DocumentElement.AppendChild(elemApiKey);

            XmlElement elemLogSize = xml.CreateElement("logsize");
            elemLogSize.InnerText = log_size.ToString();
            xml.DocumentElement.AppendChild(elemLogSize);

            XmlWriterSettings settings = new XmlWriterSettings();
            settings.Indent = true;
            settings.WriteEndDocumentOnClose = true;

            try
            {
                XmlWriter writer = XmlWriter.Create(@CONFIG_FILE, settings);
                xml.Save(writer);
                sql_database = database;
                sql_username = username;
                sql_password = password;
                sql_hostname = hostname;
                this.api_key = api_key;
                this.log_size = log_size;
                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
