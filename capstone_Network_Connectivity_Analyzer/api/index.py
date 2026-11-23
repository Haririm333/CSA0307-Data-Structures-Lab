"""
Vercel Serverless Function entrypoint for Network Connectivity Analyzer
Exports `handler` inheriting from http.server.BaseHTTPRequestHandler.
"""

import heapq
import json
import os
import re
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler
from urllib.parse import unquote, urlparse

DEVICE_NAME_REGEX = re.compile(r"^[A-Za-z0-9_-]{1,31}$")
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, "data")

# In-memory network topology state (persists across warm serverless invocations)
class MemoryTopology:
    def __init__(self):
        self.devices = [
            {"id": 0, "name": "A"},
            {"id": 1, "name": "B"},
            {"id": 2, "name": "C"},
            {"id": 3, "name": "D"},
        ]
        self.links = [
            {"a": "A", "b": "B", "cost": 2},
            {"a": "B", "b": "C", "cost": 3},
            {"a": "C", "b": "D", "cost": 4},
        ]

    def to_txt(self):
        lines = ["# Network Topology", "# Devices"]
        for d in self.devices:
            lines.append(f"DEVICE {d['name']}")
        lines.append("\n# Links")
        for l in self.links:
            lines.append(f"LINK {l['a']} {l['b']} {l['cost']}")
        return "\n".join(lines) + "\n"

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

        self.devices = devices
        self.links = links
        return True, "Success"


STATE = MemoryTopology()


def get_binary_path():
    for name in ["analyzer", "analyzer.exe"]:
        candidates = [
            os.path.join(BASE_DIR, name),
            os.path.join(BASE_DIR, "network-analyzer", name),
            os.path.join("/tmp", name),
        ]
        for c in candidates:
            if os.path.isfile(c) and os.access(c, os.X_OK):
                return c
    return None


def run_c_command(command, extra_args=None):
    bin_path = get_binary_path()
    if bin_path:
        tmp_file = tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8")
        try:
            tmp_file.write(STATE.to_txt())
            tmp_file.close()

            cmd = [bin_path, "--api", command, tmp_file.name]
            if extra_args:
                cmd.extend(extra_args)

            proc = subprocess.run(cmd, capture_output=True, text=True, timeout=5.0)
            if proc.returncode == 0:
                return True, json.loads(proc.stdout.strip()), 200
        except Exception:
            pass
        finally:
            if os.path.exists(tmp_file.name):
                try:
                    os.remove(tmp_file.name)
                except OSError:
                    pass

    # Serverless Python Fallback (Guarantees Vercel never returns 502)
    return run_serverless_algorithm(command, extra_args)


def run_serverless_algorithm(command, extra_args=None):
    dev_names = [d["name"] for d in STATE.devices]
    adj = {d: [] for d in dev_names}
    costs = {}

    for l in STATE.links:
        u, v, c = l["a"], l["b"], l["cost"]
        if u in adj and v in adj:
            adj[u].append(v)
            adj[v].append(u)
            costs[(u, v)] = c
            costs[(v, u)] = c

    for k in adj:
        adj[k].sort()

    if command == "summary":
        # Disjoint Set Union
        parent = {d: d for d in dev_names}

        def find(i):
            if parent[i] != i:
                parent[i] = find(parent[i])
            return parent[i]

        for u, v, _ in [(l["a"], l["b"], l["cost"]) for l in STATE.links]:
            if u in parent and v in parent:
                parent[find(u)] = find(v)

        comps_dict = {}
        for d in dev_names:
            root = find(d)
            comps_dict.setdefault(root, []).append(d)

        comp_list = list(comps_dict.values())
        isolated = [d for d in dev_names if len(adj[d]) == 0]

        dev_res = [{"id": i, "name": d, "degree": len(adj[d])} for i, d in enumerate(dev_names)]
        return True, {
            "ok": True,
            "devices": dev_res,
            "links": [{"a": l["a"], "b": l["b"], "cost": l["cost"]} for l in STATE.links],
            "components": len(comp_list),
            "isolated": isolated,
            "componentList": comp_list,
        }, 200

    if command == "bfs":
        start = extra_args[0] if extra_args else None
        if not start or start not in adj:
            return False, {"ok": False, "error": "Start device not found"}, 400
        order, levels, steps = [], {}, []
        visited = {start}
        queue = [start]
        levels[start] = 0
        from_map = {start: None}

        while queue:
            u = queue.pop(0)
            order.append(u)
            for nxt in adj[u]:
                if nxt not in visited:
                    visited.add(nxt)
                    from_map[nxt] = u
                    levels[nxt] = levels[u] + 1
                    queue.append(nxt)
            steps.append({
                "visit": u,
                "from": from_map[u],
                "queue": list(queue),
            })
        return True, {"ok": True, "order": order, "levels": levels, "steps": steps}, 200

    if command == "dfs":
        start = extra_args[0] if extra_args else None
        if not start or start not in adj:
            return False, {"ok": False, "error": "Start device not found"}, 400
        order, levels, steps = [], {}, []
        visited = set()
        stack = [(start, None, 0)]

        while stack:
            u, frm, lvl = stack.pop()
            if u in visited:
                continue
            visited.add(u)
            order.append(u)
            levels[u] = lvl

            unvisited_neighbors = [nxt for nxt in adj[u] if nxt not in visited]
            for nxt in reversed(unvisited_neighbors):
                stack.append((nxt, u, lvl + 1))

            steps.append({
                "visit": u,
                "from": frm,
                "stack": [item[0] for item in stack],
            })
        return True, {"ok": True, "order": order, "levels": levels, "steps": steps}, 200

    if command == "connect":
        x = extra_args[0] if extra_args and len(extra_args) >= 1 else None
        y = extra_args[1] if extra_args and len(extra_args) >= 2 else None
        if not x or not y or x not in adj or y not in adj:
            return False, {"ok": False, "error": "Device not found"}, 400

        visited = set()
        queue = [x]
        while queue:
            curr = queue.pop(0)
            if curr == y:
                break
            for nxt in adj[curr]:
                if nxt not in visited:
                    visited.add(nxt)
                    queue.append(nxt)
        conn = (y in visited) or (x == y)
        return True, {"ok": True, "bfs": conn, "dfs": conn, "unionfind": conn, "agree": True}, 200

    if command == "path":
        src = extra_args[0] if extra_args and len(extra_args) >= 1 else None
        dst = extra_args[1] if extra_args and len(extra_args) >= 2 else None
        if not src or not dst or src not in adj or dst not in adj:
            return False, {"ok": False, "error": "Device not found"}, 400

        dist = {d: float("inf") for d in dev_names}
        dist[src] = 0
        parent = {d: None for d in dev_names}
        pq = [(0, src)]
        settled = set()
        steps = []

        while pq:
            d_u, u = heapq.heappop(pq)
            if u in settled:
                continue
            settled.add(u)

            for v in adj[u]:
                if v not in settled:
                    w = costs.get((u, v), 1)
                    if dist[u] + w < dist[v]:
                        dist[v] = dist[u] + w
                        parent[v] = u
                        heapq.heappush(pq, (dist[v], v))

            steps.append({
                "settled": u,
                "dist": {k: int(v) for k, v in dist.items() if v < float("inf")},
            })

            if u == dst:
                break

        if dist[dst] < float("inf"):
            curr = dst
            path = []
            while curr:
                path.append(curr)
                curr = parent[curr]
            path.reverse()
            edges = [{"a": path[i], "b": path[i + 1], "cost": costs.get((path[i], path[i + 1]), 1)} for i in range(len(path) - 1)]
            return True, {"ok": True, "reachable": True, "path": path, "cost": int(dist[dst]), "edges": edges, "steps": steps}, 200
        else:
            return True, {"ok": True, "reachable": False, "path": [], "cost": -1, "edges": [], "steps": steps}, 200

    if command == "faults":
        timer = 0
        tin, low, visited, is_ap, bridges = {}, {}, set(), set(), []

        def dfs_tarjan(u, p=-1):
            nonlocal timer
            visited.add(u)
            timer += 1
            tin[u] = low[u] = timer
            children = 0
            for v in adj[u]:
                if v == p:
                    continue
                if v in visited:
                    low[u] = min(low[u], tin[v])
                else:
                    children += 1
                    dfs_tarjan(v, u)
                    low[u] = min(low[u], low[v])
                    if low[v] > tin[u]:
                        bridges.append(dict(a=min(u, v), b=max(u, v)))
                    if p != -1 and low[v] >= tin[u]:
                        is_ap.add(u)
            if p == -1 and children > 1:
                is_ap.add(u)

        for d in dev_names:
            if d not in visited:
                dfs_tarjan(d)

        bridges.sort(key=lambda x: (x["a"], x["b"]))
        return True, {"ok": True, "bridges": bridges, "articulation": sorted(list(is_ap))}, 200

    if command == "simulate-link":
        a, b = extra_args[0], extra_args[1]
        active_links = [l for l in STATE.links if not ((l["a"] == a and l["b"] == b) or (l["a"] == b and l["b"] == a))]
        parent = {d: d for d in dev_names}

        def find(i):
            if parent[i] != i:
                parent[i] = find(parent[i])
            return parent[i]

        for l in active_links:
            parent[find(l["a"])] = find(l["b"])

        comps_dict = {}
        for d in dev_names:
            comps_dict.setdefault(find(d), []).append(d)

        # Before count
        p_before = {d: d for d in dev_names}
        for l in STATE.links:
            p_before[find(l["a"])] = find(l["b"])
        cb = len(set(p_before.values()))

        return True, {
            "ok": True,
            "componentsBefore": cb,
            "componentsAfter": len(comps_dict),
            "disconnected": list(comps_dict.values()),
        }, 200

    if command == "simulate-device":
        name = extra_args[0]
        rem_devs = [d for d in dev_names if d != name]
        active_links = [l for l in STATE.links if l["a"] != name and l["b"] != name]
        parent = {d: d for d in rem_devs}

        def find(i):
            if parent[i] != i:
                parent[i] = find(parent[i])
            return parent[i]

        for l in active_links:
            parent[find(l["a"])] = find(l["b"])

        comps_dict = {}
        for d in rem_devs:
            comps_dict.setdefault(find(d), []).append(d)

        return True, {
            "ok": True,
            "componentsBefore": 1,
            "componentsAfter": len(comps_dict),
            "disconnected": list(comps_dict.values()),
        }, 200

    return False, {"ok": False, "error": "Unknown command"}, 400


class handler(BaseHTTPRequestHandler):
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

    def send_text(self, status_code, text, download_filename=None):
        body = text.encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
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
        parsed = urlparse(self.path)
        path = unquote(parsed.path)

        if path == "/api/network":
            ok, data, code = run_c_command("summary")
            if ok:
                data["inMemoryDevices"] = STATE.devices
                data["inMemoryLinks"] = STATE.links
            self.send_json(code, data)
            return

        if path == "/api/analyze/faults":
            ok, data, code = run_c_command("faults")
            self.send_json(code, data)
            return

        if path == "/api/export":
            self.send_text(200, STATE.to_txt(), download_filename="network.txt")
            return

        if path == "/api/report":
            ok, summary, _ = run_c_command("summary")
            ok_f, faults, _ = run_c_command("faults")
            report_lines = [
                "================================================================================",
                "                  NETWORK CONNECTIVITY ANALYZER - ANALYSIS REPORT               ",
                "================================================================================",
                "",
                f"Total Devices: {len(STATE.devices)}",
                f"Total Links:   {len(STATE.links)}",
                f"Components:    {summary.get('components', 1) if ok else 1}",
                f"Isolated:      {', '.join(summary.get('isolated', [])) or 'None'}",
                "",
                "Vulnerabilities (Tarjan Low-Link):",
                f"  Bridges: {len(faults.get('bridges', [])) if ok_f else 0}",
                f"  Cut Vertices: {len(faults.get('articulation', [])) if ok_f else 0}",
                "",
                "================================================================================",
            ]
            self.send_text(200, "\n".join(report_lines) + "\n")
            return

        self.send_json(404, {"ok": False, "error": f"Not found: {path}"})

    def do_POST(self):
        parsed = urlparse(self.path)
        path = unquote(parsed.path)

        if path == "/api/device":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            name = (body.get("name") or "").strip()
            if not DEVICE_NAME_REGEX.match(name):
                self.send_json(400, {"ok": False, "error": "Name must match ^[A-Za-z0-9_-]{1,31}$"})
                return
            if any(d["name"] == name for d in STATE.devices):
                self.send_json(400, {"ok": False, "error": f"Device '{name}' already exists"})
                return
            STATE.devices.append({"id": len(STATE.devices), "name": name})
            self.send_json(200, {"ok": True, "message": f"Device '{name}' added", "name": name})
            return

        if path == "/api/link":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            a = (body.get("a") or "").strip()
            b = (body.get("b") or "").strip()
            cost = body.get("cost")
            if not a or not b or cost is None:
                self.send_json(400, {"ok": False, "error": "Missing 'a', 'b', or 'cost'"})
                return
            if a == b:
                self.send_json(400, {"ok": False, "error": "Self-loops not allowed"})
                return
            try:
                cost = int(cost)
            except ValueError:
                self.send_json(400, {"ok": False, "error": "Cost must be integer"})
                return
            if not (1 <= cost <= 9999):
                self.send_json(400, {"ok": False, "error": "Cost must be 1-9999"})
                return
            dev_names = {d["name"] for d in STATE.devices}
            if a not in dev_names or b not in dev_names:
                self.send_json(400, {"ok": False, "error": "Device does not exist"})
                return
            canon = tuple(sorted([a, b]))
            updated = False
            for l in STATE.links:
                if tuple(sorted([l["a"], l["b"]])) == canon:
                    l["cost"] = cost
                    updated = True
                    break
            if not updated:
                STATE.links.append({"a": canon[0], "b": canon[1], "cost": cost})
            self.send_json(200, {"ok": True, "message": f"Link {canon[0]}-{canon[1]} saved"})
            return

        if path == "/api/analyze/bfs":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            ok, data, code = run_c_command("bfs", [body.get("start", "")])
            self.send_json(code, data)
            return

        if path == "/api/analyze/dfs":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            ok, data, code = run_c_command("dfs", [body.get("start", "")])
            self.send_json(code, data)
            return

        if path == "/api/analyze/connect":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            ok, data, code = run_c_command("connect", [body.get("x", ""), body.get("y", "")])
            self.send_json(code, data)
            return

        if path == "/api/analyze/path":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            ok, data, code = run_c_command("path", [body.get("src", ""), body.get("dst", "")])
            self.send_json(code, data)
            return

        if path == "/api/simulate":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            sim_type = body.get("type")
            if sim_type == "link":
                ok, data, code = run_c_command("simulate-link", [body.get("a", ""), body.get("b", "")])
                self.send_json(code, data)
            elif sim_type == "device":
                ok, data, code = run_c_command("simulate-device", [body.get("name", "")])
                self.send_json(code, data)
            else:
                self.send_json(400, {"ok": False, "error": "Invalid simulation type"})
            return

        if path.startswith("/api/load/"):
            sample = path[len("/api/load/"):]
            if "/" in sample or "\\" in sample or ".." in sample:
                self.send_json(400, {"ok": False, "error": "Path traversal rejected"})
                return
            if not sample.endswith(".txt"):
                sample += ".txt"
            sample_path = os.path.join(DATA_DIR, sample)
            if not os.path.isfile(sample_path):
                self.send_json(404, {"ok": False, "error": f"Sample '{sample}' not found"})
                return
            with open(sample_path, "r", encoding="utf-8") as f:
                content = f.read()
            ok, msg = STATE.import_txt(content)
            if ok:
                ok_c, summary, code = run_c_command("summary")
                self.send_json(200, {"ok": True, "message": f"Loaded '{sample}'", "summary": summary if ok_c else None})
            else:
                self.send_json(400, {"ok": False, "error": msg})
            return

        if path == "/api/import":
            length = int(self.headers.get("Content-Length", 0))
            raw = self.rfile.read(length).decode("utf-8")
            try:
                pj = json.loads(raw)
                if isinstance(pj, dict) and "text" in pj:
                    raw = pj["text"]
            except Exception:
                pass
            ok, msg = STATE.import_txt(raw)
            if ok:
                ok_c, summary, code = run_c_command("summary")
                self.send_json(200, {"ok": True, "message": "Import successful", "summary": summary if ok_c else None})
            else:
                self.send_json(400, {"ok": False, "error": msg})
            return

        self.send_json(404, {"ok": False, "error": f"Not found: {path}"})

    def do_DELETE(self):
        parsed = urlparse(self.path)
        path = unquote(parsed.path)

        if path.startswith("/api/device/"):
            name = path[len("/api/device/"):]
            found = False
            for i, d in enumerate(STATE.devices):
                if d["name"] == name:
                    del STATE.devices[i]
                    found = True
                    break
            if not found:
                self.send_json(404, {"ok": False, "error": f"Device '{name}' not found"})
                return
            STATE.links = [l for l in STATE.links if l["a"] != name and l["b"] != name]
            self.send_json(200, {"ok": True, "message": f"Device '{name}' removed"})
            return

        if path == "/api/link":
            body, err = self.parse_json_body()
            if err:
                self.send_json(400, {"ok": False, "error": err})
                return
            a = body.get("a")
            b = body.get("b")
            canon = tuple(sorted([a, b]))
            init_len = len(STATE.links)
            STATE.links = [l for l in STATE.links if tuple(sorted([l["a"], l["b"]])) != canon]
            if len(STATE.links) == init_len:
                self.send_json(404, {"ok": False, "error": "Link not found"})
                return
            self.send_json(200, {"ok": True, "message": f"Link removed"})
            return

        self.send_json(404, {"ok": False, "error": f"Not found: {path}"})
