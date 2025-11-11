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
        public Task ExportToCsvAsync(Structs.Entitys.Header header,List<Structs.Entitys.Camera> camList)
        {
            //var sw = Stopwatch.StartNew();
            var inv = CultureInfo.InvariantCulture;
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
            //sw.Stop();
            //Console.WriteLine($"It took {sw.ElapsedMilliseconds}ms to build the list in mem");
            var fileName = DateTime.Now.ToString("HH-mm-ss") + ".csv";
            var MapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;

            if (!Directory.Exists("./exported_cams"))
            {
                Directory.CreateDirectory("./exported_cams");
            }
            File.WriteAllTextAsync($"./exported_cams/{MapName}_{header.ConstCaptureFps}fps_{fileName}", sb.ToString());
            return Task.CompletedTask;
        }
    }
}
