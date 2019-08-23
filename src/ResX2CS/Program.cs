using System;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace ResX2CS
{
    class Program
    {
        static void Main(string[] args)
        {
            var path = @"..\..\..\..\System.Private.CorLib\Resources\Strings.resx";
            var xml = XDocument.Load(path);
            var datas = xml.Root.Nodes().OfType<XElement>().Where(x => x.Name == "data").ToList();
            using (var cs = new StreamWriter(File.Open(@"..\..\..\..\System.Private.CorLib\Resources\Strings.Designer.cs", FileMode.Create)))
            {
                StreamWriter Ident(int n)
                {
                    for (int i = 0; i < n; i++)
                        cs.Write("    ");
                    return cs;
                }

                cs.Write(@"namespace System
{
    internal static partial class SR
    {
");

                foreach (var data in datas)
                {
                    var name = data.Attribute("name").Value;
                    var value = data.Element("value").Value;
                    Ident(2).WriteLine($"public static string {name} => @\"{value.Replace("\"", "\"\"")}\";");
                }

                cs.Write(@"    }
}
");
            }
        }
    }
}
