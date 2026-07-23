using System.Globalization;
using System.Text;

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
            
            //Metadata of this capture
            sb.AppendLine($"#MapName={header.MapName}");
            sb.AppendLine($"#ConstCaptureFps={header.ConstCaptureFps.ToString(inv)}");

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

        /// <summary>
        /// Companion long-format export for bone data (`frame,object,bone,valid,px,py,pz,
        /// qx,qy,qz,qw`) -- kept as a separate file from the camera CSV rather than folded
        /// into it, so the existing camera-only consumers of that file (csv_import.py's
        /// original path, any external tooling) keep working unchanged. `frames[i]` is the
        /// bone objects captured for camera frame `i` (see BoneLogReader.cs); a frame with no
        /// bone objects yet (e.g. bone signatures unavailable on this build) just contributes
        /// no rows, not an error.
        /// </summary>
        public Task ExportBonesToCsvAsync(Structs.Entitys.Header header, List<List<Structs.Entitys.BoneObjectFrame>> frames)
        {
            var inv = CultureInfo.InvariantCulture;
            var sb = new StringBuilder(frames.Count * 128);

            sb.AppendLine($"#MapName={header.MapName}");
            sb.AppendLine($"#ConstCaptureFps={header.ConstCaptureFps.ToString(inv)}");
            sb.AppendLine("frame,object,bone,valid,px,py,pz,qx,qy,qz,qw");

            for (int i = 0; i < frames.Count; i++)
            {
                foreach (var obj in frames[i])
                {
                    foreach (var bone in obj.Bones)
                    {
                        sb.AppendFormat(inv,
                            "{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10}",
                            i,
                            obj.ObjectName,
                            bone.Name,
                            bone.Valid ? 1 : 0,
                            bone.Position.X, bone.Position.Y, bone.Position.Z,
                            bone.Rotation.X, bone.Rotation.Y, bone.Rotation.Z, bone.Rotation.W);
                        sb.AppendLine();
                    }
                }
            }

            var fileName = DateTime.Now.ToString("HH-mm-ss") + "_bones.csv";
            var mapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;

            if (!Directory.Exists("./exported_cams"))
            {
                Directory.CreateDirectory("./exported_cams");
            }
            File.WriteAllTextAsync($"./exported_cams/{mapName}_{header.ConstCaptureFps}fps_{fileName}", sb.ToString());
            return Task.CompletedTask;
        }
    }
}
