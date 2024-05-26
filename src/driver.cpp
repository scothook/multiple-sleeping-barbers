// -------------------------driver.cpp------------------------------------------
// Scot Hook
// CSS 503
// Created: 5/3/2022
// Last Modified: 5/10/2022
//------------------------------------------------------------------------------
// Purpose - Driver file to run the multiple sleeping barbers problem
// -----------------------------------------------------------------------------
// Command line arguments received are:
// argv[1] num_barbers     The number of barbers working in your barbershop
// argv[2] num_chairs      No. chairs available for customers to wait in
// argv[3] num_customers   The number of customers who need a haircut
// argv[4] service_time    Each barber’s service time (in μ seconds).
// -----------------------------------------------------------------------------

#include "shop.h"
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

void *barber(void *);
void *customer(void *);

// This class is used as a way to pass more than one argument to a thread.
class ThreadParam {
public:
  ThreadParam(Shop *shop, int id, int service_time)
      : shop(shop), id(id), service_time(service_time){};
  Shop *shop;
  int id;
  int service_time;
};

int main(int argc, char *argv[]) {
  // Read arguments from command line
  if (argc != 5) {
    cout << "Usage: num_barbers num_chairs num_customers service_time" << endl;
    return -1;
  }
  int num_barbers = atoi(argv[1]);
  int num_chairs = atoi(argv[2]);
  int num_customers = atoi(argv[3]);
  int service_time = atoi(argv[4]);

  pthread_t barber_threads[num_barbers];
  pthread_t customer_threads[num_customers];
  Shop shop(num_barbers, num_chairs);

  // create barber threads and then customer threads at random intervals
  for (int i = 0; i < num_barbers; i++) {
    ThreadParam *barber_param = new ThreadParam(&shop, i, service_time);
    pthread_create(&barber_threads[i], NULL, barber, barber_param);
  }
  for (int i = 0; i < num_customers; i++) {
    usleep(rand() % 1000);
    int id = i + 1;
    ThreadParam *customer_param = new ThreadParam(&shop, id, 0);
    pthread_create(&customer_threads[i], NULL, customer, customer_param);
  }

  // Wait for customers to finish and cancel barbers
  for (int i = 0; i < num_customers; i++) {
    pthread_join(customer_threads[i], NULL);
  }

  for (int i = 0; i < num_barbers; i++) {
    pthread_cancel(barber_threads[i]);
  }
  cout << "# customers who didn't receive a service = " << shop.get_cust_drops()
       << endl;
  return 0;
}

// the barber thread function
void *barber(void *arg) {
  // extract parameters
  ThreadParam &param = *(ThreadParam *)arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  int service_time = param.service_time;
  delete &param;

  // keep working until being terminated by the main
  while (true) {
    // shop.helloCustomer(); // temp for 1 barber
    shop.helloCustomer(id); // pick up a new customer
    usleep(service_time);
    // shop.byeCustomer(); // temp for 1 barber
    shop.byeCustomer(id); // release the customer
  }
  pthread_exit(NULL);
}

void *customer(void *arg) {
  ThreadParam &param = *(ThreadParam *)arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  delete &param;

  int barber = -1;
  if ((barber = shop.visitShop(id)) != -1) {
    shop.leaveShop(id, barber); // wait until my service is finished
  }
  pthread_exit(NULL);
}