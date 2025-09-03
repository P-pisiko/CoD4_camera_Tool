using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.FileFormats
{
    public class Csv
    {
        ///<summary>
        /// Appearntly CultureInfo.InvariantCulture forces float/double use . as a decimal seperator
        /// </summary>
        public async Task ExportToCsvAsync(List<Structs.Entitys.Camera> camList)
        {
            var sw = Stopwatch.StartNew();

            var inv = CultureInfo.InvariantCulture;
            var fileName = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss") + ".csv";

            var sb = new StringBuilder(camList.Count * 64); // rough prealloc

            sb.AppendLine("frame,x,y,z,yaw,pitch");

            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                sb.AppendFormat(inv,
                    "{0},{1},{2},{3},{4},{5}",
                    i,
                    f.X,
                    f.Y,
                    f.Z,
                    f.Yaw,
                    f.Pitch);
                sb.AppendLine();
            }
            sw.Stop();
            Console.WriteLine($"It took {sw.ElapsedMilliseconds}ms to build the list in mem"); 

            await File.WriteAllTextAsync(fileName, sb.ToString());

        }
    }
}
