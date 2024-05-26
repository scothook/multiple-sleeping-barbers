// ---------------------------shop.cpp------------------------------------------
// Scot Hook
// CSS 503
// Created: 5/3/2022
// Last Modified: 5/10/2022
//------------------------------------------------------------------------------
// Purpose - Implimentation file for barber shop class with multiple barbers
// -----------------------------------------------------------------------------

#include "shop.h"

// Initializes the shop  giving each barber a set of checkpoints in the form
// of conditional variables set to their default value.
void Shop::init() {
  in_service_ = new bool[num_barbers];
  money_paid_ = new bool[num_barbers];
  current_customer = new int[num_barbers];

  pthread_mutex_init(&mutex_, NULL);
  pthread_cond_init(&cond_customers_waiting_, NULL);

  cond_customer_served_ = new pthread_cond_t[num_barbers];
  cond_barber_paid_ = new pthread_cond_t[num_barbers];
  cond_barber_sleeping_ = new pthread_cond_t[num_barbers];

  for (int i = 0; i < num_barbers; i++) {
    in_service_[i] = 0;
    money_paid_[i] = false;
    current_customer[i] = false;
    pthread_cond_init(&cond_customer_served_[i], NULL);
    pthread_cond_init(&cond_barber_paid_[i], NULL);
    pthread_cond_init(&cond_barber_sleeping_[i], NULL);
  }
}

// converts a given int i and returns a string equivalent to i
string Shop::int2string(int i) {
  stringstream out;
  out << i;
  return out.str();
}

// prints a message with the given person's number and type
void Shop::print(int person, string message, bool isCustomer) {
  cout << ((isCustomer) ? "customer[" : "barber  [") << person
       << "]: " << message << endl;
}

// outputs the number of rejected customers
int Shop::get_cust_drops() const { return cust_drops_; }

//
int Shop::visitShop(int id) {
  pthread_mutex_lock(&mutex_);

  // If all chairs are full then leave shop
  if (waiting_chairs_.size() == max_waiting_cust_) {
    print(id, "leaves the shop because of no available waiting chairs.", true);
    ++cust_drops_;
    pthread_mutex_unlock(&mutex_);
    return -1;
  }

  // If someone is being served or transitioning to service chair
  // then take a chair and wait for service
  if (chooseBarber() == -1 || !waiting_chairs_.empty()) {
    waiting_chairs_.push(id);
    print(id,
          "takes a waiting chair. # waiting seats available = " +
              int2string(max_waiting_cust_ - waiting_chairs_.size()),
          true);
    pthread_cond_wait(&cond_customers_waiting_, &mutex_);
    waiting_chairs_.pop();
  }

  print(id,
        "moves to the service chair. # waiting seats available = " +
            int2string(max_waiting_cust_ - waiting_chairs_.size()),
        true);

  int chosenBarber = chooseBarber(); // pick a barber chair
  if (current_customer[chosenBarber] == 0) {
    current_customer[chosenBarber] = id;
    in_service_[chosenBarber] = true;
  }

  // wake up the barber just in case if he is sleeping
  pthread_cond_signal(&cond_barber_sleeping_[chosenBarber]);
  pthread_mutex_unlock(&mutex_);
  return chosenBarber;
}

void Shop::leaveShop(int customer_id, int barber_id) {
  pthread_mutex_lock(&mutex_);
  // Wait for service to be completed
  print(customer_id, "wait for the hair-cut to be done", true);
  while (in_service_[barber_id] == true) {
    pthread_cond_wait(&cond_customer_served_[barber_id], &mutex_);
  }

  // Pay the barber and signal barber appropriately
  money_paid_[barber_id] = true;
  pthread_cond_signal(&cond_barber_paid_[barber_id]);
  print(customer_id, "says good-bye to the barber.", true);
  pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int id) {
  pthread_mutex_lock(&mutex_);

  // If no customers than barber can sleep
  if (waiting_chairs_.empty() && current_customer[id] == 0) {
    print(id, "sleeps because of no customers.", false);
    pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
  }
  if (current_customer[id] == 0) // check if the customer, sit down.
  {
    pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
  }
  print(id, "starts a hair-cut service for " + int2string(current_customer[id]),
        false);
  pthread_mutex_unlock(&mutex_);
}

// function for
void Shop::byeCustomer(int id) {
  pthread_mutex_lock(&mutex_);
  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[id] = false;
  print(id,
        "says he's done with a hair-cut service for " +
            int2string(current_customer[id]),
        false);
  money_paid_[id] = false;
  pthread_cond_signal(&cond_customer_served_[id]);
  while (money_paid_[id] == false) {
    pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
  }
  // Signal to customer to get next one
  current_customer[id] = 0;
  print(id, "calls in another customer", false);
  pthread_cond_signal(&cond_customers_waiting_);
  pthread_mutex_unlock(&mutex_); // unlock
}

// finds an available barber from lowest number to highest
int Shop::chooseBarber() {
  for (int i = 0; i < num_barbers; i++) {
    if (current_customer[i] == 0) {
      return i;
    }
  }
  return -1;
}