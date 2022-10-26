using System;
using System.IO;

namespace SerialReader
{
    internal class DataExporter
    {
        private string fileName;

        public DataExporter()
        {
            fileName = DateBuilder();
            Console.WriteLine("Data will be stored here: " + fileName);
        }

        private String DateBuilder()
        {
            DateTime timestamp = DateTime.Now;
            String year = Convert.ToString(timestamp.Year);
            String month = Convert.ToString(timestamp.Month);
            String day = Convert.ToString(timestamp.Day);
            String hh = Convert.ToString(timestamp.Hour);
            String mm = Convert.ToString(timestamp.Minute);
            String ss = Convert.ToString(timestamp.Second);
            return fileName = "C:\\SerialReader_" + year + month + day + hh + mm + ss + ".csv";
        }

        public void WriteToFile(String datapacket)
        {
            if (!File.Exists(fileName))
            {
                string clientHeader = "Logged data:" + Environment.NewLine;

                File.WriteAllText(fileName, clientHeader);
            }

            File.AppendAllText(fileName, datapacket);
            Console.WriteLine(datapacket);
        }

    }
}
