#ifndef FAULTS_H
#define FAULTS_H

#include "graph.h"
#include "unionfind.h"
#include <stdbool.h>

typedef struct Bridge {
    int u;
    int v;
} Bridge;

typedef struct FaultResult {
    Bridge* bridges;
    int bridge_count;
    int* articulation_points;
    int articulation_count;
} FaultResult;

typedef struct SimResult {
    int components_before;
    int components_after;
    ComponentList* disconnected_groups;
} SimResult;

FaultResult* faults_detect(const Graph* g);
void fault_result_free(FaultResult* res);

SimResult* faults_simulate_link(Graph* g, const char* name_a, const char* name_b);
SimResult* faults_simulate_device(Graph* g, const char* name);
void sim_result_free(SimResult* res);

#endif
