#!/usr/bin/env python3
"""
Unit tests for Network Connectivity Analyzer HTTP Server & C API
Standard library only: unittest, urllib.request, json, threading, socket.
"""

import json
import os
import socket
import sys
import threading
import time
import unittest
import urllib.error
import urllib.request
from http.server import ThreadingHTTPServer

# Add parent directory to path to import server module
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if BASE_DIR not in sys.path:
    sys.path.insert(0, BASE_DIR)

from server import NetworkApiHandler, STATE, ensure_binary, DATA_DIR


def get_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('127.0.0.1', 0))
        return s.getsockname()[1]


class TestNetworkApi(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        ensure_binary()
        cls.port = get_free_port()
        cls.server_address = ('127.0.0.1', cls.port)
        cls.httpd = ThreadingHTTPServer(cls.server_address, NetworkApiHandler)
        cls.server_thread = threading.Thread(target=cls.httpd.serve_forever, daemon=True)
        cls.server_thread.start()
        time.sleep(0.1)

    @classmethod
    def tearDownClass(cls):
        cls.httpd.shutdown()
        cls.httpd.server_close()

    def url(self, path):
        return f"http://127.0.0.1:{self.port}{path}"

    def request(self, method, path, data=None, headers=None):
        if headers is None:
            headers = {}
        body = None
        if data is not None:
            if isinstance(data, (dict, list)):
                body = json.dumps(data).encode("utf-8")
                headers["Content-Type"] = "application/json"
            elif isinstance(data, str):
                body = data.encode("utf-8")
                if "Content-Type" not in headers:
                    headers["Content-Type"] = "text/plain"

        req = urllib.request.Request(self.url(path), data=body, headers=headers, method=method)
        try:
            with urllib.request.urlopen(req, timeout=5) as response:
                resp_body = response.read().decode("utf-8")
                return response.status, response.headers, resp_body
        except urllib.error.HTTPError as e:
            resp_body = e.read().decode("utf-8")
            return e.code, e.headers, resp_body

    def test_01_static_root(self):
        status, headers, body = self.request("GET", "/")
        self.assertEqual(status, 200)
        # Even if index.html is being prepared, check response
        self.assertIn("text/html", headers.get("Content-Type", ""))

    def test_02_sample_small_load_and_bridges(self):
        status, _, body = self.request("POST", "/api/load/sample_small.txt")
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])

        # Check faults on sample_small (must have 3 bridges)
        status, _, body = self.request("GET", "/api/analyze/faults")
        self.assertEqual(status, 200)
        faults = json.loads(body)
        self.assertTrue(faults["ok"])
        self.assertEqual(len(faults["bridges"]), 3)
        self.assertIn("B", faults["articulation"])
        self.assertIn("C", faults["articulation"])

    def test_03_sample_dijkstra(self):
        status, _, body = self.request("POST", "/api/load/sample_dijkstra.txt")
        self.assertEqual(status, 200)

        # Check path A -> D gives A -> B -> D with cost 5
        status, _, body = self.request("POST", "/api/analyze/path", {"src": "A", "dst": "D"})
        self.assertEqual(status, 200)
        path_res = json.loads(body)
        self.assertTrue(path_res["ok"])
        self.assertTrue(path_res["reachable"])
        self.assertEqual(path_res["cost"], 5)
        self.assertEqual(path_res["path"], ["A", "B", "D"])
        self.assertEqual(len(path_res["edges"]), 2)

    def test_04_sample_disconnected(self):
        status, _, body = self.request("POST", "/api/load/sample_disconnected.txt")
        self.assertEqual(status, 200)

        status, _, body = self.request("GET", "/api/network")
        self.assertEqual(status, 200)
        net_res = json.loads(body)
        self.assertTrue(net_res["ok"])
        self.assertGreaterEqual(net_res["components"], 2)
        self.assertIn("F", net_res["isolated"])

    def test_05_bfs_and_dfs(self):
        self.request("POST", "/api/load/sample_small.txt")

        # BFS
        status, _, body = self.request("POST", "/api/analyze/bfs", {"start": "A"})
        self.assertEqual(status, 200)
        bfs = json.loads(body)
        self.assertTrue(bfs["ok"])
        self.assertEqual(bfs["order"], ["A", "B", "C", "D"])
        self.assertEqual(bfs["levels"]["A"], 0)
        self.assertEqual(bfs["levels"]["B"], 1)
        self.assertIn("queue", bfs["steps"][0])

        # DFS
        status, _, body = self.request("POST", "/api/analyze/dfs", {"start": "A"})
        self.assertEqual(status, 200)
        dfs = json.loads(body)
        self.assertTrue(dfs["ok"])
        self.assertEqual(dfs["order"], ["A", "B", "C", "D"])
        self.assertIn("stack", dfs["steps"][0])

    def test_06_connect_endpoint(self):
        self.request("POST", "/api/load/sample_small.txt")
        status, _, body = self.request("POST", "/api/analyze/connect", {"x": "A", "y": "D"})
        self.assertEqual(status, 200)
        conn = json.loads(body)
        self.assertTrue(conn["ok"])
        self.assertTrue(conn["bfs"])
        self.assertTrue(conn["dfs"])
        self.assertTrue(conn["unionfind"])
        self.assertTrue(conn["agree"])

    def test_07_device_crud(self):
        # Add device
        status, _, body = self.request("POST", "/api/device", {"name": "TestNode_99"})
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])

        # Try to add duplicate device
        status, _, body = self.request("POST", "/api/device", {"name": "TestNode_99"})
        self.assertEqual(status, 400)
        err = json.loads(body)
        self.assertFalse(err["ok"])

        # Delete device
        status, _, body = self.request("DELETE", "/api/device/TestNode_99")
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])

        # Delete again -> 404
        status, _, _ = self.request("DELETE", "/api/device/TestNode_99")
        self.assertEqual(status, 404)

    def test_08_link_crud(self):
        self.request("POST", "/api/device", {"name": "NodeX"})
        self.request("POST", "/api/device", {"name": "NodeY"})

        # Add link
        status, _, body = self.request("POST", "/api/link", {"a": "NodeX", "b": "NodeY", "cost": 42})
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])
        self.assertEqual(data["link"]["cost"], 42)

        # Update link cost
        status, _, body = self.request("POST", "/api/link", {"a": "NodeY", "b": "NodeX", "cost": 99})
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertEqual(data["link"]["cost"], 99)

        # Remove link
        status, _, body = self.request("DELETE", "/api/link", {"a": "NodeX", "b": "NodeY"})
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])

        # Clean up
        self.request("DELETE", "/api/device/NodeX")
        self.request("DELETE", "/api/device/NodeY")

    def test_09_simulation(self):
        self.request("POST", "/api/load/sample_small.txt")

        # Simulate link failure A-B
        status, _, body = self.request("POST", "/api/simulate", {"type": "link", "a": "A", "b": "B"})
        self.assertEqual(status, 200)
        sim = json.loads(body)
        self.assertTrue(sim["ok"])
        self.assertEqual(sim["componentsBefore"], 1)
        self.assertEqual(sim["componentsAfter"], 2)

        # Simulate device failure B
        status, _, body = self.request("POST", "/api/simulate", {"type": "device", "name": "B"})
        self.assertEqual(status, 200)
        sim = json.loads(body)
        self.assertTrue(sim["ok"])
        self.assertEqual(sim["componentsBefore"], 1)
        self.assertEqual(sim["componentsAfter"], 2)

    def test_10_export_import(self):
        self.request("POST", "/api/load/sample_small.txt")

        # Export
        status, headers, export_txt = self.request("GET", "/api/export")
        self.assertEqual(status, 200)
        self.assertIn("attachment", headers.get("Content-Disposition", ""))
        self.assertIn("DEVICE A", export_txt)
        self.assertIn("LINK A B 2", export_txt)

        # Import
        status, _, body = self.request("POST", "/api/import", export_txt)
        self.assertEqual(status, 200)
        data = json.loads(body)
        self.assertTrue(data["ok"])

    def test_11_report(self):
        self.request("POST", "/api/load/sample_small.txt")
        status, headers, report = self.request("GET", "/api/report")
        self.assertEqual(status, 200)
        self.assertIn("text/plain", headers.get("Content-Type", ""))
        self.assertIn("NETWORK CONNECTIVITY ANALYZER", report)
        self.assertIn("Bridges", report)

    def test_12_validation_errors(self):
        # Invalid device name (bad chars)
        status, _, body = self.request("POST", "/api/device", {"name": "Bad name!"})
        self.assertEqual(status, 400)

        # Long device name (> 31 chars)
        status, _, body = self.request("POST", "/api/device", {"name": "A" * 35})
        self.assertEqual(status, 400)

        # Self-loop
        status, _, body = self.request("POST", "/api/link", {"a": "A", "b": "A", "cost": 5})
        self.assertEqual(status, 400)

        # Invalid cost
        status, _, body = self.request("POST", "/api/link", {"a": "A", "b": "B", "cost": -10})
        self.assertEqual(status, 400)
        status, _, body = self.request("POST", "/api/link", {"a": "A", "b": "B", "cost": 99999})
        self.assertEqual(status, 400)
        status, _, body = self.request("POST", "/api/link", {"a": "A", "b": "B", "cost": "free"})
        self.assertEqual(status, 400)

        # Non-existent devices in link
        status, _, body = self.request("POST", "/api/link", {"a": "NotExist1", "b": "NotExist2", "cost": 5})
        self.assertEqual(status, 400)

        # Path traversal in /api/load
        status, _, body = self.request("POST", "/api/load/../../secret.txt")
        self.assertEqual(status, 400)

        # Non-existent sample
        status, _, body = self.request("POST", "/api/load/non_existent_sample_12345")
        self.assertEqual(status, 404)

        # Malformed JSON
        status, _, body = self.request("POST", "/api/device", data="This is not JSON", headers={"Content-Type": "application/json"})
        self.assertEqual(status, 400)


if __name__ == "__main__":
    unittest.main()
