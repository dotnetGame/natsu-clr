using System;
using System.Collections.Generic;
using System.Text;
using Chino.Apps.Shell.Commands;
using Chino.Objects;

namespace Chino.Apps.Shell
{
    class CommandInterpreter
    {
        private readonly Dictionary<string, ShellCommand> _commands = new Dictionary<string, ShellCommand>();
        private Accessor<Directory> _currentDirectory;
        private string _currentPath;
        private CommandContext _cmdCtx = new CommandContext();

        public CommandInterpreter()
        {
            SetCurrentDirectory("/");

            RegisterCommand("free", new FreeCommand());
            RegisterCommand("echo", new EchoCommand());
            RegisterCommand("ls", new LsCommand());
        }

        public void RegisterCommand(string name, ShellCommand command)
        {
            _commands.Add(name, command);
        }

        public void Run()
        {
            while (true)
            {
                Console.Write("CS ");
                Console.Write(_currentPath);
                Console.Write("> ");
                var input = Console.ReadLine();

                if (!string.IsNullOrWhiteSpace(input))
                    ProcessInput(input);
            }
        }

        private void ProcessInput(string commandline)
        {
            var args = commandline.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            var commandName = args[0];
            if (commandName == "cd")
            {
                try
                {
                    if (args.Length == 2)
                        SetCurrentDirectory(args[1]);
                    else if (args.Length != 1)
                        Console.WriteLine("cd: invalid argument");
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                }
            }
            else if (_commands.TryGetValue(commandName, out var command))
            {
                try
                {
                    command.Execute(_cmdCtx, args);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                }
            }
            else
            {
                Console.WriteLine($"CS: command not found: {commandName}");
            }
        }

        private void SetCurrentDirectory(string path)
        {
            _currentDirectory = ObjectManager.OpenDirectory(AccessMask.GenericRead, new ObjectAttributes { Name = path, Root = path.StartsWith('/') ? null : _currentDirectory });

            if (string.IsNullOrEmpty(path))
                _currentPath = "/";
            else if (path.StartsWith('/'))
                _currentPath = path;
            else if (_currentPath.EndsWith('/'))
                _currentPath = _currentPath + path;
            else
                _currentPath = _currentPath + "/" + path;
            _cmdCtx.CurrentPath = _currentPath;
        }
    }

    public sealed class CommandContext
    {
        public string CurrentPath { get; set; }
    }

    public abstract class ShellCommand
    {
        public string Name { get; }

        public abstract void Execute(CommandContext context, string[] args);
    }
}
