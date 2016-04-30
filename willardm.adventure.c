//  Filename:     willardm.adventure.c
//  Author:       Micheal Willard
//  Class:        CS 344 - Operating Systems
//  School:       Oregon State University
//  Date:         4/24/16
//  Description:  A simple text game similar to the classic Adventure.
//                There is a series of 7 rooms, each with connections to other
//                rooms.  The player navigates from the Start Room to the End
//                Room.
//                Upon reaching the End Room, the player wins and the game ends.
//                The rooms and connections are configured randomly at the start
//                of each game.  Room info is written to files and read to
//                access data.
//
//  Input:        Upon execution, the user inputs char strings (the name of the
//                Room they want to travel to next).
//  Output:       Room data is written to files in a new directory.
//                Output to the consoles provides feedback to the user.
//
//  Notes:        Enter notes here ...
//  To Compile:   gcc -o willardm.adventure willardm.adventure.c
//  To Execute:   ./willardm.adventure

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h> // for isblank
#include <dirent.h> // for going through files

// Define Global Variables

// Declare Functions

void createDirectory(int pid, char* pathname);
void createRooms(char* pathname);
bool checkConnection(int index, int *arr, int size);
void openStartRoom(char* pathname, char* currentRoom, char* connections);
void getSelection(char* pathname, char* currentRoom, char* connections);
void readRoom(char* pathname, char* currentRoom, char* connections, int* success);

// exit(EXIT_FAILURE)
// exit(EXIT_SUCCESS)

int main(int argc, char** argv) {
  char pathname[64];
  char currentRoom[64];
  char connections[128];
  char stepsTaken[256];
  int steps = 0;
  int success = 0;
  srand(time(NULL));
  // Get the Process ID, and send that to directory creation function.
  pid_t processid = getpid();
  // printf("Process ID: %d\n", processid);
  createDirectory(processid, pathname);
  printf("Pathname: %s\n", pathname);
  // Directory is created, now build 7 Room files.
  createRooms(pathname);
  // Display START ROOM info
  openStartRoom(pathname, currentRoom, connections);
  // Add Start Room to stepsTaken
  strcat(stepsTaken, currentRoom);
  steps++;
  // printf("\nPathname: %s\n", pathname);
  // printf("\nCURRENT ROOM: %s\n", currentRoom);

  // Enter game Loop
  while (success == 0) {
    // printf("\nCalling getSelection\n");
    getSelection(pathname, currentRoom, connections);
    // printf("\nCURRENT ROOM: %s\n", currentRoom);
    //  Add currentRoom to stepsTaken
    strcat(stepsTaken, "\n");
    strcat(stepsTaken, currentRoom);
    steps++;
    // printf("\nCalling readRoom\n");
    readRoom(pathname, currentRoom, connections, &success);
  }
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n%s\n", steps, stepsTaken);
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


// Purpose:  To Build The Room files
// Input:    The Pathename to the PID named folder
// Output:   The 7 Room Files with their connections and type.
void createRooms(char* pathname) {
  // Each Room has a name
  // Each Room has @ least 3-6 connections (amount is random)
  // If Room A connects to Room B, Room B must connect to Room A.
  // Each Room has a room type, room name = Filename
  // Hard Code 10 Room names
  char *roomNames[10] = {"Alderaan", "Coruscant", "Corellia", "Endor", "Hoth", "Jakku", "Naboo", "Takodana", "Tatooine", "Yavin"};
  // Take array of indexes 0-9
  int roomIdxArray[10] = {0,1,2,3,4,5,6,7,8,9};
  char *fName;
  char *connRoom;
  int i;
  int numConn;
  int k;
  int connIdx2;
  char *connRoom2;
  char *roomType;
  int connArrSize;
  int randIdx;
  int connArr[16]={0};
  int idx;
  int temp3;
  // Shuffle them
  for (i = 0; i < 10; i++) {
    // Set the temp val
    int temp = roomIdxArray[i];
    // Get a random idx
    int randomIdx = rand() % 10;
    // Reassign
    roomIdxArray[i] = roomIdxArray[randomIdx];
    roomIdxArray[randomIdx] = temp;
  }
  // Pull the first 7 indexes
  int roomNameIdx[7];
  for (i = 0; i < 7; i++) {
    // We take the shuffled array of 10 indexes
    // Assign the first 7 to the roomNameIdx array
    roomNameIdx[i] = roomIdxArray[i];
  }
  // Check output
  // printf("\nRoom Idx Array:\n");
  // for (i = 0; i < 7; i++) {
  //   printf("%d,",roomNameIdx[i]);
  // }
  // printf("\n");
  int connIdx = roomNameIdx[6];
  connRoom = roomNames[connIdx];
  // This is an admittely hack-ey apparoach to randomizing the connections
  // I assured there is a path through, by making Room A connect to B if
  // Room B connected to Room A.  The other connections are randomly tossed in.
  for (i = 0; i < 7; i++) {
    // Use the shuffled roomNameIdx as the index for room names
    idx = roomNameIdx[i];
    // After the first case, get the guaranteed connection.
    if (i > 0) {
      if ( i % 2) {
        // If even assign next index as connections
        connIdx = roomNameIdx[i+1];
        connRoom = roomNames[connIdx];
      }
      else {
        connIdx = roomNameIdx[i-1];
        connRoom = roomNames[connIdx];
      }
    }

    fName = roomNames[idx];
    char dirTemp[64];
    strcpy(dirTemp, pathname);
    strcat(dirTemp, "/");
    strcat(dirTemp, fName);
    FILE *fp;
    fp = fopen(dirTemp, "w");
    if (fp == NULL) {
      fprintf(stderr, "Error: Failed to Create %s\n", dirTemp);
      perror("Error: createRooms");
      exit(1);
    }
    // write: ROOM NAME and fName
    fprintf(fp, "ROOM NAME: %s\n", fName);
    // Store previous room, so we know A connects to B and B connects to A
    // write CONNECTION n: prevRoom
    fprintf(fp, "CONNECTION %d: %s\n", 1, connRoom);
    // WRITE OTHER ROOMS
    // 3 to 6 connections -1
    numConn = (rand() % (5-2)+2);
    // printf("Num Conn: %d\n", numConn);
    connArrSize = numConn + 2;
    // printf("Conn Arr Size: %d\n", connArrSize);
    // Array of connection roomNameIdxs
    // memset(connArr, 0, sizeof(connArr)); Come back to this if time allows
    // Add roomNameIdx of current room
    connArr[0] = idx;
    // printf("connArr[0] = %d\n", connArr[0]);
    // Add roomNameIdx of connecting room
    connArr[1] = connIdx;
    // printf("connArr[1] = %d\n", connArr[1]);
    for (k = 2; k < connArrSize; k++) {
      // Can't use idx or connIdx
      // Get random index between 0 and 6
      randIdx = (rand() % (6-0)+0);
      // printf("Rand Idx = %d\n", randIdx);
      // USe that randIdx to get a roomNameIdx
      connIdx2 = roomNameIdx[randIdx];
      // printf("k: %d\nConn Idx 2: %d\n", k, connIdx2);
      // Check if roomNameIdx exists in connection array
      while (checkConnection(connIdx2, connArr, k)) {
        // printf("Conn Idx 2 check: %d true\n", connIdx2);
        randIdx = (rand() % (6-0)+0);
        connIdx2 = roomNameIdx[randIdx];
      }
      connArr[k] = connIdx2;
      // printf("connArr[%d] = %d\n", k, connArr[k]);
      temp3 = connArr[k];
      connRoom2 = roomNames[temp3];

      fprintf(fp, "CONNECTION %d: %s\n", k, connRoom2);
    }
    // write: ROOM_TYPE
    // write: START_ROOM, MID_ROOM, END_ROOM
    if (i == 0) {
      roomType = "START_ROOM";
    }
    else if (i == 6) {
      roomType = "END_ROOM";
    }
    else {
      roomType = "MID_ROOM";
    }
    fprintf(fp, "ROOM TYPE: %s\n", roomType);
    fclose(fp);
    // printf("\n--------------------------\n");
  }
  // Go back to first file, open and write prevRoom as connection

}

bool checkConnection(int index, int *arr, int size) {
  int i;
  for (i = 0; i < size; i++) {
    if (arr[i] == index) {
      // printf("%d is in the array\n", index);
      return true;
    }
  }
  return false;
}

// Function to open each file in directory looking for "START_ROOM"
// INPUT:   pathname (directory path)
// OUTPUT:  Contents of file
// Found this method in the dirent info
void openStartRoom(char* pathname, char* currentRoom, char* connections) {
  struct dirent *dp;
  DIR *dfd;
  char *dir;
  FILE *fp;
  int found = 0;
  char buffer[512];
  char line[256];
  char connBuffer[64];
  dir = pathname;
  int lines = 0;
  int count = 0;
  int nLine = 0;
  int ch = 0;
  char *str = "START_ROOM";
  if ((dfd = opendir(dir)) == NULL) {
    fprintf(stderr, "Can't open %s\n", dir);
    exit(1);
  }
  char filename_qfd[100];
  char new_name_qfd[100];
  // Loop through each file in the directory
  while ((dp = readdir(dfd)) != NULL && found == 0) {
    lines = 0;
    struct stat stbuf;
    sprintf(filename_qfd, "%s/%s", dir, dp->d_name);
    // printf("%s, %s, %s\n\n", dir, dp->d_name, filename_qfd);
    if (stat(filename_qfd, &stbuf) == -1) {
      // printf("Stat check on file failed: %s\n", filename_qfd);
      continue;
    }
    if ((fp = fopen(filename_qfd, "r")) == NULL ) {
      fprintf(stderr, "Error: Failed to Open %s\n", filename_qfd);
      perror("Error: openStartRoom");
      exit(1);
    }
    //  Loop through file to find match to START_ROOM
    while (fgets(buffer, 512, fp) != NULL) {
      if ((strstr(buffer, str)) != NULL) {
        // printf("Match on line %d in Filename: %s\n",lines, dp->d_name);
        // printf("%s\n", buffer);
        found = 1;
        // or strncat??
        strcpy(currentRoom, dp->d_name);
      }
      lines++;
    }
    if (found == 0) {
      // printf("%s is not the START_ROOM\n", filename_qfd);
    }
    if(fp) {
      fclose(fp);
    }
  }
  //  Start Room has been found display Contents
  printf("\nCURRENT LOCATION: %s\n", currentRoom);
  printf("POSSIBLE CONNECTIONS: ");
  strcpy(connections, "POSSIBLE CONNECTIONS: ");
  // Get # of lines in file, read the 2nd line to the n-1 line
  // Print out the 3rd string in each line
  // Reopen the file
  if ((fp = fopen(filename_qfd, "r")) == NULL ) {
    fprintf(stderr, "Error: Failed to Open %s\n", filename_qfd);
    perror("Error: openStartRoom");
    exit(1);
  }
  nLine = lines - 1;
  while (fgets(line, sizeof line, fp) != NULL) {
    // If we are on line 2 or less than n-1
    if (count >= 1 && count < nLine) {
      // Get everything after first 14 characters
      strcpy(connBuffer, line+14);
      // Chop off newline
      strtok(connBuffer, "\n");
      printf("%s, ", connBuffer);
      strcat(connections, connBuffer);
      strcat(connections, ", ");
    }
    count++;
  }
  if(fp) {
    fclose(fp);
  }
}

//  Allow user to input conenction names
//  Scan currentRoom file to see if name exists
//  If it exists, selection was good, retrun to main
void getSelection(char* pathname, char* currentRoom, char* connections) {
  char str[32];
  int found = 0;
  char roomBuffer[128];

  //cut off POSSIBLE ...
  strcpy(roomBuffer, connections+21);
  // printf("%s\n", roomBuffer);
  int n;
  while (found == 0) {
    // GET USER INPUT
    printf("\nWHERE TO? >");
    fgets(str, 32, stdin);
    n = strlen(str)-1;
    if (str[n] == '\n') {
      str[n] = '\0';
    }
    // printf("%s", str);
    // There is an issue if the input matches a part of the string
    // i.e. if Endor is in the buffer, and dor is input, there is a match.
    // I'd have to rebuild this to populate each name into a data structure
    // and then do a full string comp, i.e. connection[0] == "str"

    if (strstr(roomBuffer, str) != NULL) {
      found = 1;
      // printf("\nString Found\n");
      strcpy(currentRoom, str);
    }
    // I tried a couple approaches at a full string match, but couldn't get them working
    // *************
    // s = roomBuffer, t = str
    // char *p = roomBuffer;
    // printf("%s\n", p);
    // size_t n = strlen(str);
    // while ((p = strstr(p, str)) != NULL) {
    //   char *q = p + n;
    //   if (p == roomBuffer || isblank((unsigned char) *(p - 1))) {
    //     if (*q == '\0' || isblank((unsigned char) *q)) {
    //       found = 1;
    //       printf("\nString Found\n");
    //       strcpy(currentRoom, str);
    //       break;
    //     }
    //   }
    //   p = q;
    // }
    // **********
    // while ((p = strtok(p, " \r\n\0")) != NULL && found == 0) {
    //   if (strcmp(p, str) == 0) {
    //     found = 1;
    //     printf("\nString Found\n");
    //     strcpy(currentRoom, str);
    //   }
    // }
    // **********
    if (found == 0) {
      printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
      printf("\nCURRENT LOCATION: %s\n", currentRoom);
      printf("%s\n", connections);
    }
  }
}

//  READ ROOM
//  PURPOSE:  Use pathname and currentRoom to open the file of the next room.
//            Check the type of the Room, if END_ROOM: the exit to success
//            Else: display the currentRoom info and
//            re-populate the connections string and go back to main
//  INPUT:    currentRoom, pathname, and current connections string
//  OUTPUT:   Void, just resets the variables in main.
void readRoom(char* pathname, char* currentRoom, char* connections, int* success) {
  char *str = "END_ROOM";
  FILE *fp;
  char dirTemp[64];
  char buffer[512];
  char line[256];
  char connBuffer[64];
  int count = 0;
  int lines = 1;
  int nLine = 0;

  strcpy(dirTemp, pathname);
  strcat(dirTemp, "/");
  strcat(dirTemp, currentRoom);
  // Open current Room File
  // if ((fp = fopen(dirTemp, "r")) == NULL ) {
  //   fprintf(stderr, "Error: Failed to Open %s\n", dirTemp);
  //   perror("Error: getSelection");
  //   exit(1);
  // }
  //  Open the File, check if it is the end room
  if ((fp = fopen(dirTemp, "r")) == NULL ) {
    fprintf(stderr, "Error: Failed to Open %s\n", dirTemp);
    perror("Error: openStartRoom");
    exit(1);
  }
  //  Loop through file to find match to END_ROOM
  while (fgets(buffer, 512, fp) != NULL) {
    if ((strstr(buffer, str)) != NULL) {
      // printf("Match on line %d in Filename: %s\n",lines, dirTemp);
      // printf("%s\n", buffer);
      printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
      *success = 1;
    }
    lines++;
  }
  // printf("%s is not the END_ROOM\n", dirTemp);
  // printf("success = %d\n", *success);
  if (*success == 0) {
    printf("\nCURRENT LOCATION: %s\n", currentRoom);
    printf("POSSIBLE CONNECTIONS: ");
    strcpy(connections, "POSSIBLE CONNECTIONS: ");
    // Get # of lines in file, read the 2nd line to the n-1 line
    // Print out the 3rd string in each line
    // Reopen the file
    if ((fp = fopen(dirTemp, "r")) == NULL ) {
      fprintf(stderr, "Error: Failed to Open %s\n", dirTemp);
      perror("Error: openStartRoom");
      exit(1);
    }
    nLine = lines - 2;
    while (fgets(line, sizeof line, fp) != NULL) {
      // If we are on line 2 or less than n-1
      if (count >= 1 && count < nLine) {
        // Get everything after first 14 characters
        strcpy(connBuffer, line+14);
        // Chop off newline
        strtok(connBuffer, "\n");
        printf("%s, ", connBuffer);
        strcat(connections, connBuffer);
        strcat(connections, ", ");
      }
      count++;
    }
  }

  if(fp) {
    fclose(fp);
  }
}
