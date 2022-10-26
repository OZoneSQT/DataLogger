using System;
using System.IO.Ports;

namespace SerialReader
{
    internal class Program
    {
        private static int baud = 9600;                    
        private static Parity parity = Parity.None;
        private static int packetSize = 8;                  // number of bits in a data packet OBS is this right???
        private static StopBits stopBits = StopBits.One;    // is this right???

        public static int Main(string[] args)
        {
            int i = 0;

            Console.WriteLine("Welcome to SerialReader...");
            Console.WriteLine("Save Serial data stream to *.csv file");

            try
            {
                Console.WriteLine("\nStartup...");

                SerialCom serialCom = new SerialCom();
                serialCom.SetSerialPort(baud, parity, packetSize, stopBits);


                i = serialCom.MainAsync(args).GetAwaiter().GetResult();


                serialCom.EndConnection();

            }
            catch (Exception e)
            {
                Console.WriteLine("The startup process exits with error: " + e.StackTrace);
                Console.WriteLine("\nTry to run program as admin (Right - clik on program and select[Run as administrator]).");

                System.Threading.Thread.Sleep(10000);

                Environment.Exit(1);
            }

            return i;
        }

    }
}
