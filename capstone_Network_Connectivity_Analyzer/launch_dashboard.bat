@echo off
title Network Connectivity Analyzer
cd /d "%~dp0"
echo ===============================================================
echo         NETWORK CONNECTIVITY ANALYZER - DASHBOARD LAUNCHER
echo ===============================================================
if not exist analyzer.exe (
    echo [*] Compiling C engine...
    gcc -std=c11 -Wall -Wextra -O2 -Isrc -o analyzer.exe src/graph.c src/bfs.c src/dfs.c src/unionfind.c src/dijkstra.c src/faults.c src/cli.c src/api.c src/main.c -lm
)
echo [*] Opening dashboard at http://127.0.0.1:8080 ...
start "" "http://127.0.0.1:8080"
echo [*] Running server... Press Ctrl+C to stop.
python server.py --port 8080
pause
