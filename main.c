#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "BENSCHILLIBOWL.h"

#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant
BENSCHILLIBOWL *bcb;

/**
 * Thread function that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long) tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        // Allocate memory for an order
        Order* order = (Order*)malloc(sizeof(Order));
        if (!order) {
            perror("Failed to allocate memory for order");
            pthread_exit(NULL);
        }

        // Select a random menu item
        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;

        printf("Customer #%d placing order for %s\n", customer_id, order->menu_item);

        // Add the order to the restaurant
        AddOrder(bcb, order);
    }

    pthread_exit(NULL);
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long) tid;
    int orders_fulfilled = 0;

    while (1) {
        // Get an order from the restaurant
        Order* order = GetOrder(bcb);

        if (!order) { // No more orders to fulfill
            break;
        }

        // Fulfill the order
        printf("Cook #%d fulfilled order for customer #%d (%s)\n", cook_id, order->customer_id, order->menu_item);
        orders_fulfilled++;

        // Free the memory allocated for the order
        free(order);
    }

    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    pthread_exit(NULL);
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    // Open the restaurant
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    // Create cook threads
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)(long)(i + 1));
    }

    // Create customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)(long)(i + 1));
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // Close the restaurant once all customers are done
    CloseRestaurant(bcb);

    // Wait for all cook threads to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }

    printf("Restaurant is now closed.\n");
    return 0;
}
