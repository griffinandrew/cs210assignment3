/**
 * Assignment #3: Strings, structs, pointers, command-line arguments.
 *  Let's use our knowledge to write a simple flight management system!
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Limit constants
#define MAX_CITY_NAME_LEN 20
#define MAX_FLIGHTS_PER_CITY 5 //3
#define MAX_DEFAULT_SCHEDULES 50 //2

// Time definitions
#define TIME_MIN 0
#define TIME_MAX ((60 * 24)-1)
#define TIME_NULL -1


/******************************************************************************
 * Structure and Type definitions                                             *
 ******************************************************************************/
typedef int time_t;                        // integers used for time values
typedef char city_t[MAX_CITY_NAME_LEN+1];; // null terminate fixed length city
 
// Structure to hold all the information for a single flight
//   A city's schedule has an array of these
struct flight {
  time_t time;       // departure time of the flight
  int available;  // number of seats currently available on the flight
  int capacity;   // maximum seat capacity of the flight
};

// Structure for an individual flight schedule
// The main data structure of the program is an Array of these structures
// Each structure will be placed on one of two linked lists:
//                free or active
// Initially the active list will be empty and all the schedules
// will be on the free list.  Adding a schedule is finding the first
// free schedule on the free list, removing it from the free list,
// setting its destination city and putting it on the active list
struct flight_schedule {
  city_t destination;                          // destination city name
  struct flight flights[MAX_FLIGHTS_PER_CITY]; // array of flights to the city
  struct flight_schedule *next;                // link list next pointer
  struct flight_schedule *prev;                // link list prev pointer
};

/******************************************************************************
 * Global / External variables                                                *
 ******************************************************************************/
// This program uses two global linked lists of Schedules.  See comments
// of struct flight_schedule above for details
struct flight_schedule *flight_schedules_free = NULL;
struct flight_schedule *flight_schedules_active = NULL;


/******************************************************************************
 * Function Prototypes                                                        *
 ******************************************************************************/
// Misc utility io functions
int city_read(city_t city);           
bool time_get(time_t *time_ptr);      
bool flight_capacity_get(int *capacity_ptr);
void print_command_help(void);

// Core functions of the program
void flight_schedule_initialize(struct flight_schedule array[], int n);
struct flight_schedule * flight_schedule_find(city_t city);
struct flight_schedule * flight_schedule_allocate(void);
void flight_schedule_free(struct flight_schedule *fs);
void flight_schedule_add(city_t city);
void flight_schedule_listAll(void);
void flight_schedule_list(city_t city);
void flight_schedule_add_flight(city_t city);
void flight_schedule_remove_flight(city_t city);
void flight_schedule_schedule_seat(city_t city);
void flight_schedule_unschedule_seat(city_t city);
void flight_schedule_remove(city_t city);

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs);
int  flight_compare_time(const void *a, const void *b);


int main(int argc, char *argv[]) 
{
  long n = MAX_DEFAULT_SCHEDULES;
  char command;
  city_t city;

  if (argc > 1) {
    // If the program was passed an argument then try and convert the first
    // argument in the a number that will override the default max number
    // of schedule we will support
    char *end;
    n = strtol(argv[1], &end, 10); // CPAMA p 787
    if (n==0) {
      printf("ERROR: Bad number of default max scedules specified.\n");
      exit(EXIT_FAILURE);
    }
  }

  // C99 lets us allocate an array of a fixed length as a local 
  // variable.  Since we are doing this in main and a call to main will be 
  // active for the entire time of the program's execution this
  // array will be alive for the entire time -- its memory and values
  // will be stable for the entire program execution.
  struct flight_schedule flight_schedules[n];
 
  // Initialize our global lists of free and active schedules using
  // the elements of the flight_schedules array
  flight_schedule_initialize(flight_schedules, n);

  // DEFENSIVE PROGRAMMING:  Write code that avoids bad things from happening.
  //  When possible, if we know that some particular thing should have happened
  //  we think of that as an assertion and write code to test them.
  // Use the assert function (CPAMA p749) to be sure the initilization has set
  // the free list to a non-null value and the the active list is a null value.
  assert(flight_schedules_free != NULL && flight_schedules_active == NULL);

  // Print the instruction in the beginning
  print_command_help();

  // Command processing loop
  while (scanf(" %c", &command) == 1) {
    switch (command) {
    case 'A': 
      //  Add an active flight schedule for a new city eg "A Toronto\n"
      city_read(city);
      flight_schedule_add(city);

      break;
    case 'L':
      // List all active flight schedules eg. "L\n"
      flight_schedule_listAll();
      break;
    case 'l': 
      // List the flights for a particular city eg. "l\n"
      city_read(city);
      flight_schedule_list(city);
      break;
    case 'a':
      // Adds a flight for a particular city "a Toronto\n
      //                                      360 100\n"
      city_read(city);
      flight_schedule_add_flight(city);
      break;
    case 'r':
      // Remove a flight for a particular city "r Toronto\n
      //                                        360\n"
      city_read(city);
      flight_schedule_remove_flight(city);
	break;
    case 's':
      // schedule a seat on a flight for a particular city "s Toronto\n
      //                                                    300\n"
      city_read(city);
      flight_schedule_schedule_seat(city);
      break;
    case 'u':
      // unschedule a seat on a flight for a particular city "u Toronto\n
      //                                                      360\n"
        city_read(city);
        flight_schedule_unschedule_seat(city);
        break;
    case 'R':
      // remove the schedule for a particular city "R Toronto\n"
      city_read(city);
      flight_schedule_remove(city);  
      break;
    case 'h':
        print_command_help();
        break;
    case 'q':
      goto done;
    default:
      printf("Bad command. Use h to see help.\n");
    }
  }
 done:
  return EXIT_SUCCESS;
}

/**********************************************************************
 * city_read: Takes in and processes a given city following a command *
 *********************************************************************/
int city_read(city_t city) {
  int ch, i=0;

  // skip leading non letter characters
  while (true) {
    ch = getchar();
    if ((ch >= 'A' && ch <= 'Z') || (ch >='a' && ch <='z')) {
      city[i++] = ch;
      break;
    }
  }
  while ((ch = getchar()) != '\n') {
    if (i < MAX_CITY_NAME_LEN) {
      city[i++] = ch;
    }
  }
  city[i] = '\0';
  return i;
}


/****************************************************************
 * Message functions so that your messages match what we expect *
 ****************************************************************/
void msg_city_bad(char *city) {
  printf("No schedule for %s\n", city);
}

void msg_city_exists(char *city) {
  printf("There is a schedule of %s already.\n", city);
}

void msg_schedule_no_free(void) {
  printf("Sorry no more free schedules.\n");
}

void msg_city_flights(char *city) {
  printf("The flights for %s are:", city);
}

void msg_flight_info(int time, int avail, int capacity) {
  printf(" (%d, %d, %d)", time, avail, capacity);
}

void msg_city_max_flights_reached(char *city) {
  printf("Sorry we cannot add more flights on this city.\n");
}

void msg_flight_bad_time(void) {
  printf("Sorry there's no flight scheduled on this time.\n");
}

void msg_flight_no_seats(void) {
    printf("Sorry there's no more seats available!\n");
}

void msg_flight_all_seats_empty(void) {
  printf("All the seats on this flights are empty!\n");
}

void msg_time_bad() {
  printf("Invalid time value\n");
}

void msg_capacity_bad() {
  printf("Invalid capacity value\n");
}

void print_command_help()
{
  printf("Here are the possible commands:\n"
	 "A <city name>     - Add an active empty flight schedule for\n"
	 "                    <city name>\n"
	 "L                 - List cities which have an active schedule\n"
	 "l <city name>     - List the flights for <city name>\n"
	 "a <city name>\n"
         "<time> <capacity> - Add a flight for <city name> @ <time> time\n"
	 "                    with <capacity> seats\n"  
	 "r <city name>\n"
         "<time>            - Remove a flight form <city name> whose time is\n"
	 "                    <time>\n"
	 "s <city name>\n"
	 "<time>            - Attempt to schedule seat on flight to \n"
	 "                    <city name> at <time> or next closest time on\n"
	 "                    which their is an available seat\n"
	 "u <city name>\n"
	 "<time>            - unschedule a seat from flight to <city name>\n"
	 "                    at <time>\n"
	 "R <city name>     - Remove schedule for <city name>\n"
	 "h                 - print this help message\n"
	 "q                 - quit\n"
);
}


/****************************************************************
 * Resets a flight schedule                                     *
 ****************************************************************/
void flight_schedule_reset(struct flight_schedule *fs) {
    fs->destination[0] = 0;
    for (int i=0; i<MAX_FLIGHTS_PER_CITY; i++) {
      fs->flights[i].time = TIME_NULL;
      fs->flights[i].available = 0;
      fs->flights[i].capacity = 0;
    }
    fs->next = NULL;
    fs->prev = NULL;
}

/******************************************************************
* Initializes the flight_schedule array that will hold any flight *
* schedules created by the user. This is called in main for you.  *
 *****************************************************************/

void flight_schedule_initialize(struct flight_schedule array[], int n)
{
  flight_schedules_active = NULL;
  flight_schedules_free = NULL;

  // takes care of empty array case
  if (n==0) return;

  // Loop through the Array connecting them
  // as a linear doubly linked list
  if (n == 1) {
    flight_schedule_reset(&array[0]);
    array[0].next = NULL;
    array[0].prev = NULL;
    flight_schedules_free = &array[0];
    return;
  }

  flight_schedule_reset(&array[0]);
  array[0].next = &array[1];
  array[0].prev = NULL;

  for (int i=1; i<n-1; i++) {
    flight_schedule_reset(&array[i]);
    array[i].next = &array[i+1];
    array[i].prev = &array[i-1];
  }

  // Takes care of last node.  
  flight_schedule_reset(&array[n-1]); // reset clears all fields
  array[n-1].next = NULL;
  array[n-1].prev = &array[n-2];
  flight_schedules_free = &array[0];
}

/***********************************************************
 * time_get: read a time from the user
   Time in this program is a minute number 0-((24*60)-1)=1439
   -1 is used to indicate the NULL empty time 
   This function should read in a time value and check its 
   validity.  If it is not valid eg. not -1 or not 0-1439
   It should print "Invalid Time" and return false.
   othewise it should return the value in the integer pointed
   to by time_ptr.
 ***********************************************************/
bool time_get(int *time_ptr) {
  if (scanf("%d", time_ptr)==1) {
    return (TIME_NULL == *time_ptr || 
	    (*time_ptr >= TIME_MIN && *time_ptr <= TIME_MAX));
  } 
  msg_time_bad();
  return false;
}

/***********************************************************
 * flight_capacity_get: read the capacity of a flight from the user
   This function should read in a capacity value and check its 
   validity.  If it is not greater than 0, it should print 
   "Invalid capacity value" and return false. Othewise it should 
   return the value in the integer pointed to by cap_ptr.
 ***********************************************************/
bool flight_capacity_get(int *cap_ptr) {
  if (scanf("%d", cap_ptr)==1) {
    return *cap_ptr > 0;
  }
  msg_capacity_bad();
  return false;
}

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs) 
{
  qsort(fs->flights, MAX_FLIGHTS_PER_CITY, sizeof(struct flight),
	flight_compare_time);
}

int flight_compare_time(const void *a, const void *b) 
{
  const struct flight *af = a;
  const struct flight *bf = b;
  
  return (af->time - bf->time);
}

//WRITE SOME CODE!
//flight schedule allocate
//takes a schedule off the free list and places it on the active list
//returns pointer to updated flight schedule
struct flight_schedule * flight_schedule_allocate(void){

struct flight_schedule *fs = flight_schedules_free;

flight_schedules_free = flight_schedules_free->next;

flight_schedules_free->prev = NULL;

fs->next = flight_schedules_active;


if(flight_schedules_active != NULL){
  flight_schedules_active->prev = fs;
  
}
flight_schedules_active= fs;

return (flight_schedules_free == NULL ? NULL : flight_schedules_active);
}
/*
  struct flight_schedule *temp =  flight_schedules_free;

  if (flight_schedules_free != NULL){
    flight_schedules_free = temp->next; //make sure 
    if(temp->next != NULL) {
      flight_schedules_free->prev = NULL;
    }
    
  }


  temp->next = flight_schedules_active;
  //if active not null set active prev to temp
  if(flight_schedules_active != NULL){
    flight_schedules_active->prev = temp;
  }
  flight_schedules_active = temp;

  /*
  flight_schedules_free-> prev->next = flight_schedules_active;
  
  //temp->next = flight_schedules_active;
  flight_schedules_active = flight_schedules_free->prev;
  flight_schedules_free->prev = NULL;
  
  //temp->prev = NULL;
  //struct flight_schedule *temp2 = flight_schedules_active

  if(flight_schedules_active -> next != NULL){
    flight_schedules_active->next->prev = flight_schedules_active;
  }

  //flight_schedules_active = temp;
  temp = flight_schedules_active;
  */
  //return(temp); //return temp temp could be null resulting in error
//}

//flight schedule free
//takes schedule off active list, resets it, then places the node back on free list
void flight_schedule_free(struct flight_schedule *fs){



  if (fs->next != NULL && fs == flight_schedules_active) { // firtst node with others
    flight_schedules_active = fs->next;
    flight_schedules_active->prev = NULL;
  }
  else if (fs == flight_schedules_active){   //only one node
    flight_schedules_active = NULL;
  }
  else if (fs->next == NULL){ // last
    fs->prev->next = NULL;
  }
  else{
    fs->prev->next = fs->next;
    fs->next->prev = fs->prev;
  }
  flight_schedule_reset(fs);

  if (flight_schedules_free == NULL){
    flight_schedules_free = fs;
  }
  else{
    flight_schedules_free->prev = fs;
    fs->next = flight_schedules_free;
    flight_schedules_free = fs;
    fs->prev = NULL;
  }


/*
  flight_schedules_active = flight_schedules_active->next;
  flight_schedule_reset(fs);
  fs->next = flight_schedules_free;
  flight_schedules_free = fs;
  fs->next->prev = fs;
*/

  /*int counter = 0;
  struct flight_schedule *temp = flight_schedules_active;
  while(temp != NULL) {
    if(temp == fs){ //use flight schedule find instead
      break;
    }
    temp = temp->next;
    counter++;
  }
  if (counter == 0){
    flight_schedules_active = temp->next;
    flight_schedules_active->prev = NULL;
    temp->next = NULL;

    flight_schedule_reset(temp);

    temp->next = flight_schedules_free; //this block of code adds temp to front of free
    flight_schedules_free = temp;
    flight_schedules_free->next->prev = flight_schedules_free;
    //flight_schedule_reset(temp);

  }
  else{ 
    temp->prev->next = temp->next;   //actualy this is the else case end case not required
    temp->next->prev = temp->prev;
    temp->next = NULL;
    temp->prev = NULL;

    flight_schedule_reset(temp);

    temp->next = flight_schedules_free;
    flight_schedules_free = temp;
    flight_schedules_free->next->prev = flight_schedules_free;
  }
}
*/
}

struct flight_schedule *flight_schedule_find(city_t city){ //or char *city//it says city_t city cant use strcmp then ja
  struct flight_schedule *temp = flight_schedules_active;
  int found = 0;

//  if (temp == NULL) {
 //   msg_city_bad(city);
 // }

  while(temp != NULL){
    if (strcmp(city,temp->destination) == 0){ //may have to use casts?
      found =1;
      //continue;
      return(temp);
    }
    else {
      found = 0;
    }
    temp = temp->next;
  }
 //this might return null 
 //return(temp);
}


void flight_schedule_add(city_t city){ //should this call get city

  //first cheeck to see if city exists
  if (flight_schedules_free == NULL){
    msg_schedule_no_free();
  }

  struct flight_schedule *fs = flight_schedule_find(city);
  if (fs != NULL)  {
    msg_city_exists(city);
  }

  if (fs == NULL){
  struct flight_schedule *p = flight_schedule_allocate(); // returns pointer and assigns to p
  strncpy(p->destination, city, MAX_CITY_NAME_LEN);
  
  //p->destination = ch; 

  }
}


void flight_schedule_remove(city_t city){
  //struct flight_schedule *temp = flight_schedule_find(city);
  if(flight_schedule_find(city) == NULL){
   msg_city_bad(city);
  }
  else{
    //printf("removed");
    flight_schedule_free(flight_schedule_find(city));
  }
  //printf("removed");
}



void flight_schedule_listAll(void){
  struct flight_schedule *temp = flight_schedules_active; //need to add messages
  while(temp != NULL){
    printf("%s\n", temp->destination); // print temp is disaplying memory address
    temp = temp->next;
  }

}

void flight_schedule_list(city_t city){
  if(flight_schedule_find(city) == NULL){
    msg_city_bad(city);
    return;
  }
  struct flight_schedule *fs = flight_schedule_find(city);
  msg_city_flights(city);
  for(int j =0; j < MAX_FLIGHTS_PER_CITY; j++){
    if(fs->flights[j].time != -1){
      msg_flight_info(fs->flights[j].time, fs->flights[j].available, fs->flights[j].capacity);
    }
  }
  printf("\n");




  /*struct flight_schedule *temp = flight_schedules_active; 
  msg_city_flights(city);
  while(temp != NULL){
    if (strcmp(city,temp->destination) == 0){
      printf("%s\n", temp->destination); //displaying memory address
    }
    temp = temp->next;
  }
  */
}

void flight_schedule_add_flight(city_t city){
  //flight_schedule_add(city);
  struct flight_schedule *temp = flight_schedule_find(city);
  struct flight *flights;
  flights = temp->flights;
  int full = 0;
  int i =0;

  if (temp == NULL){
    msg_city_bad(city);
   // return; // check return 
  }

  while(temp->flights[i].time != -1){ //seg fault here
    
    //if (i == MAX_FLIGHTS_PER_CITY){
    //  full = 1;
    //  break;
    //}
    i++;
      if (i == MAX_FLIGHTS_PER_CITY){  //seg fault is becasue it willl try to index into flights i that is outside range
      full = 1;
      break;
    }

  }

  if(full){
    msg_city_max_flights_reached(city);
  }
  else{

    int t = (flights[i].time);
    
    time_get(&t);
    //printf("ADD FLIGHT time %d", t);
    temp->flights[i].time = t;
    int c = (temp->flights[i].capacity); // look for value of c 
    
    flight_capacity_get(&c);
    //printf("ADD_FLIGHT cap %d", c);
    temp->flights[i].available = c; 
    temp->flights[i].capacity = c;
  }
  /*
  int *t;
  time_get(t); //do this then check if t is 0 to see if valid or not 
  
  if(time_get(t) == 0) { //or just t
    msg_flight_bad_time();      
    return; //maybe use go to end
   }

  int c = 15;
  flight_capacity_get(&c);  //this also has to be done as above to get appropraite io
  
  if (flight_capacity_get(&c) == 0){ //show message plane is full
      //msg_flight_no_seats();
      return;
  }

  //struct flight *flights;
  //flights = temp->flights;
  //flights[i]
  //loop thru flights until open flight then add flight

  for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(flights[i].time == -1){
      flights[i].time = *t;//was flights[i] will this change correct one?
      flights[i].capacity = c;
      break;
    }
  }
  //else return that it failed
  */
}



void flight_schedule_remove_flight(city_t city){
  struct flight_schedule *temp = flight_schedule_find(city);
 //idk if this is the right time to call or even need too

  time_t t;
  time_get(&t); //if 1 continue; else tell user
  int found = 0;
  if (temp == NULL){
    msg_city_bad(city);
  }
  else{
  struct flight *fl;
  fl = temp->flights;

  for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(fl[i].time == t){
      fl[i].time = -1;//was flights[i] will this change correct one? or TIME_NULL
      fl[i].capacity = 0;  
      fl[i].available = 0;
      found = 1;
      break;
    }
  }
  if (found == 0){
    msg_flight_bad_time();
  }
  }

}


void flight_schedule_schedule_seat(city_t city) {
  struct flight_schedule *temp = flight_schedule_find(city);

  struct flight *fl;
  fl = temp->flights;
  time_t t;
  time_get(&t); // was int *t
  if(temp == NULL) {
    printf("null");
    msg_city_bad(city);
   }
   else {
    //int nearest = 0; 
    //int position = 0;
    int i =0;
    printf("here");
    flight_schedule_sort_flights_by_time(temp);
    printf("sorted");
    while(fl[i].time != -1 && i != MAX_FLIGHTS_PER_CITY-1 ){
      flight_schedule_list(city);
      if(fl[i].time < t && fl[i+1].time == -1){
        msg_flight_bad_time();
        break;
      }
      flight_schedule_list(city);
      if (fl[i].time >= t && fl[i].available != 0){ //this and under where else if
        fl[i].available--;
        printf("ava");
        break;
      }
      flight_schedule_list(city);
      if (fl[i].time >= t && fl[i].available == 0){
        msg_flight_no_seats();
        break;
      }
      i++;
    }
   }

/*
  for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(flights[i].time >= *t){
      if(flights[i].time - nearest <= nearest - *t) {  //had time instead of t
        nearest = flights[i].time;
        position = i;
      }
    }
  }
flights[position].available--;
  */
}

 void flight_schedule_unschedule_seat(city_t city){
  struct flight_schedule *temp = flight_schedule_find(city);

  struct flight *flights;
  flights = temp->flights;
  time_t t;
  time_get(&t);

  int nearest = 0;
  int position = 0;

  for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(flights[i].time == q){ //origrinally was time 
     flights[i].available++;
     break;
    }
  }
}


