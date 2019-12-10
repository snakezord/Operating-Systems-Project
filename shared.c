//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#include "shared.h"
//Lists
flight_departure_t* flights_departure = NULL;
flight_arrival_t* flights_arrival = NULL;
flight_departure_t* flights_departure_copy = NULL;
flight_arrival_t* flights_arrival_copy = NULL;
//QUEUES
arrival_queue_t* arrival_queue = NULL;
departure_queue_t* departure_queue = NULL;

pthread_mutex_t holding_arrival_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t holding_arrival_condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t holding_departure_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t holding_departure_condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t flight_departure_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flight_arrival_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CT_flight_departure_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CT_flight_arrival_mutex = PTHREAD_MUTEX_INITIALIZER;

int TERMINATE = 0;


void print_list_departures(){
    flight_departure_t * current = flights_departure;
    if(current == NULL)
        printf("Departures list is empty!\n");
    while (current != NULL) {
        current = current->next;
    }
}

void print_list_arrivals(){
    flight_arrival_t * current = flights_arrival;
    if(current == NULL)
        printf("Arrivals list is empty!\n");
    while (current != NULL) {
        current = current->next;
    }
}

void append_to_list_departures(flight_departure_t *flight_to_add){
    int id = 1;
    if(flights_departure == NULL){
        flight_to_add->id = id;
        flights_departure = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_departure_t* current = flights_departure;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_list_departures_copy(flight_departure_t *flight_to_add){
    int id = 1;
    if(flights_departure_copy== NULL){
        flight_to_add->id = id;
        flights_departure_copy = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_departure_t* current = flights_departure_copy;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_queue_departures(departure_queue_t *flight_to_add){
    if(departure_queue == NULL){
        departure_queue = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    departure_queue_t* current = departure_queue;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_list_arrivals(flight_arrival_t *flight_to_add){
    int id = 1;
    if(flights_arrival == NULL){
        flight_to_add->id = id;
        flights_arrival = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_arrival_t* current = flights_arrival;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_list_arrivals_copy(flight_arrival_t *flight_to_add){
    int id = 1;
    if(flights_arrival_copy == NULL){
        flight_to_add->id = id;
        flights_arrival_copy = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_arrival_t* current = flights_arrival_copy;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_queue_arrivals(arrival_queue_t *flight_to_add){
    if(arrival_queue == NULL){
        arrival_queue = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    arrival_queue_t* current = arrival_queue;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = flight_to_add;
    current->next->next = NULL;
}

flight_arrival_t * popFirstArrival(flight_arrival_t ** flights_arrival) 
{
    flight_arrival_t * first = *flights_arrival;
    flight_arrival_t * next_node = (*flights_arrival)->next;
    
    if (*flights_arrival == NULL) {
        return NULL;
    }
    free(*flights_arrival);
    *flights_arrival = next_node;
    return first;   
}

void popFirstArrivalQueue(arrival_queue_t ** flights_arrival) 
{
    arrival_queue_t * next_node = (*flights_arrival)->next;

    if (*flights_arrival == NULL) {
        return;
    }
    free(*flights_arrival);
    *flights_arrival = next_node; 
}


flight_departure_t * popFirstDeparture(flight_departure_t ** flights_departure) 
{
    flight_departure_t * first = NULL;
    flight_departure_t * next_node = NULL;

    if (*flights_departure == NULL) {
        return NULL;
    }
    next_node = (*flights_departure)->next;
    first = *flights_departure;
    free(*flights_departure);
    *flights_departure = next_node;
    return first; 
}


void popFirstDepartureQueue(departure_queue_t ** flights_departure) 
{
    departure_queue_t * next_node = (*flights_departure)->next;

    if (*flights_departure == NULL) {
        return;
    }
    free(*flights_departure);
    *flights_departure = next_node;  
}

int count_total_arrivals_queue(){
    int total = 0;
    arrival_queue_t * current = arrival_queue;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    return total;
}

int count_total_departures_queue(){
    int total = 0;
    departure_queue_t * current = departure_queue;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    return total;
}

int count_total_arrivals(){
    int total = 0;
    flight_arrival_t * current = flights_arrival;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    return total;
}

int count_total_departures(){
    int total = 0;
    flight_departure_t * current = flights_departure;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    return total;
}

int msleep(long msec)
{
    struct timespec ts;
    int res;
    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
    return res;
}

int get_empty_slot(){
    int i=0;
    while(sharedMemorySlots->slots[i].occupied != 0){
        i++;
    }
    sharedMemorySlots->slots[i].occupied = 1;
    return i;
}

int cmp( const void *left, const void *right )
{
    const departure_queue_t *a = ( const departure_queue_t *)left;
    const departure_queue_t *b = ( const departure_queue_t *)right;

    if ( b->takeoff < a->takeoff )
    {
        return -1;
    }
    else if ( a->takeoff < b->takeoff )
    {
        return 1;
    }
    else
    {
        return  (  a->takeoff < b->takeoff ) - ( b->takeoff < a->takeoff );
    }
}

/* sorts the linked list by changing next pointers (not data) */
void DepartureMergeSort(departure_queue_t** headRef) 
{ 
    departure_queue_t* head = *headRef; 
    departure_queue_t* a; 
    departure_queue_t* b; 
  
    /* Base case -- length 0 or 1 */
    if ((head == NULL) || (head->next == NULL)) { 
        return; 
    } 
  
    /* Split head into 'a' and 'b' sublists */
    DepartureFrontBackSplit(head, &a, &b); 
  
    /* Recursively sort the sublists */
    DepartureMergeSort(&a); 
    DepartureMergeSort(&b);
  
    /* answer = merge the two sorted lists together */
    *headRef = DepartureSortedMerge(a, b); 
} 
  
/* See https:// www.geeksforgeeks.org/?p=3622 for details of this  
function */
departure_queue_t* DepartureSortedMerge(departure_queue_t* a, departure_queue_t* b) 
{ 
    departure_queue_t* result = NULL; 
  
    /* Base cases */
    if (a == NULL) 
        return (b); 
    else if (b == NULL) 
        return (a); 
  
    /* Pick either a or b, and recur */
    if (a->takeoff <= b->takeoff) { 
        result = a; 
        result->next = DepartureSortedMerge(a->next, b); 
    } 
    else { 
        result = b; 
        result->next = DepartureSortedMerge(a, b->next); 
    } 
    return (result); 
} 
  
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves, 
    and return the two lists using the reference parameters. 
    If the length is odd, the extra node should go in the front list. 
    Uses the fast/slow pointer strategy. */
void DepartureFrontBackSplit(departure_queue_t* source, departure_queue_t** frontRef, departure_queue_t** backRef) 
{ 
    departure_queue_t* fast; 
    departure_queue_t* slow; 
    slow = source; 
    fast = source->next; 
  
    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL) { 
        fast = fast->next; 
        if (fast != NULL) { 
            slow = slow->next; 
            fast = fast->next; 
        } 
    } 
  
    /* 'slow' is before the midpoint in the list, so split it in two 
    at that point. */
    *frontRef = source; 
    *backRef = slow->next; 
    slow->next = NULL; 
}


/* sorts the linked list by changing next pointers (not data) */
void ArrivalMergeSort(arrival_queue_t** headRef) 
{ 
    arrival_queue_t* head = *headRef; 
    arrival_queue_t* a; 
    arrival_queue_t* b; 
  
    /* Base case -- length 0 or 1 */
    if ((head == NULL) || (head->next == NULL)) { 
        return; 
    } 
  
    /* Split head into 'a' and 'b' sublists */
    ArrivalFrontBackSplit(head, &a, &b); 
  
    /* Recursively sort the sublists */
    ArrivalMergeSort(&a); 
    ArrivalMergeSort(&b); 
  
    /* answer = merge the two sorted lists together */
    *headRef = ArrivalSortedMerge(a, b); 
} 
  
/* See https:// www.geeksforgeeks.org/?p=3622 for details of this  
function */
arrival_queue_t* ArrivalSortedMerge(arrival_queue_t* a, arrival_queue_t* b) 
{ 
    arrival_queue_t* result = NULL; 
  
    /* Base cases */
    if (a == NULL) 
        return (b); 
    else if (b == NULL) 
        return (a); 
  
    /* Pick either a or b, and recur */
    if (a->eta <= b->eta) { 
        result = a; 
        result->next = ArrivalSortedMerge(a->next, b); 
    } 
    else { 
        result = b; 
        result->next = ArrivalSortedMerge(a, b->next); 
    } 
    return (result); 
} 
  
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves, 
    and return the two lists using the reference parameters. 
    If the length is odd, the extra node should go in the front list. 
    Uses the fast/slow pointer strategy. */
void ArrivalFrontBackSplit(arrival_queue_t* source, arrival_queue_t** frontRef, arrival_queue_t** backRef) 
{ 
    arrival_queue_t* fast; 
    arrival_queue_t* slow; 
    slow = source; 
    fast = source->next; 
  
    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL) { 
        fast = fast->next; 
        if (fast != NULL) { 
            slow = slow->next; 
            fast = fast->next; 
        } 
    } 
  
    /* 'slow' is before the midpoint in the list, so split it in two 
    at that point. */
    *frontRef = source; 
    *backRef = slow->next; 
    slow->next = NULL; 
}