#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TOTAL_RECORDS 100000

static const char *other_states[] = {
    "KA", "KL", "AP", "TS", "MH", "DL", "GJ", "RJ", "UP", "WB", "PY"
};
#define NUM_OTHER_STATES ((int)(sizeof(other_states) / sizeof(other_states[0])))

int main(void)
{
    const char *out_path = "Task01/input/registrations.txt";
    FILE *fp = fopen(out_path, "w");
    if (!fp) {
        perror("Failed to open output file");
        return 1;
    }

    srand(1337); /* Fixed seed for reproducibility */

    for (int i = 0; i < TOTAL_RECORDS; i++) {
        char state[3];
        /* 80% TN, 20% other states */
        if (rand() % 100 < 80) {
            state[0] = 'T';
            state[1] = 'N';
            state[2] = '\0';
        } else {
            int idx = rand() % NUM_OTHER_STATES;
            state[0] = other_states[idx][0];
            state[1] = other_states[idx][1];
            state[2] = '\0';
        }

        int district = (rand() % 99) + 1; /* 01 - 99 */
        char series[3];
        series[0] = 'A' + (rand() % 26);
        series[1] = 'A' + (rand() % 26);
        series[2] = '\0';
        int number = (rand() % 9999) + 1; /* 0001 - 9999 */

        fprintf(fp, "%s%02d%s%04d\n", state, district, series, number);
    }

    fclose(fp);
    printf("Successfully generated %d registration numbers in %s\n", TOTAL_RECORDS, out_path);
    return 0;
}
