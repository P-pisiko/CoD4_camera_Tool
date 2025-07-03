using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.FileFormats
{
    public class Csv
    {
        public Csv(List<Structs.Entitys.Camera> camList) 
        {
            ///<summary>
            /// Appearntly this forces float/double use . as a decimal seperator
            /// </summary>
            var inv = CultureInfo.InvariantCulture; 

            using var writer = new StreamWriter("positions.csv");
            writer.WriteLine("frame,x,y,z,yaw,pitch");
            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                writer.WriteLine(string.Format(inv,
                "{0},{1},{2},{3},{4},{5}",
                i,
                f.X,
                f.Y,
                f.Z,
                f.Pitch,
                f.Yaw));
            }
        }
    }
}
