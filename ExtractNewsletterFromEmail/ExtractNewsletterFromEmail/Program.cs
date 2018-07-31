using System;
using OpenPop.Mime;
using System.Collections.Generic;
using OpenPop.Pop3;
using System.Data.SqlClient;
using System.Text.RegularExpressions;
using Topshelf;
using Topshelf.Quartz;
using Quartz;
using HtmlAgilityPack;

namespace ExtractNewsletterFromEmail
{
    public class MyService
    {
        public void Start() { }
        public void Stop() { }
    }
    public class MyJob : IJob
    {
        // Seu hostname, username and password
        const int timeBetweenQueries = 30000;   // 30 seconds between queries.
        const string hostname = "localhost";
        const int port = 110;
        const bool useSSL = false;
        const string username = "user";
        const string password = "password";
        public void Execute(IJobExecutionContext context)
        {
            Console.WriteLine("Query started.");
            QueryEmail();
        }

        /// <summary>
        /// Example showing:
        ///  - how to fetch all messages from a POP3 server
        /// </summary>
        /// <param name="hostname">Hostname of the server. For example: pop3.live.com</param>
        /// <param name="port">Host port to connect to. Normally: 110 for plain POP3, 995 for SSL POP3</param>
        /// <param name="useSsl">Whether or not to use SSL to connect to server</param>
        /// <param name="username">Username of the user on the server</param>
        /// <param name="password">Password of the user on the server</param>
        /// <returns>All Messages on the POP3 server</returns>
        static Queue<Message> FetchAllMessages(string hostname, int port, bool useSsl, string username, string password)
        {
            // The client disconnects from the server when being disposed
            using (Pop3Client client = new Pop3Client())
            {
                // Connect to the server
                client.Connect(hostname, port, useSsl);

                // Authenticate ourselves towards the server
                client.Authenticate(username, password);

                // Get the number of messages in the inbox
                int messageCount = client.GetMessageCount();

                // We want to download all messages
                Queue<Message> allMessages = new Queue<Message>();

                // Messages are numbered in the interval: [1, messageCount]
                // Ergo: message numbers are 1-based.
                // Most servers give the latest message the highest number
                for (int i = messageCount; i > 0; i--)
                {
                    allMessages.Enqueue(client.GetMessage(i));
                }

                // Now return the fetched messages
                return allMessages;
            }
        }

        /// <summary>
        /// Example showing:
        ///  - how to delete a specific message from a server
        /// </summary>
        /// <param name="hostname">Hostname of the server. For example: pop3.live.com</param>
        /// <param name="port">Host port to connect to. Normally: 110 for plain POP3, 995 for SSL POP3</param>
        /// <param name="useSsl">Whether or not to use SSL to connect to server</param>
        /// <param name="username">Username of the user on the server</param>
        /// <param name="password">Password of the user on the server</param>
        /// <param name="messageNumber">
        /// The number of the message to delete.
        /// Must be in range [1, messageCount] where messageCount is the number of messages on the server.
        /// </param>
        

        static bool DeleteMessageByMessageId(Pop3Client client, string messageId)
        {
            // Get the number of messages on the POP3 server
            int messageCount = client.GetMessageCount();

            // Run trough each of these messages and download the headers
            for (int messageItem = messageCount; messageItem > 0; messageItem--)
            {
                // If the Message ID of the current message is the same as the parameter given, delete that message
                if (client.GetMessageHeaders(messageItem).MessageId == messageId)
                {
                    // Delete
                    client.DeleteMessage(messageItem);
                    return true;
                }
            }

            // We did not find any message with the given messageId, report this back
            return false;
        }

        static string ProcessNewsletter(ref string campaignID, string newsletter)
        {
            //Console.WriteLine("Processing ...");
            Match match;
            if (Regex.IsMatch(newsletter, "IDCamp=.+?(?=&)"))           // If match for IDCamp is found store it, otherwise continue as it's not a newsletter.
                match = Regex.Match(newsletter, "IDCamp=.+?(?=&)");
            else
                return "Not newsletter";
            campaignID = match.Value.Remove(0, 7);                      // Remove IDCamp=
            //Console.WriteLine("Removing footer");
            RemoveFooter(ref newsletter);                               // Removes the footer using HtmlAgilePack
            newsletter = newsletter.Replace("'", "''");                 // To prevent SQL from misinterpreting '.
            //Console.WriteLine("Processing complete.");
            return newsletter;
        }

        static void RemoveFooter(ref string newsletter)
        {
            var doc = new HtmlDocument();
            doc.LoadHtml(newsletter);
            try
            {
                HtmlNode node = doc.DocumentNode.SelectSingleNode("//table[@class='footerWrapper']");   // Finds the table tag with class of footerWrapper.
                node.ParentNode.RemoveChild(node, false);
                newsletter = doc.DocumentNode.InnerHtml;
            }
            catch
            {
                Console.WriteLine("Footer node not found. Newsletter unchanged");
            }
        }

        static void QueryEmail()
        {
            Console.WriteLine("Querying ...");
            // As we only need to access messages sequentially, Queue is suitable.
            Queue<Message> allMessages = FetchAllMessages(hostname, port, useSSL, username, password);
            List<string> messagesToDelete = new List<string>();

            string newsletter;
            string campaignID = "";

            using (var conn = new SqlConnection())
            {
                conn.ConnectionString = "Data Source=hostname,port; Database=databasename; User Id=id; Password=password";
                conn.Open();

                Console.WriteLine("Number of emails: " + allMessages.Count);

                foreach (Message m in allMessages)
                {
                    messagesToDelete.Add(m.Headers.MessageId);
                    newsletter = m.FindFirstHtmlVersion().GetBodyAsText();      // Saves the whole email into string.
                    //newsletter = m.FindFirstPlainTextVersion().GetBodyAsText(); // Used for testing when mailing html as plaintext.
                    if (String.Equals((newsletter = ProcessNewsletter(ref campaignID, newsletter)), "Not newsletter"))  // If e-mail does not have newsletter characteristics, it's discarded.
                    {
                        Console.WriteLine("E-mail not recognised as a newsletter and discarded");
                        continue;
                    }

                    SqlCommand command = new SqlCommand("UPDATE Campaigns SET Newsletter = '" + newsletter + "' WHERE ID = '" + campaignID + "';", conn);
                    command.ExecuteNonQuery();
                }
                //Console.WriteLine("saving");
                //SaveNewsletterToFile(conn, campaignID);                      // Save to file for testing.
            }

            using (var client = new Pop3Client())
            {
                client.Connect(hostname, port, useSSL);
                client.Authenticate(username, password);

                foreach (string s in messagesToDelete)
                {
                    DeleteMessageByMessageId(client, s);    // Deletes all previously processed messages.
                }
            }
        }

        // Deprecated methods
        static void DeleteMessageOnServer(string hostname, int port, bool useSsl, string username, string password, int messageNumber)
        {
            // The client disconnects from the server when being disposed
            using (Pop3Client client = new Pop3Client())
            {
                // Connect to the server
                client.Connect(hostname, port, useSsl);

                // Authenticate ourselves towards the server
                client.Authenticate(username, password);

                // Mark the message as deleted
                // Notice that it is only MARKED as deleted
                // POP3 requires you to "commit" the changes
                // which is done by sending a QUIT command to the server
                // You can also reset all marked messages, by sending a RSET command.
                client.DeleteMessage(messageNumber);

                // When a QUIT command is sent to the server, the connection between them are closed.
                // When the client is disposed, the QUIT command will be sent to the server
                // just as if you had called the Disconnect method yourself.
            }
        }
        static string RemoveFooterRegex(string newsletter) // Method removes footer and returns newsletter without it.
        {
            // Deprecated due to inconsistency. Better to do with html, mainly HtmlAgilityPack.
            try
            {
                System.IO.File.WriteAllText("C:/Users/martink/Desktop/Things/Emmares/EMMARES.MVP/readNL.html", newsletter);
                //newsletter = System.IO.File.ReadAllText("C:/Users/martink/Desktop/Things/Emmares/EMMARES.MVP/sampleNL.html");
                int startFooterIndex = newsletter.IndexOf(@"Help\s+us\s+revolutionise\s+email\s+marketing.");    // Approximate location of the area we're trying to remove.
                int endFooterIndex = newsletter.LastIndexOf("Copyright c");                             // Not the best way to identify the end, but safe of html parsing the easiest. Might be better to reimplement as html, if this doesn't work well.
                string partialFirst = newsletter.Remove(startFooterIndex);                              // Remove everything after footer's starting string.
                partialFirst = partialFirst.Remove(partialFirst.LastIndexOf(@"<table\s+style"));           // Remove the start of the table representing footer.
                string partialLast = newsletter.Remove(0, endFooterIndex);                              // Remove everything up to specified end of footer.
                partialLast = partialLast.Remove(0, partialLast.IndexOf("IDCamp") + 6);
                partialLast = partialLast.Remove(0, partialLast.IndexOf("IDCamp") + 6);                 // Index of second IDCamp in leftover, which is the last one in footer.
                partialLast = partialLast.Remove(0, partialLast.IndexOf("</table>") + 8);               // Finally finds the end of footer.
                return partialFirst + partialLast;                                                      // Glue together everything that isn't footer. Note, with proper knowledge of exactly where footer ends/starts it could all be done with one remove.
            }
            catch   // If it fails for whatever reason we return the original newsletter.
            {
                Console.WriteLine("Footer not recognised. Returned original newsletter.");
                //Console.WriteLine(newsletter);
                return newsletter;
            }
        }

        // Testing methods
        static void DisplayNode(HtmlNode node)
        {
            Console.WriteLine("Node Name: " + node.Name);

            Console.Write("\n" + node.Name + " children:\n");

            DisplayChildNodes(node);
        }
        static void DisplayChildNodes(HtmlNode nodeElement)
        {
            HtmlNodeCollection childNodes = nodeElement.ChildNodes;

            if (childNodes.Count == 0)
            {
                Console.WriteLine(nodeElement.Name + " has no children");
            }
            else
            {

                foreach (var node in childNodes)
                {
                    if (node.NodeType == HtmlNodeType.Element)
                    {
                        Console.WriteLine(node.OuterHtml);
                    }
                }
            }
        }
        static void SaveNewsletterToFile(SqlConnection conn, string CampaignID, string path = "newsletter_without_footer.html")
        {
            SqlCommand cmd = new SqlCommand   // Unrelated. Code to get data from Database, since application has character limit.
            {
                CommandText = "SELECT Campaigns.Newsletter FROM Campaigns WHERE Campaigns.ID = '" + CampaignID + "';",
                CommandType = System.Data.CommandType.Text,
                Connection = conn
            };
            SqlDataReader reader = cmd.ExecuteReader();
            Console.WriteLine(reader.Read());
            System.IO.File.WriteAllText(path, reader.GetString(0));
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            var hf = HostFactory.Run(x =>
            {
                x.Service<MyService>(s =>
                {
                s.ConstructUsing(name => new MyService());
                s.WhenStarted(tc => tc.Start());
                s.WhenStopped(tc => tc.Stop());

                s.ScheduleQuartzJob(q =>
                    q.WithJob(() =>
                        JobBuilder.Create<MyJob>().Build())
                        .AddTrigger(() => TriggerBuilder.Create()
                            .WithSimpleSchedule(b => b
                                .WithIntervalInSeconds(30)
                                .RepeatForever())
                            .Build()));
                });
                x.RunAsLocalSystem()
                    .DependsOnEventLog()
                    .StartAutomatically()
                    .EnableServiceRecovery(rc => rc.RestartService(1));

                x.SetDescription("Queries newsletters.");
                x.SetDisplayName("EmmaresNL");
                x.SetServiceName("Emmares Newsletter Service");
            });
            // Configure service hosting (remember to run the service as administrator)
            /*var settings = new ServiceHostingSettings
            {
                ServiceName = "Emmares",
                DisplayName = "Emmares Newsletter",
                Description = "Used to extract newsletters from preview@emmares.com at interval of " + timeBetweenQueries / 1000 + " sec",
                StartMode = ServiceStartMode.Automatic,
                ServiceAccount = ServiceAccount.LocalSystem,
                RecoveryOptions = new ServiceRecoveryOptions
                {
                    FirstFailureAction = ServiceRecoveryAction.RestartService,
                    SecondFailureAction = ServiceRecoveryAction.RestartService,
                    SubsequentFailureAction = ServiceRecoveryAction.RestartComputer,
                    ResetFailureCountWaitDays = 1,
                    RestartServiceWaitMinutes = 3,
                    RestartSystemWaitMinutes = 3
                }
            };

            // Parse command-line options and execute
            ServiceHost.Run(service, settings, args);*/
        }
    }
}
