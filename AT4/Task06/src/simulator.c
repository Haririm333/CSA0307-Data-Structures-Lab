#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "order_system.h"

static const char *sample_restaurants[] = {
    "A2B Veg", "Biryani Blues", "Burger King", "Chai Point", "Dominos",
    "Empire", "FreshMenu", "Haldirams", "KFC", "McDonalds",
    "Paradise Biryani", "Pizza Hut", "Subway", "Taco Bell", "Wow Momo"
};
#define NUM_RESTAURANTS ((int)(sizeof(sample_restaurants) / sizeof(sample_restaurants[0])))

static double get_time_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

typedef struct {
    int total_orders;
    double place_order_total_ms;
    double place_order_avg_us;
    double lookup_total_ms;
    double lookup_avg_us;
    double assign_total_ms;
    double assign_avg_us;
    double cancel_total_ms;
    double cancel_avg_us;
    double report_total_ms;
} SimulationResult;

static SimulationResult run_simulation(int num_orders)
{
    SimulationResult res;
    res.total_orders = num_orders;

    DeliverySystem *sys = system_init(num_orders);

    srand(1337 + num_orders);

    long long current_sim_time = 100000;

    /* 1. Place Orders Benchmark */
    double t0 = get_time_sec();
    for (int i = 0; i < num_orders; i++) {
        int id = i + 1;
        const char *rest = sample_restaurants[rand() % NUM_RESTAURANTS];
        int amount = (rand() % 40 + 1) * 50; /* Rs 50 to 2000 */
        long long created = current_sim_time + (rand() % 3600);
        long long promised = created + (rand() % 1800 + 1200); /* 20-50 min later */

        place_order(sys, id, rest, amount, created, promised);
    }
    double t1 = get_time_sec();
    res.place_order_total_ms = (t1 - t0) * 1000.0;
    res.place_order_avg_us = (res.place_order_total_ms / num_orders) * 1000.0;

    /* 2. Lookup Benchmark (Sample 20,000 lookups) */
    int num_lookups = 20000;
    t0 = get_time_sec();
    volatile int found_count = 0;
    for (int i = 0; i < num_lookups; i++) {
        int target_id = (rand() % num_orders) + 1;
        Order *o = lookup(sys, target_id);
        if (o) found_count++;
    }
    (void)found_count;
    t1 = get_time_sec();
    res.lookup_total_ms = (t1 - t0) * 1000.0;
    res.lookup_avg_us = (res.lookup_total_ms / num_lookups) * 1000.0;

    /* 3. Random Cancellations (~5% of orders) */
    int num_cancels = num_orders / 20;
    t0 = get_time_sec();
    int actual_cancels = 0;
    for (int i = 0; i < num_cancels; i++) {
        int cancel_id = (rand() % num_orders) + 1;
        if (cancel_order(sys, cancel_id)) {
            actual_cancels++;
        }
    }
    t1 = get_time_sec();
    res.cancel_total_ms = (t1 - t0) * 1000.0;
    res.cancel_avg_us = (actual_cancels > 0) ? (res.cancel_total_ms / actual_cancels) * 1000.0 : 0.0;

    /* 4. Assign Next Rider (~50% of orders assigned) */
    int num_assigns = num_orders / 2;
    current_sim_time += 1800; /* advance 30 min */
    t0 = get_time_sec();
    int actual_assigns = 0;
    for (int i = 0; i < num_assigns; i++) {
        Order *assigned = assign_next_rider(sys, current_sim_time);
        if (assigned) actual_assigns++;
    }
    t1 = get_time_sec();
    res.assign_total_ms = (t1 - t0) * 1000.0;
    res.assign_avg_us = (actual_assigns > 0) ? (res.assign_total_ms / actual_assigns) * 1000.0 : 0.0;

    /* 5. Hourly Report Generation */
    Order **report_buf = malloc(num_orders * sizeof(Order *));
    int report_count = 0;
    t0 = get_time_sec();
    hourly_report(sys, report_buf, &report_count);
    t1 = get_time_sec();
    res.report_total_ms = (t1 - t0) * 1000.0;
    free(report_buf);

    system_free(sys);
    return res;
}

int main(void)
{
    printf("=========================================================================================================\n");
    printf("          TASK 6: PRODUCTION SCALE SIMULATOR (50,000 and 100,000 ORDERS)\n");
    printf("=========================================================================================================\n");

    int test_loads[] = {50000, 100000};
    int n_loads = 2;

    FILE *csv = fopen("Task06/output/simulator_metrics.csv", "w");
    if (!csv) {
        perror("Failed to open simulator_metrics.csv");
        return 1;
    }
    fprintf(csv, "Orders,Place_Tot_Ms,Place_Avg_Us,Lookup_Avg_Us,Cancel_Avg_Us,Assign_Avg_Us,Report_Tot_Ms\n");

    for (int l = 0; l < n_loads; l++) {
        int n = test_loads[l];
        printf("Executing simulation workload for N = %d orders...\n", n);
        SimulationResult r = run_simulation(n);

        printf("\n--- RESULTS FOR N = %d ORDERS ---\n", n);
        printf("  Place Order:     Total %8.2f ms | Avg %6.2f us per order (Hash + Heap Push)\n",
               r.place_order_total_ms, r.place_order_avg_us);
        printf("  Lookup by ID:    Total %8.2f ms | Avg %6.2f us per lookup (O(1) Hash Table)\n",
               r.lookup_total_ms, r.lookup_avg_us);
        printf("  Cancel Order:    Total %8.2f ms | Avg %6.2f us per cancel (O(log n) Heap Purge)\n",
               r.cancel_total_ms, r.cancel_avg_us);
        printf("  Assign Rider:    Total %8.2f ms | Avg %6.2f us per assign (O(log n) Pop Max)\n",
               r.assign_total_ms, r.assign_avg_us);
        printf("  Hourly Report:   Total %8.2f ms for %d orders (Stable Merge Sort)\n\n",
               r.report_total_ms, n);

        fprintf(csv, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                n, r.place_order_total_ms, r.place_order_avg_us, r.lookup_avg_us,
                r.cancel_avg_us, r.assign_avg_us, r.report_total_ms);
    }
    printf("=========================================================================================================\n");

    fclose(csv);
    printf("Simulation metrics saved to Task06/output/simulator_metrics.csv\n");
    return 0;
}
