#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Swap is a function that swaps the position of two elements in an array
  p is the array
  x and y are the indexes that hold the elements to be swapped
  no output */
void swap(int* p, int x, int y){
  int temp;
  temp = p[y];
  p[y] = p[x];
  p[x] = temp;
}

//---------------------------------------------------------------
/* generate is a function that generates a long list of pseudorandom integers and writes them to a file
  m: the number of ints to be generated
  fileptr: the file to write to
  allTracks: the array that all our data is represented in */
void generate(int m, FILE *fileptr, int allTracks[][5]){
  int randn;
  fileptr = fopen("inputs", "w"); //open file

  if(fileptr == NULL){
    perror("Failed to create file\n");
  }

  //generate m random integers and put them in inputs file
  for(int i = 0; i < m; i++){
    randn = rand() % 100; //mod 100 to enforce [0:99] range
    putw(randn, fileptr);
    allTracks[i][0] = randn;
  }
  fclose(fileptr);
}

//------------------------------------------------------------
/* Takes an array of tracks and an array of delays and fills in allTracks which is a big array that represents all of our corresponding delays to the corresponding track for each scheduling algorithm
  M: the number of inputs that we generate
  allTracks: the array that represents all our data
  tracks: the array of all tracks in the order they were serviced
  delays: the array of the corresponding tracks
  algo: the integer representation of each scheduling algorithm
        ex - FIFO = 1, SSTF = 2, SCAN = 3, CSCAN = 4 */
void writeToOutput(int m, int allTracks[][5], int tracks[], int delays[], int algo){
  for(int i = 0; i < m; i++){
    for(int j = 0; j < m; j++){
      if(allTracks[i][0] == tracks[j]){
        allTracks[i][algo] = delays[j];
        //if we represent the final data as "steps" the above line will be changed to allTracks[i][algo] = j;
        tracks[j] = (tracks[j] * -1);
        break;
      }
    }
  }
}

//---------------------------------------------------------------
/*  the fifo function opens the input file, records the difference between each track in the order they arrive, then calculates the number of tracks traversed total. The tracks are stored in trackArray and the time it took to seek to each track is stored in delays
  m: the number of inputs generated
  fileptr: the input file to be opened and read from
  allTracks: the array representing all tracks and delays for all scheduling algorithms */
int fifo(int m, FILE *fileptr, int allTracks[][5]){
  int traversed = 0; //total tracks traversed
  int trackArray[m];
  int delays[m];
  int index = 0;
  int pos = 50; //initialize current r/w arm position at 0
  int nextTrack; //track we will seek to

  fileptr = fopen("inputs", "r");

  while((nextTrack = getw(fileptr)) != EOF) {
    trackArray[index] = nextTrack;
    traversed = traversed + abs(pos - nextTrack);
    delays[index] = traversed;
    //if delays are cumulative, change ^ to delays[index]=traversed
    pos = nextTrack;
    index++;
  }
  fclose(fileptr);
  writeToOutput(m, allTracks, trackArray, delays, 1); //fills the array with the delays
  return traversed;
}

//-------------------------------------------------------------
/* the sstf function opens the input file and stores all inputs in an array, then calculates the closest pending request and seeks to that track, it calculates the distance between the current tack and the track it will be seeking to, then calculates the number of tracks traversed total. The tracks are stored in pending and the time it took to seek to each track is stored in delays
  m: the number of inputs generated
  fileptr: the input file to be opened and read from
  allTracks: the array representing all tracks and delays for all scheduling algorithms */
int sstf(int m, FILE *fileptr, int allTracks[][5]){
  int pending[m]; //array of pending reqs, will end in order they were serviced
  int delays[m]; 
  int index = 0; //for file read
  int track; //for file read
  int currTrack = 50; //initialize current position at 50
  int nextTrack = currTrack;
  int shortestDistance;
  int traversed = 0;

  fileptr = fopen("inputs", "r"); //read file

  while((track = getw(fileptr)) != EOF ) {
    pending[index] = track;
    index++;
  }
  fclose(fileptr);

  for(int i = 0; i < m; i++){
    shortestDistance = abs(currTrack - pending[i]);
    nextTrack = i;
    for(int j = i; j < m; j++){
      if(abs(currTrack - pending[j]) < shortestDistance){
        shortestDistance = abs(currTrack - pending[j]);
        nextTrack = j;
      }
    }
    traversed = traversed + abs(currTrack - pending[nextTrack]);
    delays[i] = traversed;
    currTrack = pending[nextTrack]; //move r/w arm to closest track
    if(nextTrack != i){
      swap(pending, i, nextTrack); //swap tracks
    }
  }
  writeToOutput(m, allTracks, pending, delays, 2); //fills the array with the delays
  return traversed;
}

//--------------------------------------------------------------
/* the scan function opens the input file and stores all inputs in an array, then calculates the closest pending request in one direction and seeks to that track, if there are no requests in that direction, scan flips the direction and finds the nearest track in that direction, it calculates the distance between the current tack and the track it will be seeking to, then calculates the number of tracks traversed total. The tracks are stored in pending and the time it took to seek to each track is stored in delays
  m: the number of inputs generated
  fileptr: the input file to be opened and read from
  allTracks: the array representing all tracks and delays for all scheduling algorithms */
int scan(int m, FILE *fileptr, int allTracks[][5]){
  int pending[m];
  int delays[m];
  int direction = -1; //start going up
  int track;
  int index = 0;
  
  fileptr = fopen("inputs", "r"); //read file

  while((track = getw(fileptr)) != EOF) {
    pending[index] = track;
    index++;
  }
  fclose(fileptr);

  int currTrack = 50; //initialize current position at 50
  int nextTrack = currTrack;
  int shortestDistance;
  int traversed = 0;

  for(int i = 0; i < m; i++){
    flip:
    shortestDistance = (currTrack - pending[i]) * (direction);
    nextTrack = i;
    for(int j = i; j < m; j++){
      if((((currTrack - pending[j])*(direction) < shortestDistance) && ((currTrack - pending[j])*(direction) >= 0)) || shortestDistance < 0){
        shortestDistance = (currTrack - pending[j])*(direction);
        nextTrack = j;
      }
    }
    if(shortestDistance >= 0){
      traversed = traversed + abs(currTrack - pending[nextTrack]);
      delays[i] = traversed;
      currTrack = pending[nextTrack];
      swap(pending, i, nextTrack);
    } else {
      direction = direction * -1; //flip direction
      goto flip;
    }
  }
  writeToOutput(m, allTracks, pending, delays, 3); //fills the array with the delays
  return traversed;
}

//-----------------------------------------------------------------
/* the cscan function opens the input file and stores all inputs in an array, then calculates the closest pending request in increasing order and seeks to that track, if there are no requests in that direction, cscan seeks to the 0 position and continues searching in the same dir it calculates the distance between the current tack and the track it will be seeking to, then calculates the number of tracks traversed total. The tracks are stored in pending and the time it took to seek to each track is stored in delays
  m: the number of inputs generated
  fileptr: the input file to be opened and read from
  allTracks: the array representing all tracks and delays for all scheduling algorithms */
int cscan(int m, FILE *fileptr, int allTracks[][5]){
  int pending[m];
  int delays[m];
  int direction = -1; //always going up
  int track;
  int index = 0;
  
  fileptr = fopen("inputs", "r"); //read file
  while((track = getw(fileptr)) != EOF) {
    pending[index] = track;
    index++;
  }
  fclose(fileptr);

  int currTrack = 50; //initialize current position at 50
  int nextTrack = currTrack;
  int shortestDistance;
  int traversed = 0;

  for(int i = 0; i < m; i++){
    goToZero:
    shortestDistance = (currTrack - pending[i]) * (direction); //init shortest distance
    nextTrack = i; //init nexttrack
    for(int j = i; j < m; j++){
      if((((currTrack - pending[j])*(direction) < shortestDistance) && ((currTrack - pending[j])*(direction) >= 0)) || shortestDistance < 0){
        shortestDistance = (currTrack-pending[j])*(direction);
        nextTrack = j;
      }
    }
    if(shortestDistance >= 0){
      traversed = traversed + abs(currTrack - pending[nextTrack]);
      delays[i] = traversed;
      currTrack = pending[nextTrack];
      swap(pending, i, nextTrack);
    } else {
      traversed = traversed + (currTrack - 0);
      currTrack = 0;
      goto goToZero; //move r/w arm to zero and continue seaching with same i value
    }
  }
  writeToOutput(m, allTracks, pending, delays, 4); //fills the array with the delays
  return traversed;
}

//--------------------------------------------------------------
//main
int main(int argc, char* argv[]) {
  int numRequests = 0;
  
  if((argc != 2) && (argc != 3)){
    printf("Incorrect Command Line Arguments\n");
    printf("Expected number of seek requests (int > 0) and optional -r flag for non-deterministic seeding\n");
    return 0;
  } else if((argc == 3) && (strcmp(argv[2], "-r") == 0)){
    //non-determininstic seeding of rand
    srand(time(NULL));
    sscanf(argv[1], "%d", &numRequests);
  } else {
    //determininstic seeding of rand
    srand(2);
    sscanf(argv[1], "%d", &numRequests);
  }
  int allTracks[numRequests][5];

  FILE *fileptr = NULL; //file pointer for inputs
  generate(numRequests, fileptr, allTracks); 
  
  printf("Tracks traversed with FIFO: %d\n", fifo(numRequests, fileptr, allTracks));
  printf("Tracks traversed with SSTF: %d\n", sstf(numRequests, fileptr, allTracks));
   printf("Tracks traversed with SCAN: %d\n", scan(numRequests, fileptr, allTracks));
  printf("Tracks traversed with CSCAN: %d\n", cscan(numRequests, fileptr, allTracks));

  //Code to open output.txt and print to file
  char temp0[64];
  char temp1[64];
  char temp2[64];
  char temp3[64];
  char temp4[64];
  
  FILE *outputptr = NULL; //file pointer for outputs
  outputptr = fopen("output.txt", "w"); //open output for writing
  if(outputptr == NULL){
    perror("Failed to create file\n");
  }

  fprintf(outputptr, "%s","Track,FIFO,SSTF,SCAN,CSCAN\n");
  for(int i = 0; i < numRequests; i++){
    //change ints to strings
    sprintf(temp0, "%d", allTracks[i][0]); 
    sprintf(temp1, "%d", allTracks[i][1]);
    sprintf(temp2, "%d", allTracks[i][2]);
    sprintf(temp3, "%d", allTracks[i][3]);
    sprintf(temp4, "%d", allTracks[i][4]);
    //concatenate the temp strings into a single string for 1 output.txt line
    strcat(temp0, ","); 
    strcat(temp0, temp1);
    strcat(temp0, ",");
    strcat(temp0, temp2);
    strcat(temp0, ",");
    strcat(temp0, temp3);
    strcat(temp0, ",");
    strcat(temp0, temp4);
    strcat(temp0, "\n");
    fprintf(outputptr, "%s", temp0);
  }
  fclose(outputptr);
  return 0;
}