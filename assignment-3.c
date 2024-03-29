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
#define MAX_FLIGHTS_PER_CITY 5 // should be 5
#define MAX_DEFAULT_SCHEDULES 50  // should be 50 if this is 2 test4 fails idk why

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

//flight schedule allocate
//takes a schedule off the free list and places it on the active list
//returns pointer to updated flight schedule
//first fs points to the free list and takes off a node, if that wasn't the last one on free set the new first node's prev to be null
//add the new node onto the active list
//if there is another node on active add new node to front of list
//the new active schedule leads with fs and returns that value
struct flight_schedule * flight_schedule_allocate(void){
  struct flight_schedule *fs = flight_schedules_free;
  flight_schedules_free = fs->next; //first fs points to the free list and takes off a node
  if(flight_schedules_free != NULL){ //if that wasn't the last one on free set the new first node's prev to be null
    flight_schedules_free->prev = NULL; 
  }
  fs->next = flight_schedules_active;
  if(flight_schedules_active != NULL){ //if there is another node on active add new node to front of list
    flight_schedules_active->prev = fs;
  }
  flight_schedules_active = fs; //the new active schedule leads with fs and returns that value
  return (fs);
}

//flight schedule free
//takes schedule off active list, resets it, then places the node back on free list
//if the parameter is the first node accompanied with others take first node off and then set active to the next null and its prev to null
//if there is only one node on active, take node off set it to null
//if it is the last node take off end by setting the prev node to null
//otherwise if in middle set surrounding nodes to eachother
//reset the given schedule
//if no schedules on free add given to free
//otherwise add node to front of free
void flight_schedule_free(struct flight_schedule *fs){
  if (fs->next != NULL && fs == flight_schedules_active) { //if the parameter is the first node accompanied with others take first node off and then set active to the next null and its prev to null
    flight_schedules_active = fs->next;
    flight_schedules_active->prev = NULL;
  }
  else if (fs == flight_schedules_active){   //if there is only one node on active, take node off set it to null
    flight_schedules_active = NULL;
  }
  else if (fs->next == NULL){ //if it is the last node take off end by setting the prev node to null
    fs->prev->next = NULL;
  }
  else{ //otherwise if in middle set surrounding nodes to eachother
    fs->prev->next = fs->next;
    fs->next->prev = fs->prev;
  }
  flight_schedule_reset(fs); //reset schedule

  if (flight_schedules_free == NULL){ //if no schedules on free add given to free
    flight_schedules_free = fs;
  }
  else{//otherwise add node to front of free
    flight_schedules_free->prev = fs;
    fs->next = flight_schedules_free;
    flight_schedules_free = fs;
    fs->prev = NULL;
  }
}

//Takes as input a city and traverses the active flight list until it finds the flight schedule for this
//city, if it exists
//Returns the flight schedule of said city
//first the function finds the city and assigns it to temp
//traverse active list using temp, check using string compare to see if city is on active list
//if it is found return it 
struct flight_schedule *flight_schedule_find(city_t city){
  struct flight_schedule *temp = flight_schedules_active; //the function finds the city and assigns it to temp
  while(temp != NULL){ //traverse active list using temp
    if (strcmp(city,temp->destination) == 0){ //check using string compare to see if city is on active list 
      return(temp);
    }
    temp = temp->next;
  }
}

//takes an input city and adds a flight schedule for given city
//first makes sure there is room to add new city, if not msg and return
//find the city and set it equal to fs 
//if the city is already on the list, this is told to user 
//otherwise allocate is called to make room to add city then using string copy to copy given city to new node
void flight_schedule_add(city_t city){
  //first cheeck to see if city exists
  if (flight_schedules_free == NULL){ //makes sure there is room to add new city, if not msg and return
    msg_schedule_no_free();
    return;
  }
  struct flight_schedule *fs = flight_schedule_find(city); //find the city and set it equal to fs 
  if (fs != NULL)  { //if on list msg is output
    msg_city_exists(city);
    return;
  }
  if (fs == NULL){ //flight not yet added
    struct flight_schedule *p = flight_schedule_allocate();  //allocate is called to make room to add city
    strncpy(p->destination, city, MAX_CITY_NAME_LEN); //using string copy to copy given city to new node
  }
}

//takes as input a city and removes the flight schedule for that city if it exists
//checks to make sure city exists if it doesn't tells user
//otherwise find schedule for city using find then use free to clear it 
void flight_schedule_remove(city_t city){
  if(flight_schedule_find(city) == NULL){ //checks to make sure city exists if it doesn't tells user
    msg_city_bad(city);
    return;
  }
  else{
    flight_schedule_free(flight_schedule_find(city)); //otherwise find schedule for city using find then use free to clear it
  }
}


//lists all existing active flight schedules
//uses temp to traverse active printing all flights
void flight_schedule_listAll(void){
  struct flight_schedule *temp = flight_schedules_active; 
  while(temp != NULL){
    printf("%s\n", temp->destination);
    temp = temp->next;
  }
}

//lists all of the flights for a given city with there flight info
//checks to make sure city exists if it doesn't tells user
//fs points to the given city msg about city then traverses displaying flight info for each flight
void flight_schedule_list(city_t city){
  if(flight_schedule_find(city) == NULL){ //checks to make sure city exists if it doesn't tells user
    msg_city_bad(city);
    return;
  }
  struct flight_schedule *fs = flight_schedule_find(city);
  msg_city_flights(city); //find the city and msg that 
  for(int j =0; j < MAX_FLIGHTS_PER_CITY; j++){ //loop through schedule displaying attributes 
    if(fs->flights[j].time != -1){
      msg_flight_info(fs->flights[j].time, fs->flights[j].available, fs->flights[j].capacity);
    }
  }
  printf("\n");
}


//takes as input city and adds a given flight to that city uses get time and get capacity
//first the function finds the city and assigns it to temp, temp is then used to set flights to the citys flight schedule 
//next is gets the time and capcity and assures they are valid values 
//checks to make sure a city was found
//uses while loop to check for next node to add new flight too
//if found loop is broken and attributes are set accordingly otherwise the arrray is full and displays that
void flight_schedule_add_flight(city_t city){
  struct flight_schedule *temp = flight_schedule_find(city); //first the function finds the city and assigns it to temp 
  struct flight *flights;
  flights = temp->flights; // temp is then used to set flights to the citys flight schedule
  int full = 0;
  int i = 0;
  time_t t;
  if(time_get(&t) == 0){ //had to change time_get and capcity get to the front and check for good vals, so doesn't result in bad command
    return;
  }
  int c;
  if (flight_capacity_get(&c) == 0){ //gets the time and capcity and assures they are valid values 
    return;
  }
  if (temp == NULL){ //checks to make sure a city was found
    msg_city_bad(city);
    return;
  }
  while(temp->flights[i].time != -1){ //uses while loop to check for next node to add new flight too
    i++;
    if (i == MAX_FLIGHTS_PER_CITY){ 
      full = 1;
      break;
    }
  }
  if(full){ //if full loop is broken the arrray is full and displays that
    msg_city_max_flights_reached(city);
    return;
  }
  else{ //otherwise if found set attributes accordingly
    temp->flights[i].time = t;
    temp->flights[i].available = c; 
    temp->flights[i].capacity = c;
  }
}

//takes as input a city and removes given fliht for that city
//first the function finds the city and assigns it to temp, then makes sure city is valid 
//next we read in the time for the flight 
//temp is then used to set fl to the citys flight schedule, then this is used to traverse flights
//uses if that time is found sets attributes to remove, otherwise time was bad and that is displayed
void flight_schedule_remove_flight(city_t city){
  struct flight_schedule *temp = flight_schedule_find(city);
  int found = 0;
  if (temp == NULL){ //function finds the city and assigns it to temp, then makes sure city is valid , displaying messae if bad
    msg_city_bad(city);
    return;
  }
  else{
    time_t t;
    time_get(&t); // we read in the time for the flight 
    struct flight *fl;
    fl = temp->flights; 
    for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){ //temp is then used to set fl to the citys flight schedule, then this is used to travere flights
      if(fl[i].time == t){ //if that time is found sets attributes to remove, otherwise time was bad and that is displayed
        fl[i].time = -1;
        fl[i].capacity = 0;  
        fl[i].available = 0;
        found = 1;
        break;
      }
    }
    if (found == 0){ //otherwise bad time displayed
      msg_flight_bad_time();
    }
  }
}

//takes as input city and schedules seat on a flight for tahat city
//the user can specify a time and your program should schedule the next avaiable flight from given time
//first the function finds the city and assigns it to temp, then makes sure city is valid 
//temp is then used to set fl to the citys flight schedule
//next we read in the time for the flight and check its valid
//then the flights are sorted, next i loop through checking for the next avaible flight and make sure it has seats 
//if it does seat is scheduled otherwise no seats avaiable 
void flight_schedule_schedule_seat(city_t city) {
  struct flight_schedule *temp = flight_schedule_find(city);
  if(temp == NULL) {
    msg_city_bad(city); // if city doesn't exist display bad city msg
  }
  struct flight *fl;
  fl = temp->flights;
  time_t t;
  if (time_get(&t) == 0){ //make sure time is valid first to avoid bad command  error
    return;
  }
   else {
    int i = 0;
    flight_schedule_sort_flights_by_time(temp); //sort schedules by time
    for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){ //loop through checking for the next avaible flight and make sure it has seats 
      if (fl[i].time >= t && fl[i].available != 0){  //if found with seats schedules seat
        fl[i].available--;
        return;
      }
      if (fl[i].time >= t && fl[i].available == 0){ //if found but no seats, msg no seats
        msg_flight_no_seats();
        return;
      }
    }
    msg_flight_no_seats(); //if not found no seats
  }
}


//Takes as input a city and unschedules a seat on a given flight for this city.
//The user must specify the exact time for the flight that they are unscheduling.
//first the function finds the city and assigns it to temp
//next we read in the time for the flight and check its valid
//temp is then used to set flights to the citys flight schedule
//loop through checking to see if exact time found and if there are availabale seats to unschedule, update or display message accordingly

 void flight_schedule_unschedule_seat(city_t city){
  struct flight_schedule *temp = flight_schedule_find(city);
  time_t t; //needed to check time at begining to fix bad command bug 
  if (time_get(&t) == 0){ //check to make sure valid
    return;
  }
  struct flight *flights;
  flights = temp->flights; //temp is then used to set flights to the citys flight schedule

  for(int i = 0; i < MAX_FLIGHTS_PER_CITY; i++){//loop through checking to see if exact time found
    if(flights[i].time == t && (flights[i].available < flights[i].capacity)){ //if there are availabale seats to unschedule, update 
     flights[i].available++;
     return;
    }
    if(flights[i].time == t && (flights[i].available == flights[i].capacity)){ //if no seats display message accordingly
      msg_flight_all_seats_empty();
      return;
    }
  }
  msg_flight_bad_time(); //given time didnt match time in schedule
}
