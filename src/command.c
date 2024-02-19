#include "command.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "command_client.h"
#include "command_fix_line.h"
#include "command_map.h"
#include "command_server.h"
#include "command_test_csv.h"
#include "command_triangulate.h"
#include "types.h"

typedef int (*CommandHandler)(int, char **);

typedef struct Command {
  // Is command hidden?
  bool hidden;
  // Command name
  char *name;
  // Command description
  char *description;
  // Command handler
  CommandHandler handler;
} Command;

internal void printUsage(const char *prog, const Command *commands,
                         size_t ncommands) {
  fprintf(stderr, "Usage: %s [COMMAND]\n\n", prog);
  fprintf(stderr, "COMMAND:\n");
  for (size_t i = 0; i < ncommands; i++) {
    if (!commands[i].hidden) {
      fprintf(stderr, "    %-20s - %s\n", commands[i].name,
              commands[i].description);
    }
  }
}

int run(int argc, char **argv) {
  static const Command commands[] = {
      {
          .hidden = false,
          .name = "map",
          .description = "My first aproach to drawing geospaitial data on the "
                         "screen. Command is drawing geo features from the "
                         "geobuf file provided in command-line arguments.",
          .handler = commandMap,
      },
      {
          .hidden = false,
          .name = "triangulate",
          .description = "Allows to draw polygon on the screen and see how "
                         "triangulation works.",
          .handler = commandTriangulate,
      },
      {
          .hidden = false,
          .name = "fix-line",
          .description = "fix-line is a command that allows to draw line run "
                         "Douglas-Pekuer simplifiction algorithm with "
                         "different epsilon values.",
          .handler = commandFixLine,
      },
      {
          .hidden = false,
          .name = "server",
          .description = "simple server",
          .handler = commandServer,
      },
      {
          .hidden = false,
          .name = "client",
          .description = "simple client",
          .handler = commandClient,
      },
      {
          .hidden = true,
          .name = "test-csv",
          .description = "Command for testing CSV parsing",
          .handler = commandTestCsv,
      },
  };

  size_t ncommands = sizeof(commands) / sizeof(Command);

  if (argc < 2) {
    printUsage(argv[0], commands, ncommands);
    return 1;
  }

  char *command = argv[1];

  for (size_t i = 0; i < ncommands; i++) {
    size_t len = strlen(commands[i].name);
    if (strncmp(command, commands[i].name, len) == 0) {
      return commands[i].handler(argc - 1, ++argv);
    }
  }

  fprintf(stderr, "Error: unknown command %s\n\n", argv[1]);
  printUsage(argv[0], commands, ncommands);

  return 1;
}
