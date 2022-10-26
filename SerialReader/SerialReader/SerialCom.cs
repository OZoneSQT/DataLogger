using System;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.IO;

namespace SerialReader
{
    internal class SerialCom
    {
        private String portId = "";
        private int portNumber = 1;
        private SerialPort serialPort = null;
        private DataExporter dataExporter = null;

        public void SetSerialPort(int baud, Parity parity, int packetSize, StopBits stopBits)
        {
            Console.WriteLine("> Init of Serial communication ...");

            if (portNumber > 99) InitErrorHandler(baud, parity, packetSize, stopBits);

            Console.WriteLine("testing port: " + portNumber);
            portId = "COM" + Convert.ToString(portNumber);

            GenerateSerialPort(portId, baud, parity, packetSize, stopBits);
            dataExporter = new DataExporter();
        }

        private void GenerateSerialPort(String portId, int baud, Parity parity, int packetSize, StopBits stopBits)
        {
            try
            {
                serialPort = new SerialPort(portId, baud, parity, packetSize, stopBits);
                serialPort.NewLine = "\r";
                serialPort.Open();
                Console.WriteLine("> Serial communication via " + portId + " started");
            }
            catch (IOException e) // Throws System.IO.Exception (port does not exist) if i = 3 the first port is hit/set/started
            {
                portNumber++;
                SetSerialPort(baud, parity, packetSize, stopBits);
            }
            catch (UnauthorizedAccessException e)  // Throws System.UnauthorizedAccessException if port is taken by other process
            {
                portNumber++;
                SetSerialPort(baud, parity, packetSize, stopBits);
            }
        }

        public void EndConnection()
        {
            serialPort.Close();
            Console.WriteLine("> Serial communication via " + portId + " stopped");
        }

        private void InitErrorHandler(int baud, Parity parity, int packetSize, StopBits stopBits)
        {
            Console.WriteLine("> Troubleshooting:\nRightclick on [Windows start menu] and select [Device manager]");
            Console.WriteLine("  Check for connected serialports under [Ports (COM & LPT)], and enter portnumber in console, and press [enter]:");
            Console.WriteLine("  NOTE: If there not is any serial ports type in \"n\" in the console, and press [enter]:");

            String result = Console.ReadLine();

            if (result == "n") MissingPortGuide();
            if (result == null) InitErrorHandler(baud, parity, packetSize, stopBits);

            String portId = "COM" + result;
            GenerateSerialPort(portId, baud, parity, packetSize, stopBits);
        }

        private static void MissingPortGuide()
        {
            Console.WriteLine("> Solution to the issue with missing serialports.");
            Console.WriteLine("  Use 2 pcs. USB to Serial adapters and connect the serialports with a NULL-modem (cross-cabel).");
            Console.WriteLine("  Then connect both USB ports to your computer.");
        }


        // RX

        public async Task<int> MainAsync(string[] args)
        {

            Console.WriteLine("> DEVICE STARTED");
            bool _continue = true;
            String input;

            Random rng = new Random();

            do
            {
                input = await ReadDataAsync();

                //TODO Print input to console and write to .csv file
                
            } while (_continue);
            serialPort.Write($"KTHXBYE");
            Console.WriteLine("> DEVICE STOPPED");

            return 0;
        }

        private async Task<string> ReadDataAsync()
        {
            byte[] buffer = new byte[serialPort.ReadBufferSize];

            int bytesRead = await serialPort.BaseStream.ReadAsync(buffer, 0, buffer.Length);

            return Encoding.ASCII.GetString(buffer).Substring(0, bytesRead - serialPort.NewLine.Length);
        }

    }
}