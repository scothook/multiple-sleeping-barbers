// ---------------------------shop.h--------------------------------------------
// Scot Hook
// CSS 503
// Created: 5/3/2022
// Last Modified: 5/10/2022
//------------------------------------------------------------------------------
// Purpose - Header file for barber shop class with multiple barbers
// -----------------------------------------------------------------------------

#ifndef SHOP_H_
#define SHOP_H_
#include <iostream>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <string>
using namespace std;

#define kDefaultNumChairs 1 // the default number of chairs for waiting = 3
#define kDefaultBarbers 1   // the default number of barbers = 1

class Shop {
public:
  // initialize a shop with a set number of barbers and chairs
  Shop(int nBarbers, int nChairs)
      : num_barbers((nBarbers > 0) ? nBarbers : kDefaultBarbers),
        max_waiting_cust_((nChairs > 0) ? nChairs : kDefaultNumChairs),
        cust_drops_(0) {
    init();
  };

  // initialize a shop with the default barbers and chairs
  Shop()
      : num_barbers(kDefaultBarbers), max_waiting_cust_(kDefaultNumChairs),
        current_customer(0), cust_drops_(0) {
    init();
  };

  // bool visitShop(int id);   // return true only when a customer got a service
  int visitShop(int id); // return barber ID or -1 (not served)
  void leaveShop(int customer_id, int barber_id);
  void helloCustomer(int id);
  void byeCustomer(int id);

  int get_cust_drops() const;
  int chooseBarber();

private:
  const int max_waiting_cust_; // the max number of threads that can wait
  const int num_barbers;

  bool *in_service_;     // of a barber
  bool *money_paid_;     // to a barber
  int *current_customer; // in a barber's chair

  queue<int> waiting_chairs_; // includes the ids of all waiting threads

  int cust_drops_;

  // Mutexes and condition variables to coordinate threads
  // mutex_ is used in conjuction with all conditional variables
  pthread_mutex_t mutex_;
  pthread_cond_t cond_customers_waiting_; // for whole shop
  pthread_cond_t *cond_customer_served_;  // specific to each barber
  pthread_cond_t *cond_barber_paid_;      // specific to each barber
  pthread_cond_t *cond_barber_sleeping_;  // specific to each barber

  void init();              // initializes a shop
  string int2string(int i); // converts an integer to a string for printing

  // prints a message based on the person's number and whether they are a
  // customer or a barber
  void print(int person, string message, bool isCustomer);
};

#endif