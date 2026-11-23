#include "cli.h"
#include "api.h"
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc >= 2 && strcmp(argv[1], "--api") == 0) {
        return api_main(argc, argv);
    }
    return cli_main(argc, argv);
}
