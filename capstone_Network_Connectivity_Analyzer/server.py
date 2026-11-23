#!/usr/bin/env python3
"""
Network Connectivity Analyzer - HTTP Server
Standard library only: http.server, json, subprocess, tempfile, threading, os, sys, re, argparse.
"""

import argparse
import http.server
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
from urllib.parse import urlparse, unquote

DEVICE_NAME_REGEX = re.compile(r"^[A-Za-z0-9_-]{1,31}$")
PORT = 8080
HOST = "127.0.0.1"

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
WEB_DIR = os.path.join(BASE_DIR, "web")
DATA_DIR = os.path.join(BASE_DIR, "data")


def get_binary_path():
    exe_name = "analyzer.exe" if os.name == "nt" else "analyzer"
    # Check in BASE_DIR or network-analyzer subfolder
    candidates = [
        os.path.join(BASE_DIR, exe_name),
        os.path.join(BASE_DIR, "network-analyzer", exe_name),
    ]
    for c in candidates:
        if os.path.isfile(c) and os.access(c, os.X_OK):
            return c
    return None


def ensure_binary():
    bin_path = get_binary_path()
    if bin_path:
        return bin_path

    print("[Server] Analyzer binary missing, attempting automatic build...")
    # Try make / mingw32-make
    make_cmd = None
    if shutil.which("mingw32-make"):
        make_cmd = "mingw32-make"
    elif shutil.which("make"):
        make_cmd = "make"

    if make_cmd:
        try:
            res = subprocess.run([make_cmd], cwd=BASE_DIR, timeout=30, capture_output=True, text=True)
            if res.returncode == 0:
                bin_path = get_binary_path()
                if bin_path:
                    print(f"[Server] Successfully built binary with {make_cmd}: {bin_path}")
                    return bin_path
        except Exception as e:
            print(f"[Server] Make failed: {e}")

    # Fallback to direct gcc compilation
    if shutil.which("gcc"):
        exe_name = "analyzer.exe" if os.name == "nt" else "analyzer"
        out_path = os.path.join(BASE_DIR, exe_name)
        src_files = [
            os.path.join(BASE_DIR, "src", f)
            for f in ["graph.c", "bfs.c", "dfs.c", "unionfind.c", "dijkstra.c", "faults.c", "cli.c", "api.c", "main.c"]
        ]
        cmd = ["gcc", "-std=c11", "-Wall", "-Wextra", "-O2", f"-I{os.path.join(BASE_DIR, 'src')}", "-o", out_path] + src_files + ["-lm"]
        try:
            res = subprocess.run(cmd, timeout=30, capture_output=True, text=True)
            if res.returncode == 0 and os.path.isfile(out_path):
                print(f"[Server] Successfully compiled binary with gcc: {out_path}")
                return out_path
        except Exception as e:
            print(f"[Server] GCC build failed: {e}")

    raise RuntimeError("Could not find or build the analyzer binary. Please run make or gcc first.")


class NetworkState:
    def __init__(self):
        self.lock = threading.Lock()
        self.devices = []
        self.links = []
        # Preload sample_small if available
        sample_path = os.path.join(DATA_DIR, "sample_small.txt")
        if os.path.isfile(sample_path):
            self.load_from_txt_file(sample_path)

    def load_from_txt_file(self, filepath):
        with open(filepath, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
        return self.import_txt(content)

    def import_txt(self, content):
        devices = []
        dev_set = set()
        links = []
        link_set = set()

        for line in content.splitlines():
            line = line.strip()
            if not line or line.startswith("#") or line.startswith(";"):
                continue

            parts = line.split()
            if parts[0] == "DEVICE" and len(parts) >= 2:
                name = parts[1]
                if not DEVICE_NAME_REGEX.match(name):
                    return False, f"Invalid device name: {name}"
                if name not in dev_set:
                    dev_set.add(name)
                    devices.append({"id": len(devices), "name": name})
            elif parts[0] == "LINK" and len(parts) >= 3:
                a, b = parts[1], parts[2]
                cost = 1
                if len(parts) >= 4:
                    try:
                        cost = int(parts[3])
                    except ValueError:
                        return False, f"Invalid link cost: {parts[3]}"
                if not (1 <= cost <= 9999):
                    return False, f"Link cost must be between 1 and 9999 (got {cost})"
                if not DEVICE_NAME_REGEX.match(a) or not DEVICE_NAME_REGEX.match(b):
                    return False, f"Invalid device name in link: {a} - {b}"
                if a == b:
                    return False, f"Self-loops are not allowed: {a}"
                if a not in dev_set:
                    dev_set.add(a)
                    devices.append({"id": len(devices), "name": a})
                if b not in dev_set:
                    dev_set.add(b)
                    devices.append({"id": len(devices), "name": b})
                canon = tuple(sorted([a, b]))
                if canon not in link_set:
                    link_set.add(canon)
                    links.append({"a": canon[0], "b": canon[1], "cost": cost})
            elif len(parts) == 1:
                name = parts[0]
                if not DEVICE_NAME_REGEX.match(name):
                    return False, f"Invalid device name: {name}"
                if name not in dev_set:
                    dev_set.add(name)
                    devices.append({"id": len(devices), "name": name})
            elif len(parts) >= 2:
                a, b = parts[0], parts[1]
                cost = 1
                if len(parts) >= 3:
                    try:
                        cost = int(parts[2])
                    except ValueError:
                        return False, f"Invalid link cost: {parts[2]}"
                if not (1 <= cost <= 9999):
                    return False, f"Link cost must be between 1 and 9999 (got {cost})"
                if not DEVICE_NAME_REGEX.match(a) or not DEVICE_NAME_REGEX.match(b):
                    return False, f"Invalid device name in link: {a} - {b}"
                if a == b:
                    return False, f"Self-loops are not allowed: {a}"
                if a not in dev_set:
                    dev_set.add(a)
                    devices.append({"id": len(devices), "name": a})
                if b not in dev_set:
                    dev_set.add(b)
                    devices.append({"id": len(devices), "name": b})
                canon = tuple(sorted([a, b]))
                if canon not in link_set:
                    link_set.add(canon)
                    links.append({"a": canon[0], "b": canon[1], "cost": cost})

        with self.lock:
            self.devices = devices
            self.links = links
        return True, "Success"

    def to_txt(self):
        with self.lock:
            lines = ["# Network Topology Configuration", "# Devices"]
            for d in self.devices:
                lines.append(f"DEVICE {d['name']}")
            lines.append("\n# Links")
            for l in self.links:
                lines.append(f"LINK {l['a']} {l['b']} {l['cost']}")
            return "\n".join(lines) + "\n"

    def get_snapshot(self):
        with self.lock:
            return {
                "devices": [dict(d) for d in self.devices],
                "links": [dict(l) for l in self.links],
            }


STATE = NetworkState()


def run_c_command(command, extra_args=None, custom_txt=None):
    bin_path = ensure_binary()
    txt_data = custom_txt if custom_txt is not None else STATE.to_txt()

    tmp_file = tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8")
    try:
        tmp_file.write(txt_data)
        tmp_file.close()

        cmd = [bin_path, "--api", command, tmp_file.name]
        if extra_args:
            cmd.extend(extra_args)

        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5.0,
            shell=False,
        )

        out = proc.stdout.strip()
        try:
            parsed = json.loads(out)
            return proc.returncode == 0, parsed, proc.returncode
        except json.JSONDecodeError:
            return False, {"ok": False, "error": f"Invalid JSON from C binary: {out or proc.stderr}"}, 500
    except subprocess.TimeoutExpired:
        return False, {"ok": False, "error": "C analyzer process timed out (5s limit)"}, 504
    except Exception as e:
        return False, {"ok": False, "error": f"Subprocess execution failed: {str(e)}"}, 500
    finally:
        if os.path.exists(tmp_file.name):
            try:
                os.remove(tmp_file.name)
            except OSError:
                pass


class NetworkApiHandler(http.server.BaseHTTPRequestHandler):
    server_version = "NetworkAnalyzer/1.0"

    def send_json(self, status_code, data):
        body = json.dumps(data, indent=2).encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
        self.wfile.write(body)

    def send_text(self, status_code, text, content_type="text/plain; charset=utf-8", download_filename=None):
        body = text.encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        if download_filename:
            self.send_header("Content-Disposition", f'attachment; filename="{download_filename}"')
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def parse_json_body(self):
        try:
            length = int(self.headers.get("Content-Length", 0))
            if length == 0:
                return None, "Empty request body"
            raw = self.rfile.read(length).decode("utf-8")
            return json.loads(raw), None
        except Exception as e:
            return None, f"Malformed JSON: {str(e)}"

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        parsed_url = urlparse(self.path)
        path = unquote(parsed_url.path)

        if path == "/":
            self.serve_static("index.html")
            return

        if not path.startswith("/api/"):
            # Static file serving from web/
            rel_path = path.lstrip("/")
            self.serve_static(rel_path)
            return

        # API GET endpoints
        if path == "/api/network":
            # Current network + summary
            ok, summary, code = run_c_command("summary")
            snap = STATE.get_snapshot()
            if ok:
                summary["inMemoryDevices"] = snap["devices"]
                summary["inMemoryLinks"] = snap["links"]
                self.send_json(200, summary)
            else:
                self.send_json(400 if code == 1 else 500, summary)
            return

        if path == "/api/analyze/faults":
            ok, data, code = run_c_command("faults")
            self.send_json(200 if ok else 400, data)
            return

        if path == "/api/export":
            txt_content = STATE.to_txt()
            self.send_text(200, txt_content, download_filename="network.txt")
            return

        if path == "/api/report":
            report_text = self.generate_plain_text_report()
            self.send_text(200, report_text)
            return

        self.send_json(404, {"ok": False, "error": f"Endpoint not found: {path}"})

    def do_POST(self):
        parsed_url = urlparse(self.path)
        path = unquote(parsed_url.path)

        if path == "/api/device":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            name = body.get("name")
            if not name or not isinstance(name, str):
                self.send_json(400, {"ok": False, "error": "Missing or invalid 'name' field"})
                return
            name = name.strip()
            if not DEVICE_NAME_REGEX.match(name):
                self.send_json(400, {"ok": False, "error": "Device name must match ^[A-Za-z0-9_-]{1,31}$"})
                return
            with STATE.lock:
                for d in STATE.devices:
                    if d["name"] == name:
                        self.send_json(400, {"ok": False, "error": f"Device '{name}' already exists"})
                        return
                new_id = len(STATE.devices)
                STATE.devices.append({"id": new_id, "name": name})
            self.send_json(200, {"ok": True, "message": f"Device '{name}' added", "name": name})
            return

        if path == "/api/link":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            a = body.get("a")
            b = body.get("b")
            cost = body.get("cost")
            if not a or not b or cost is None:
                self.send_json(400, {"ok": False, "error": "Missing 'a', 'b', or 'cost' field"})
                return
            if not isinstance(a, str) or not isinstance(b, str):
                self.send_json(400, {"ok": False, "error": "Device names 'a' and 'b' must be strings"})
                return
            a, b = a.strip(), b.strip()
            if not DEVICE_NAME_REGEX.match(a) or not DEVICE_NAME_REGEX.match(b):
                self.send_json(400, {"ok": False, "error": "Device names must match ^[A-Za-z0-9_-]{1,31}$"})
                return
            if a == b:
                self.send_json(400, {"ok": False, "error": "Self-loops are not allowed"})
                return
            try:
                cost = int(cost)
            except (ValueError, TypeError):
                self.send_json(400, {"ok": False, "error": "Cost must be an integer"})
                return
            if not (1 <= cost <= 9999):
                self.send_json(400, {"ok": False, "error": "Cost must be an integer between 1 and 9999"})
                return

            with STATE.lock:
                dev_names = {d["name"] for d in STATE.devices}
                if a not in dev_names:
                    self.send_json(400, {"ok": False, "error": f"Device '{a}' does not exist"})
                    return
                if b not in dev_names:
                    self.send_json(400, {"ok": False, "error": f"Device '{b}' does not exist"})
                    return
                canon = tuple(sorted([a, b]))
                # Check if link exists to update or add
                updated = False
                for l in STATE.links:
                    if tuple(sorted([l["a"], l["b"]])) == canon:
                        l["cost"] = cost
                        updated = True
                        break
                if not updated:
                    STATE.links.append({"a": canon[0], "b": canon[1], "cost": cost})
            action = "updated" if updated else "added"
            self.send_json(200, {"ok": True, "message": f"Link {canon[0]}-{canon[1]} {action}", "link": {"a": canon[0], "b": canon[1], "cost": cost}})
            return

        if path == "/api/analyze/bfs":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            start = body.get("start")
            if not start or not isinstance(start, str) or not DEVICE_NAME_REGEX.match(start.strip()):
                self.send_json(400, {"ok": False, "error": "Valid 'start' device name required"})
                return
            ok, data, code = run_c_command("bfs", [start.strip()])
            self.send_json(200 if ok else 400, data)
            return

        if path == "/api/analyze/dfs":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            start = body.get("start")
            if not start or not isinstance(start, str) or not DEVICE_NAME_REGEX.match(start.strip()):
                self.send_json(400, {"ok": False, "error": "Valid 'start' device name required"})
                return
            ok, data, code = run_c_command("dfs", [start.strip()])
            self.send_json(200 if ok else 400, data)
            return

        if path == "/api/analyze/connect":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            x = body.get("x")
            y = body.get("y")
            if not x or not y or not isinstance(x, str) or not isinstance(y, str):
                self.send_json(400, {"ok": False, "error": "Devices 'x' and 'y' required"})
                return
            x, y = x.strip(), y.strip()
            if not DEVICE_NAME_REGEX.match(x) or not DEVICE_NAME_REGEX.match(y):
                self.send_json(400, {"ok": False, "error": "Device names must match ^[A-Za-z0-9_-]{1,31}$"})
                return
            ok, data, code = run_c_command("connect", [x, y])
            self.send_json(200 if ok else 400, data)
            return

        if path == "/api/analyze/path":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            src = body.get("src")
            dst = body.get("dst")
            if not src or not dst or not isinstance(src, str) or not isinstance(dst, str):
                self.send_json(400, {"ok": False, "error": "Devices 'src' and 'dst' required"})
                return
            src, dst = src.strip(), dst.strip()
            if not DEVICE_NAME_REGEX.match(src) or not DEVICE_NAME_REGEX.match(dst):
                self.send_json(400, {"ok": False, "error": "Device names must match ^[A-Za-z0-9_-]{1,31}$"})
                return
            ok, data, code = run_c_command("path", [src, dst])
            self.send_json(200 if ok else 400, data)
            return

        if path == "/api/simulate":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            sim_type = body.get("type")
            if sim_type == "link":
                a = body.get("a")
                b = body.get("b")
                if not a or not b or not isinstance(a, str) or not isinstance(b, str):
                    self.send_json(400, {"ok": False, "error": "Simulation link requires 'a' and 'b'"})
                    return
                ok, data, code = run_c_command("simulate-link", [a.strip(), b.strip()])
                self.send_json(200 if ok else 400, data)
                return
            elif sim_type == "device":
                name = body.get("name")
                if not name or not isinstance(name, str):
                    self.send_json(400, {"ok": False, "error": "Simulation device requires 'name'"})
                    return
                ok, data, code = run_c_command("simulate-device", [name.strip()])
                self.send_json(200 if ok else 400, data)
                return
            else:
                self.send_json(400, {"ok": False, "error": "Invalid simulation type: must be 'link' or 'device'"})
                return

        if path.startswith("/api/load/"):
            sample = path[len("/api/load/"):]
            # Whitelist filenames to prevent path traversal
            if "/" in sample or "\\" in sample or ".." in sample:
                self.send_json(400, {"ok": False, "error": "Path traversal attempt rejected"})
                return
            if not sample.endswith(".txt"):
                sample += ".txt"
            sample_path = os.path.join(DATA_DIR, sample)
            if not os.path.isfile(sample_path):
                self.send_json(404, {"ok": False, "error": f"Sample file '{sample}' not found"})
                return

            ok, msg = STATE.load_from_txt_file(sample_path)
            if ok:
                ok_c, summary, code = run_c_command("summary")
                self.send_json(200, {"ok": True, "message": f"Loaded '{sample}'", "summary": summary if ok_c else None})
            else:
                self.send_json(400, {"ok": False, "error": msg})
            return

        if path == "/api/import":
            length = int(self.headers.get("Content-Length", 0))
            if length == 0:
                self.send_json(400, {"ok": False, "error": "Empty import payload"})
                return
            raw_text = self.rfile.read(length).decode("utf-8")
            # If payload is JSON {"text": "..."}
            try:
                parsed_json = json.loads(raw_text)
                if isinstance(parsed_json, dict) and "text" in parsed_json:
                    raw_text = parsed_json["text"]
            except Exception:
                pass

            ok, msg = STATE.import_txt(raw_text)
            if ok:
                ok_c, summary, code = run_c_command("summary")
                self.send_json(200, {"ok": True, "message": "Import successful", "summary": summary if ok_c else None})
            else:
                self.send_json(400, {"ok": False, "error": msg})
            return

        self.send_json(404, {"ok": False, "error": f"Endpoint not found: {path}"})

    def do_DELETE(self):
        parsed_url = urlparse(self.path)
        path = unquote(parsed_url.path)

        if path.startswith("/api/device/"):
            name = path[len("/api/device/"):]
            if not name or not DEVICE_NAME_REGEX.match(name):
                self.send_json(400, {"ok": False, "error": "Invalid device name"})
                return
            with STATE.lock:
                found = False
                for i, d in enumerate(STATE.devices):
                    if d["name"] == name:
                        found = True
                        del STATE.devices[i]
                        break
                if not found:
                    self.send_json(404, {"ok": False, "error": f"Device '{name}' not found"})
                    return
                # Remove incident links
                STATE.links = [l for l in STATE.links if l["a"] != name and l["b"] != name]
                # Re-index remaining devices
                for idx, d in enumerate(STATE.devices):
                    d["id"] = idx
            self.send_json(200, {"ok": True, "message": f"Device '{name}' and its links removed"})
            return

        if path == "/api/link":
            # DELETE /api/link with {"a": "...", "b": "..."} in body
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            a = body.get("a")
            b = body.get("b")
            if not a or not b or not isinstance(a, str) or not isinstance(b, str):
                self.send_json(400, {"ok": False, "error": "Devices 'a' and 'b' required"})
                return
            a, b = a.strip(), b.strip()
            canon = tuple(sorted([a, b]))
            with STATE.lock:
                init_len = len(STATE.links)
                STATE.links = [l for l in STATE.links if tuple(sorted([l["a"], l["b"]])) != canon]
                if len(STATE.links) == init_len:
                    self.send_json(404, {"ok": False, "error": f"Link {canon[0]}-{canon[1]} not found"})
                    return
            self.send_json(200, {"ok": True, "message": f"Link {canon[0]}-{canon[1]} removed"})
            return

        self.send_json(404, {"ok": False, "error": f"Endpoint not found: {path}"})

    def serve_static(self, rel_path):
        clean_rel = os.path.normpath(rel_path).lstrip("\\/")
        file_path = os.path.join(WEB_DIR, clean_rel)

        # Check path traversal
        if not os.path.abspath(file_path).startswith(os.path.abspath(WEB_DIR)):
            self.send_json(403, {"ok": False, "error": "Forbidden"})
            return

        if not os.path.isfile(file_path):
            self.send_json(404, {"ok": False, "error": f"File '{clean_rel}' not found"})
            return

        mime_types = {
            ".html": "text/html; charset=utf-8",
            ".css": "text/css; charset=utf-8",
            ".js": "application/javascript; charset=utf-8",
            ".json": "application/json; charset=utf-8",
            ".svg": "image/svg+xml",
            ".png": "image/png",
            ".ico": "image/x-icon",
            ".txt": "text/plain; charset=utf-8",
            ".woff2": "font/woff2",
            ".woff": "font/woff",
            ".ttf": "font/ttf",
        }
        ext = os.path.splitext(file_path)[1].lower()
        content_type = mime_types.get(ext, "application/octet-stream")

        with open(file_path, "rb") as f:
            content = f.read()

        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)

    def generate_plain_text_report(self):
        lines = []
        lines.append("================================================================================")
        lines.append("                  NETWORK CONNECTIVITY ANALYZER - ANALYSIS REPORT               ")
        lines.append("================================================================================")
        lines.append("")

        snap = STATE.get_snapshot()
        lines.append(f"Topology Summary:")
        lines.append(f"  Total Devices: {len(snap['devices'])}")
        lines.append(f"  Total Links:   {len(snap['links'])}")
        lines.append("")

        # Summary analysis via C
        ok, summary, _ = run_c_command("summary")
        if ok:
            lines.append("Device Degrees:")
            for d in summary.get("devices", []):
                lines.append(f"  - Device '{d['name']}': degree {d['degree']}")
            lines.append("")
            lines.append(f"Connected Components: {summary.get('components', 0)}")
            for idx, comp in enumerate(summary.get("componentList", []), 1):
                lines.append(f"  Component {idx} ({len(comp)} devices): {', '.join(comp)}")
            lines.append("")
            isolated = summary.get("isolated", [])
            if isolated:
                lines.append(f"Isolated Devices ({len(isolated)}): {', '.join(isolated)}")
            else:
                lines.append("Isolated Devices: None")
            lines.append("")

        # Faults analysis via C
        ok_f, faults, _ = run_c_command("faults")
        if ok_f:
            lines.append("Vulnerability & Fault Detection (Weak Points):")
            bridges = faults.get("bridges", [])
            lines.append(f"  Critical Bridges ({len(bridges)}):")
            if bridges:
                for b in bridges:
                    lines.append(f"    * Link [{b['a']} <--> {b['b']}] is a bridge (failure partitions the network)")
            else:
                lines.append("    * None (all edges have redundant connectivity)")

            ap = faults.get("articulation", [])
            lines.append(f"  Critical Articulation Points / Cut Vertices ({len(ap)}):")
            if ap:
                for a in ap:
                    lines.append(f"    * Device '{a}' is a single point of failure (cut vertex)")
            else:
                lines.append("    * None (biconnected network)")
            lines.append("")

        lines.append("Network Health Assessment:")
        if ok and summary.get("components", 1) > 1:
            lines.append("  STATUS: ALERT - The network is currently fragmented into multiple disconnected components.")
        elif ok_f and (faults.get("bridges") or faults.get("articulation")):
            lines.append("  STATUS: WARNING - The network is connected but contains single points of failure (bridges/cut vertices).")
        else:
            lines.append("  STATUS: OPTIMAL - The network is fully connected with redundant paths.")

        lines.append("")
        lines.append("================================================================================")
        lines.append("                        END OF REPORT - SIMATS CAPSTONE                         ")
        lines.append("================================================================================")
        return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description="Network Connectivity Analyzer Web Server")
    parser.add_argument("--port", type=int, default=8080, help="Port to bind (default: 8080)")
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Host to bind (default: 127.0.0.1)")
    args = parser.parse_args()

    # Pre-build C binary if needed
    ensure_binary()

    server_address = (args.host, args.port)
    httpd = http.server.ThreadingHTTPServer(server_address, NetworkApiHandler)
    print(f"[Server] Running at http://{args.host}:{args.port}")
    print(f"[Server] Press Ctrl+C to stop.")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[Server] Shutting down...")
    finally:
        httpd.server_close()


if __name__ == "__main__":
    main()
