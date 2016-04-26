// Filename:     willardm.adventure.c
// Author:       Micheal Willard
// Class:        CS 344 - Operating Systems
// School:       Oregon State University
// Date:         4/24/16
// Description:  A simple text game similar to the classic Adventure.
//               There is a series of 7 rooms, each with connections to other
//               rooms.  The player navigates from the Start Room to the End
//               Room.
//               Upon reaching the End Room, the player wins and the game ends.
//               The rooms and connections are configured randomly at the start
//               of each game.  Room info is written to files and read to
//               access data.
//
// Input:        Upon execution, the user inputs char strings (the name of the
//               Room they want to travel to next).
// Output:       Room data is written to files in a new directory.
//               Output to the consoles provides feedback to the user.
//
// Notes:        Enter notes here ...
// gcc -o willardm.adventure willardm.adventure.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

// Define Global Variables

// Declare Functions

void createDirectory(int pid, char* pathname);

int main(int argc, char** argv) {
  char pathname[64];
  // Get the Process ID, and send that to directory creation function.
  pid_t processid = getpid();
  printf("Process ID: %d\n", processid);
  createDirectory(processid, pathname);
  printf("Pathname: %s\n", pathname);
  // Directory is created, now build 7 Room files.
  

  return 0;
}

// Create a Directory to store the Room files
//const char *
void createDirectory(int pid, char* pathname) {
  // Directory of form <username>.rooms.<processid>
  char path[64] = "./willardm.rooms.";
  char buffer[33];
  // Convert pid to string
  sprintf(buffer, "%d", pid);
  // cat the full path name together
  strcat(pathname, path);
  strcat(pathname, buffer);

  struct stat st = {0};
  // Check if directory already exists
  if (stat(pathname, &st) == -1) {
    // If check passes, create new Directory
    int check;
    check = mkdir(pathname, 0777);
    if (check == -1) {
      // Check reveals mkdir failed
      fprintf(stderr, "Directory creation failed.\n");
      perror("Error: in createDirectory");
      exit(1);
    }
  }
  else {
    fprintf(stderr, "Directory exists.\n");
    perror("Error: in createDirectory");
    exit(1);
  }
}
