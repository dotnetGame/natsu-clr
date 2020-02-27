using System;
using System.Collections.Generic;
using System.Text;
using Chino.Apps.Shell.Commands;

namespace Chino.Apps.Shell
{
    class CommandInterpreter
    {
        private readonly Dictionary<string, ShellCommand> _commands = new Dictionary<string, ShellCommand>();

        public CommandInterpreter()
        {
            RegisterCommand("free", new FreeCommand());
            RegisterCommand("echo", new EchoCommand());
        }

        public void RegisterCommand(string name, ShellCommand command)
        {
            _commands.Add(name, command);
        }

        public void Run()
        {
            while (true)
            {
                Console.Write("$ ");
                var input = Console.ReadLine();

                if (!string.IsNullOrWhiteSpace(input))
                    ProcessInput(input);
            }
        }

        private void ProcessInput(string commandline)
        {
            var args = commandline.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            var commandName = args[0];
            if (_commands.TryGetValue(commandName, out var command))
            {
                try
                {
                    command.Execute(args);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                }
            }
            else
            {
                Console.WriteLine($"sh: command not found: {commandName}");
            }
        }
    }

    public abstract class ShellCommand
    {
        public string Name { get; }

        public abstract void Execute(string[] args);
    }
}
