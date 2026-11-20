#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "order_system.h"

static void test_cancel_removes_from_heap(void)
{
    printf("[TEST 1] Testing order cancellation removes from heap and syncs status...\n");
    DeliverySystem *sys = system_init(16);

    place_order(sys, 101, "PizzaHut", 500, 1000, 1100);
    place_order(sys, 102, "BurgerKing", 300, 1005, 1050); /* Most overdue */
    place_order(sys, 103, "Subway", 200, 1010, 1200);
    place_order(sys, 104, "KFC", 400, 1015, 1080);

    /* Verify lookup works */
    Order *o102 = lookup(sys, 102);
    assert(o102 != NULL);
    assert(o102->id == 102);
    assert(o102->status == ORDER_PENDING);
    assert(o102->heap_index >= 0);

    /* Cancel the most overdue order (102) */
    bool cancelled = cancel_order(sys, 102);
    assert(cancelled == true);
    assert(o102->status == ORDER_CANCELLED);
    assert(o102->heap_index == -1);

    /* Subsequent cancel returns false */
    assert(cancel_order(sys, 102) == false);

    /* Pop all orders from heap and verify 102 is never assigned */
    int assigned_count = 0;
    while (sys->heap->size > 0) {
        Order *assigned = assign_next_rider(sys, 2000);
        assert(assigned != NULL);
        assert(assigned->id != 102); /* 102 must NOT appear! */
        assigned_count++;
    }
    assert(assigned_count == 3); /* exactly 101, 104, 103 */

    system_free(sys);
    printf("  -> PASS: Order 102 was cleanly purged from heap and cancelled in hash table.\n");
}

static void test_assignment_order(void)
{
    printf("[TEST 2] Testing assignment order strictly follows maximum delay...\n");
    DeliverySystem *sys = system_init(16);

    /* Promised times:
       ID 1: promised 1500 (delay = 500 at t=2000)
       ID 2: promised 1100 (delay = 900 at t=2000) -> 1st priority
       ID 3: promised 1300 (delay = 700 at t=2000) -> 2nd priority
       ID 4: promised 1800 (delay = 200 at t=2000) -> 4th priority
    */
    place_order(sys, 1, "Dominos", 400, 1000, 1500);
    place_order(sys, 2, "Dominos", 400, 1000, 1100);
    place_order(sys, 3, "Dominos", 400, 1000, 1300);
    place_order(sys, 4, "Dominos", 400, 1000, 1800);

    Order *first = assign_next_rider(sys, 2000);
    assert(first != NULL && first->id == 2);

    Order *second = assign_next_rider(sys, 2000);
    assert(second != NULL && second->id == 3);

    Order *third = assign_next_rider(sys, 2000);
    assert(third != NULL && third->id == 1);

    Order *fourth = assign_next_rider(sys, 2000);
    assert(fourth != NULL && fourth->id == 4);

    assert(assign_next_rider(sys, 2000) == NULL);

    system_free(sys);
    printf("  -> PASS: Riders assigned in exact mathematical order of overdue delay (2 -> 3 -> 1 -> 4).\n");
}

static void test_report_stability(void)
{
    printf("[TEST 3] Testing hourly report sorting and stability on equal keys...\n");
    DeliverySystem *sys = system_init(16);

    place_order(sys, 10, "ZomatoKitchen", 300, 100, 500);
    place_order(sys, 20, "BiryaniBlues",  450, 101, 500);
    place_order(sys, 30, "BiryaniBlues",  200, 102, 500);
    place_order(sys, 40, "BiryaniBlues",  450, 103, 500); /* Equal amount 450 to #20 */
    place_order(sys, 50, "BiryaniBlues",  450, 104, 500); /* Equal amount 450 to #20 & #40 */
    place_order(sys, 60, "AppleDine",     100, 105, 500);

    Order *report[16];
    int count = 0;
    hourly_report(sys, report, &count);
    assert(count == 6);

    /* Check restaurant alphabetical ordering:
       0: AppleDine
       1: BiryaniBlues (200)
       2: BiryaniBlues (450 - ID 20)
       3: BiryaniBlues (450 - ID 40)
       4: BiryaniBlues (450 - ID 50)
       5: ZomatoKitchen (300)
    */
    assert(strcmp(report[0]->restaurant, "AppleDine") == 0);

    assert(strcmp(report[1]->restaurant, "BiryaniBlues") == 0 && report[1]->amount == 200);

    /* Equal amounts (450) must strictly preserve arrival sequence: 20 -> 40 -> 50 */
    assert(strcmp(report[2]->restaurant, "BiryaniBlues") == 0 && report[2]->id == 20);
    assert(strcmp(report[3]->restaurant, "BiryaniBlues") == 0 && report[3]->id == 40);
    assert(strcmp(report[4]->restaurant, "BiryaniBlues") == 0 && report[4]->id == 50);

    assert(strcmp(report[5]->restaurant, "ZomatoKitchen") == 0);

    system_free(sys);
    printf("  -> PASS: Report is strictly sorted and preserves FIFO stability for equal amounts.\n");
}

int main(void)
{
    printf("================================================================================\n");
    printf("               TASK 6: SYSTEM VERIFICATION TEST SUITE (ASSERTS)\n");
    printf("================================================================================\n");

    test_cancel_removes_from_heap();
    test_assignment_order();
    test_report_stability();

    printf("\n>>> ALL TESTS PASSED SUCCESSFULLY! (Zero Assert Failures) <<<\n");
    return 0;
}
