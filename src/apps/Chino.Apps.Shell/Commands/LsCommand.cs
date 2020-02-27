using System;
using System.Collections.Generic;
using System.Text;
using Chino.Memory;
using Chino.Objects;

namespace Chino.Apps.Shell.Commands
{
    class LsCommand : ShellCommand
    {
        public override void Execute(CommandContext context, string[] args)
        {
            if (args.Length > 2)
            {
                Console.WriteLine("ls: invalid argument");
                return;
            }

            var dir = ObjectManager.OpenDirectory(AccessMask.GenericRead, new ObjectAttributes { Name = GetPath(context, args) });

            foreach (var item in dir.GetChildren())
            {
                if (item.IsDirectory)
                    Console.Write("<DIR>\t");
                else
                    Console.Write("\t");
                Console.WriteLine(item.Name);
            }

            Console.WriteLine();
        }

        private string GetPath(CommandContext context, string[] args)
        {
            string path;
            if (args.Length == 1)
                path = context.CurrentPath;
            else if (context.CurrentPath.EndsWith('/'))
                path = context.CurrentPath + args[1];
            else
                path = context.CurrentPath + "/" + args[1];
            return path;
        }
    }
}
