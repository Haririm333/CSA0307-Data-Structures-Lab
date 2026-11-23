#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_banner(void) {
    printf("===============================================================\n");
    printf("        NETWORK CONNECTIVITY ANALYZER - DASHBOARD LAUNCHER     \n");
    printf("===============================================================\n");
}

int main(void) {
    print_banner();

    // 1. Set current working directory to directory of this executable
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH)) {
        char* last_slash = strrchr(exe_path, '\\');
        if (last_slash) {
            *last_slash = '\0';
            SetCurrentDirectoryA(exe_path);
        }
    }

    printf("[1/4] Checking working directory: %s\n", exe_path);

    // 2. Check if analyzer.exe exists, build if missing
    WIN32_FIND_DATAA find_data;
    HANDLE h_find = FindFirstFileA("analyzer.exe", &find_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        printf("[2/4] analyzer.exe missing, compiling C engine with gcc...\n");
        const char* build_cmd = "gcc -std=c11 -Wall -Wextra -O2 -Isrc -o analyzer.exe src/graph.c src/bfs.c src/dfs.c src/unionfind.c src/dijkstra.c src/faults.c src/cli.c src/api.c src/main.c -lm";
        int build_res = system(build_cmd);
        if (build_res != 0) {
            printf("[ERROR] Failed to compile C engine with gcc. Please verify gcc is in PATH.\n");
            system("pause");
            return 1;
        }
        printf("[2/4] Successfully built analyzer.exe!\n");
    } else {
        FindClose(h_find);
        printf("[2/4] Found C engine: analyzer.exe\n");
    }

    // 3. Start python server
    printf("[3/4] Starting HTTP server on http://127.0.0.1:8080 ...\n");
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd_line[] = "python server.py --port 8080";
    if (!CreateProcessA(NULL, cmd_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        // Fallback to python3
        char cmd_fallback[] = "python3 server.py --port 8080";
        if (!CreateProcessA(NULL, cmd_fallback, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            printf("[ERROR] Failed to launch python server. Ensure Python 3.8+ is installed.\n");
            system("pause");
            return 1;
        }
    }

    // Wait 800ms for server socket to initialize
    Sleep(800);

    // 4. Launch default browser to http://127.0.0.1:8080
    printf("[4/4] Opening dashboard in your default browser...\n");
    ShellExecuteA(NULL, "open", "http://127.0.0.1:8080", NULL, NULL, SW_SHOWNORMAL);

    printf("\n===============================================================\n");
    printf("  Dashboard is LIVE at http://127.0.0.1:8080                   \n");
    printf("  Keep this window open while using the web application.       \n");
    printf("  Press Ctrl+C to stop the server and exit.                    \n");
    printf("===============================================================\n\n");

    // Wait for the python server process to terminate
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
