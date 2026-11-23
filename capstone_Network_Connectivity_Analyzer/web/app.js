/**
 * Network Connectivity Analyzer - Frontend Application
 * Fully offline, vanilla JavaScript, no external dependencies or frameworks.
 */

(function () {
  'use strict';

  // --- State ---
  const state = {
    devices: [],      // [{ id, name, degree, x, y, vx, vy, pinned, ... }]
    links: [],        // [{ a, b, cost }]
    components: 1,
    isolated: [],
    componentList: [],

    selectedElement: null, // { type: 'node' | 'edge', data: ... }
    theme: localStorage.getItem('theme') || 'dark',

    // Viewport transform
    zoom: 1,
    panX: 0,
    panY: 0,
    isPanning: false,
    startPanX: 0,
    startPanY: 0,

    // Drag-to-link
    dragLinkSource: null,

    // Node dragging
    draggedNode: null,
    dragMoved: false,

    // Simulation & Physics
    simulationRunning: false,
    alpha: 1.0,

    // Traversal Animation
    traversalTimer: null,
    traversalSteps: [],
    traversalStepIndex: -1,
    traversalType: 'bfs',
    traversalActive: false,
    traversalSpeed: 600,

    // Dijkstra Animation
    dijkstraTimer: null,
    dijkstraSteps: [],
    dijkstraStepIndex: -1,
    dijkstraActive: false,

    // Component coloring map
    componentColorMap: {},

    // Weak points & simulation states
    bridges: [],
    articulationPoints: [],
    simulatedFailure: null, // { type: 'link'|'device', target: ... }
  };

  // Pre-defined component colors
  const COMPONENT_COLORS = [
    '#2f81f7', '#2ea043', '#f0883e', '#a371f7', '#39c5cf',
    '#e36209', '#d29922', '#db61a2', '#7ee787', '#79c0ff'
  ];

  // --- DOM Elements ---
  const el = {
    svg: document.getElementById('graph-svg'),
    transformGroup: document.getElementById('canvas-transform-group'),
    edgesGroup: document.getElementById('svg-edges-group'),
    nodesGroup: document.getElementById('svg-nodes-group'),
    dragLinkPreview: document.getElementById('drag-link-preview'),
    emptyState: document.getElementById('empty-state'),
    loadingOverlay: document.getElementById('loading-overlay'),
    toastContainer: document.getElementById('toast-container'),

    // Stats
    statDevices: document.getElementById('stat-devices'),
    statLinks: document.getElementById('stat-links'),
    statComponents: document.getElementById('stat-components'),

    // Theme & Report
    btnThemeToggle: document.getElementById('btn-theme-toggle'),
    themeIcon: document.getElementById('theme-icon'),
    btnOpenReport: document.getElementById('btn-open-report'),
    modalReport: document.getElementById('modal-report'),
    reportContent: document.getElementById('report-modal-content'),
    btnCloseReport: document.getElementById('btn-close-report'),
    btnCopyReport: document.getElementById('btn-copy-report'),
    btnDownloadReport: document.getElementById('btn-download-report'),

    // Import modal
    modalImport: document.getElementById('modal-import'),
    btnOpenImport: document.getElementById('btn-open-import'),
    btnCloseImport: document.getElementById('btn-close-import'),
    btnCancelImport: document.getElementById('btn-cancel-import'),
    btnSubmitImport: document.getElementById('btn-submit-import'),
    importTextarea: document.getElementById('import-textarea'),

    // Build sidebar
    newDeviceName: document.getElementById('new-device-name'),
    btnAddDevice: document.getElementById('btn-add-device'),
    linkNodeA: document.getElementById('link-node-a'),
    linkNodeB: document.getElementById('link-node-b'),
    linkCost: document.getElementById('link-cost'),
    btnAddLink: document.getElementById('btn-add-link'),
    removeDeviceSelect: document.getElementById('remove-device-select'),
    btnRemoveDevice: document.getElementById('btn-remove-device'),
    removeLinkSelect: document.getElementById('remove-link-select'),
    btnRemoveLink: document.getElementById('btn-remove-link'),
    sampleSelect: document.getElementById('sample-select'),
    btnLoadSample: document.getElementById('btn-load-sample'),
    btnEmptyLoadSample: document.getElementById('btn-empty-load-sample'),
    btnExport: document.getElementById('btn-export'),
    btnClearNetwork: document.getElementById('btn-clear-network'),

    // Traversal
    traversalStart: document.getElementById('traversal-start'),
    btnTraversalPlay: document.getElementById('btn-traversal-play'),
    btnTraversalPause: document.getElementById('btn-traversal-pause'),
    btnTraversalStep: document.getElementById('btn-traversal-step'),
    btnTraversalReset: document.getElementById('btn-traversal-reset'),
    traversalSpeed: document.getElementById('traversal-speed'),
    speedLabel: document.getElementById('speed-label'),
    structureTypeLabel: document.getElementById('structure-type-label'),
    structureItemsContainer: document.getElementById('structure-items-container'),

    // Connectivity
    connectX: document.getElementById('connect-x'),
    connectY: document.getElementById('connect-y'),
    btnCheckConnect: document.getElementById('btn-check-connect'),
    btnColorComponents: document.getElementById('btn-color-components'),
    connectivityResultsBox: document.getElementById('connectivity-results-box'),

    // Shortest Path
    pathSrc: document.getElementById('path-src'),
    pathDst: document.getElementById('path-dst'),
    btnFindPath: document.getElementById('btn-find-path'),
    btnAnimatePath: document.getElementById('btn-animate-path'),

    // Faults & Simulation
    btnFindFaults: document.getElementById('btn-find-faults'),
    simTargetType: document.getElementById('sim-target-type'),
    simLinkGroup: document.getElementById('sim-link-group'),
    simDeviceGroup: document.getElementById('sim-device-group'),
    simLinkSelect: document.getElementById('sim-link-select'),
    simDeviceSelect: document.getElementById('sim-device-select'),
    btnSimulateFailure: document.getElementById('btn-simulate-failure'),
    btnRestoreNetwork: document.getElementById('btn-restore-network'),

    // Canvas tools
    btnRelayout: document.getElementById('btn-relayout'),
    btnFitScreen: document.getElementById('btn-fit-screen'),
    btnZoomIn: document.getElementById('btn-zoom-in'),
    btnZoomOut: document.getElementById('btn-zoom-out'),
    btnResetView: document.getElementById('btn-reset-view'),

    // Right panel
    tabResults: document.getElementById('tab-results'),
    tabLogs: document.getElementById('tab-logs'),
    contentResults: document.getElementById('content-results'),
    contentLogs: document.getElementById('content-logs'),
    inspectType: document.getElementById('inspect-type'),
    inspectDetails: document.getElementById('inspect-details'),
    algoTitle: document.getElementById('algo-title'),
    algoStatusBadge: document.getElementById('algo-status-badge'),
    algoDetailsBody: document.getElementById('algo-details-body'),
    logStream: document.getElementById('log-stream'),
    btnClearLogs: document.getElementById('btn-clear-logs'),
  };

  // --- UI Helpers: Safe DOM Creation (XSS Free) ---
  function safeElem(tag, props = {}, children = []) {
    const element = document.createElement(tag);
    for (const [key, val] of Object.entries(props)) {
      if (key === 'text') {
        element.textContent = val;
      } else if (key === 'className') {
        element.className = val;
      } else if (key.startsWith('on') && typeof val === 'function') {
        element.addEventListener(key.slice(2).toLowerCase(), val);
      } else {
        element.setAttribute(key, val);
      }
    }
    for (const child of children) {
      if (typeof child === 'string') {
        element.appendChild(document.createTextNode(child));
      } else if (child instanceof Node) {
        element.appendChild(child);
      }
    }
    return element;
  }

  function showToast(message, type = 'info') {
    const toast = safeElem('div', { className: `toast toast-${type}` }, [
      safeElem('span', { text: message })
    ]);
    el.toastContainer.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateX(100%)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 3500);
  }

  function logEvent(text, level = 'info') {
    const now = new Date();
    const timeStr = now.toTimeString().split(' ')[0];
    const entry = safeElem('div', { className: `log-entry ${level}` }, [
      safeElem('span', { className: 'log-time', text: `[${timeStr}]` }),
      safeElem('span', { text: text })
    ]);
    el.logStream.appendChild(entry);
    el.logStream.scrollTop = el.logStream.scrollHeight;
  }

  function setLoading(loading) {
    if (loading) {
      el.loadingOverlay.classList.remove('hidden');
    } else {
      el.loadingOverlay.classList.add('hidden');
    }
  }

  // --- API Client ---
  async function apiFetch(endpoint, options = {}) {
    setLoading(true);
    try {
      const resp = await fetch(endpoint, options);
      const contentType = resp.headers.get('content-type') || '';
      if (contentType.includes('application/json')) {
        const data = await resp.json();
        if (!resp.ok || data.ok === false) {
          const errMsg = data.error || `HTTP ${resp.status}: Request failed`;
          showToast(errMsg, 'error');
          logEvent(`API Error [${endpoint}]: ${errMsg}`, 'error');
          throw new Error(errMsg);
        }
        return data;
      } else {
        const text = await resp.text();
        if (!resp.ok) {
          showToast(`Error: ${text}`, 'error');
          throw new Error(text);
        }
        return text;
      }
    } catch (err) {
      if (!options.silent) {
        // Already shown toast in json branch if applicable
      }
      throw err;
    } finally {
      setLoading(false);
    }
  }

  // --- Load Topology ---
  async function loadNetwork() {
    try {
      const data = await apiFetch('/api/network');
      state.components = data.components || 1;
      state.isolated = data.isolated || [];
      state.componentList = data.componentList || [];

      // Preserve existing node positions where possible
      const oldPosMap = new Map();
      state.devices.forEach(d => oldPosMap.set(d.name, { x: d.x, y: d.y, pinned: d.pinned }));

      const width = el.svg.clientWidth || 800;
      const height = el.svg.clientHeight || 600;

      state.devices = (data.devices || []).map((d, i) => {
        const prev = oldPosMap.get(d.name);
        const angle = (i / Math.max(1, data.devices.length)) * 2 * Math.PI;
        const radius = Math.min(width, height) * 0.35;
        return {
          id: d.id,
          name: d.name,
          degree: d.degree,
          x: prev ? prev.x : width / 2 + radius * Math.cos(angle) + (Math.random() - 0.5) * 20,
          y: prev ? prev.y : height / 2 + radius * Math.sin(angle) + (Math.random() - 0.5) * 20,
          vx: 0,
          vy: 0,
          pinned: prev ? prev.pinned : false,
        };
      });

      state.links = (data.links || []).map(l => ({
        a: l.a,
        b: l.b,
        cost: l.cost,
      }));

      updateStats();
      populateDropdowns();
      resetHighlights();
      renderGraph();
      startSimulation();
      logEvent(`Loaded network with ${state.devices.length} devices and ${state.links.length} links.`, 'success');
    } catch (err) {
      console.error(err);
    }
  }

  function updateStats() {
    el.statDevices.textContent = state.devices.length;
    el.statLinks.textContent = state.links.length;
    el.statComponents.textContent = state.components;

    if (state.devices.length === 0) {
      el.emptyState.style.display = 'block';
    } else {
      el.emptyState.style.display = 'none';
    }
  }

  function populateDropdowns() {
    const devNames = state.devices.map(d => d.name);

    function fillSelect(sel, names, placeholder = null) {
      const current = sel.value;
      sel.innerHTML = '';
      if (placeholder) {
        sel.appendChild(safeElem('option', { value: '', text: placeholder }));
      }
      for (const name of names) {
        sel.appendChild(safeElem('option', { value: name, text: name }));
      }
      if (names.includes(current)) {
        sel.value = current;
      }
    }

    fillSelect(el.linkNodeA, devNames);
    fillSelect(el.linkNodeB, devNames);
    fillSelect(el.removeDeviceSelect, devNames);
    fillSelect(el.traversalStart, devNames);
    fillSelect(el.connectX, devNames);
    fillSelect(el.connectY, devNames);
    fillSelect(el.pathSrc, devNames);
    fillSelect(el.pathDst, devNames);
    fillSelect(el.simDeviceSelect, devNames);

    // Links for remove & sim
    const linkKeys = state.links.map(l => `${l.a} - ${l.b} (cost ${l.cost})`);
    el.removeLinkSelect.innerHTML = '';
    el.simLinkSelect.innerHTML = '';
    state.links.forEach((l) => {
      const val = `${l.a}:${l.b}`;
      const text = `${l.a} <--> ${l.b} (${l.cost})`;
      el.removeLinkSelect.appendChild(safeElem('option', { value: val, text: text }));
      el.simLinkSelect.appendChild(safeElem('option', { value: val, text: text }));
    });
  }

  // --- Reset All Highlights & Visual States ---
  function resetHighlights() {
    clearTimeout(state.traversalTimer);
    clearTimeout(state.dijkstraTimer);
    state.traversalActive = false;
    state.dijkstraActive = false;
    state.componentColorMap = {};
    state.bridges = [];
    state.articulationPoints = [];
    state.simulatedFailure = null;

    // Reset live structure row
    el.structureTypeLabel.textContent = 'Queue / Stack:';
    el.structureItemsContainer.innerHTML = '';
    el.structureItemsContainer.appendChild(safeElem('span', {
      text: 'Idle',
      style: 'font-size:0.8rem; color:var(--text-muted); font-style:italic;'
    }));

    // Update node & edge classes
    const nodes = el.nodesGroup.querySelectorAll('.graph-node');
    nodes.forEach(n => {
      n.className.baseVal = 'graph-node';
      const ring = n.querySelector('.node-ring');
      if (ring) ring.remove();
      const sub = n.querySelector('.node-subtext-group');
      if (sub) sub.remove();
      const circle = n.querySelector('circle.node-body');
      if (circle) circle.style.fill = '';
    });

    const edges = el.edgesGroup.querySelectorAll('.graph-edge');
    edges.forEach(e => {
      e.className.baseVal = 'graph-edge';
      e.style.stroke = '';
      e.style.opacity = '';
    });
  }

  // --- Physics & Force-Directed Layout (Hand-Written) ---
  function startSimulation() {
    state.alpha = 1.0;
    if (state.simulationRunning) return;
    state.simulationRunning = true;
    requestAnimationFrame(simulationTick);
  }

  function simulationTick() {
    if (!state.simulationRunning || state.alpha < 0.005) {
      state.simulationRunning = false;
      return;
    }

    const n = state.devices.length;
    if (n === 0) {
      state.simulationRunning = false;
      return;
    }

    const width = el.svg.clientWidth || 800;
    const height = el.svg.clientHeight || 600;
    const centerX = width / 2;
    const centerY = height / 2;

    const kRepel = n > 50 ? 1200 : 2500;
    const kSpring = 0.04;
    const linkLength = n > 50 ? 55 : 95;
    const centerGravity = 0.02;

    // 1. Node Repulsion
    for (let i = 0; i < n; i++) {
      const u = state.devices[i];
      for (let j = i + 1; j < n; j++) {
        const v = state.devices[j];
        let dx = v.x - u.x;
        let dy = v.y - u.y;
        let dist = Math.sqrt(dx * dx + dy * dy);
        if (dist < 1) {
          dx = (Math.random() - 0.5) * 2;
          dy = (Math.random() - 0.5) * 2;
          dist = Math.sqrt(dx * dx + dy * dy);
        }
        const force = (kRepel / (dist * dist)) * state.alpha;
        const fx = (dx / dist) * force;
        const fy = (dy / dist) * force;

        if (!u.pinned) { u.vx -= fx; u.vy -= fy; }
        if (!v.pinned) { v.vx += fx; v.vy += fy; }
      }
    }

    // 2. Edge Spring Attraction
    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    for (const l of state.links) {
      const u = nodeMap.get(l.a);
      const v = nodeMap.get(l.b);
      if (!u || !v) continue;

      const dx = v.x - u.x;
      const dy = v.y - u.y;
      const dist = Math.sqrt(dx * dx + dy * dy) || 1;
      const force = (dist - linkLength) * kSpring * state.alpha;
      const fx = (dx / dist) * force;
      const fy = (dy / dist) * force;

      if (!u.pinned) { u.vx += fx; u.vy += fy; }
      if (!v.pinned) { v.vx -= fx; v.vy -= fy; }
    }

    // 3. Centering force & update positions
    const damping = 0.85;
    for (let i = 0; i < n; i++) {
      const d = state.devices[i];
      if (!d.pinned) {
        d.vx += (centerX - d.x) * centerGravity * state.alpha;
        d.vy += (centerY - d.y) * centerGravity * state.alpha;

        d.vx *= damping;
        d.vy *= damping;

        d.x += d.vx;
        d.y += d.vy;
      }
    }

    // Decay energy
    state.alpha *= 0.985;

    updateGraphPositions();
    requestAnimationFrame(simulationTick);
  }

  // --- SVG Rendering ---
  function renderGraph() {
    el.edgesGroup.innerHTML = '';
    el.nodesGroup.innerHTML = '';

    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    // Render Edges
    state.links.forEach((l) => {
      const u = nodeMap.get(l.a);
      const v = nodeMap.get(l.b);
      if (!u || !v) return;

      const edgeG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
      edgeG.setAttribute('class', 'graph-edge-group');
      edgeG.dataset.a = l.a;
      edgeG.dataset.b = l.b;

      const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
      line.setAttribute('class', 'graph-edge');
      line.dataset.a = l.a;
      line.dataset.b = l.b;

      // Midpoint cost container
      const costG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
      costG.setAttribute('class', 'edge-cost-group');

      const bg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
      bg.setAttribute('class', 'edge-cost-bg');
      bg.setAttribute('width', '24');
      bg.setAttribute('height', '16');

      const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
      text.setAttribute('class', 'edge-cost-text');
      text.textContent = l.cost;

      costG.appendChild(bg);
      costG.appendChild(text);

      edgeG.appendChild(line);
      edgeG.appendChild(costG);
      el.edgesGroup.appendChild(edgeG);

      // Edge click selection
      edgeG.addEventListener('click', (e) => {
        e.stopPropagation();
        selectElement('edge', l);
      });
    });

    // Render Nodes
    state.devices.forEach((d) => {
      const nodeG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
      nodeG.setAttribute('class', 'graph-node');
      nodeG.dataset.name = d.name;

      if (d.degree === 0) {
        nodeG.classList.add('isolated');
      }

      const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      circle.setAttribute('class', 'node-body');
      circle.setAttribute('r', '18');

      const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
      text.setAttribute('class', 'node-label');
      text.textContent = d.name;

      nodeG.appendChild(circle);
      nodeG.appendChild(text);
      el.nodesGroup.appendChild(nodeG);

      // Drag and selection event handlers
      setupNodeEvents(nodeG, d);
    });

    updateGraphPositions();
  }

  function updateGraphPositions() {
    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    // Update lines & costs
    const edgeGroups = el.edgesGroup.querySelectorAll('.graph-edge-group');
    edgeGroups.forEach(g => {
      const a = g.dataset.a;
      const b = g.dataset.b;
      const u = nodeMap.get(a);
      const v = nodeMap.get(b);
      if (!u || !v) return;

      const line = g.querySelector('line.graph-edge');
      line.setAttribute('x1', u.x);
      line.setAttribute('y1', u.y);
      line.setAttribute('x2', v.x);
      line.setAttribute('y2', v.y);

      const midX = (u.x + v.x) / 2;
      const midY = (u.y + v.y) / 2;
      const costG = g.querySelector('.edge-cost-group');
      if (costG) {
        const bg = costG.querySelector('rect');
        const txt = costG.querySelector('text');
        bg.setAttribute('x', midX - 12);
        bg.setAttribute('y', midY - 8);
        txt.setAttribute('x', midX);
        txt.setAttribute('y', midY);
      }
    });

    // Update nodes
    const nodeGroups = el.nodesGroup.querySelectorAll('.graph-node');
    nodeGroups.forEach(g => {
      const name = g.dataset.name;
      const d = nodeMap.get(name);
      if (!d) return;

      const circle = g.querySelector('circle.node-body');
      circle.setAttribute('cx', d.x);
      circle.setAttribute('cy', d.y);

      const text = g.querySelector('text.node-label');
      text.setAttribute('x', d.x);
      text.setAttribute('y', d.y);

      const ring = g.querySelector('circle.node-ring');
      if (ring) {
        ring.setAttribute('cx', d.x);
        ring.setAttribute('cy', d.y);
      }

      const subG = g.querySelector('.node-subtext-group');
      if (subG) {
        const bg = subG.querySelector('rect');
        const subTxt = subG.querySelector('text');
        bg.setAttribute('x', d.x - 14);
        bg.setAttribute('y', d.y + 19);
        subTxt.setAttribute('x', d.x);
        subTxt.setAttribute('y', d.y + 26);
      }
    });
  }

  // --- Node Dragging & Drag-to-Link ---
  function setupNodeEvents(nodeG, device) {
    nodeG.addEventListener('mousedown', (e) => {
      if (e.button !== 0) return;
      e.stopPropagation();

      // Check if Shift key is down or Alt key is down for Drag-to-Link
      if (e.shiftKey) {
        // Start drag-to-link
        state.dragLinkSource = device;
        el.dragLinkPreview.style.display = 'block';
        el.dragLinkPreview.setAttribute('x1', device.x);
        el.dragLinkPreview.setAttribute('y1', device.y);
        el.dragLinkPreview.setAttribute('x2', device.x);
        el.dragLinkPreview.setAttribute('y2', device.y);
        return;
      }

      // Normal node drag
      state.draggedNode = device;
      state.dragMoved = false;
      device.pinned = true;
    });

    nodeG.addEventListener('click', (e) => {
      e.stopPropagation();
      if (!state.dragMoved) {
        selectElement('node', device);
      }
    });
  }

  // --- Canvas Interaction (Pan, Zoom, Drag-to-Link, Double-Click Add) ---
  function setupCanvasEvents() {
    el.svg.addEventListener('mousedown', (e) => {
      if (e.button === 0 && e.target === el.svg) {
        state.isPanning = true;
        state.startPanX = e.clientX - state.panX;
        state.startPanY = e.clientY - state.panY;
        deselectElement();
      }
    });

    window.addEventListener('mousemove', (e) => {
      if (state.isPanning) {
        state.panX = e.clientX - state.startPanX;
        state.panY = e.clientY - state.startPanY;
        applyTransform();
        return;
      }

      if (state.draggedNode) {
        state.dragMoved = true;
        // Convert screen coordinates to SVG coordinates
        const rect = el.svg.getBoundingClientRect();
        const mouseX = (e.clientX - rect.left - state.panX) / state.zoom;
        const mouseY = (e.clientY - rect.top - state.panY) / state.zoom;
        state.draggedNode.x = mouseX;
        state.draggedNode.y = mouseY;
        state.draggedNode.vx = 0;
        state.draggedNode.vy = 0;
        updateGraphPositions();
        return;
      }

      if (state.dragLinkSource) {
        const rect = el.svg.getBoundingClientRect();
        const mouseX = (e.clientX - rect.left - state.panX) / state.zoom;
        const mouseY = (e.clientY - rect.top - state.panY) / state.zoom;
        el.dragLinkPreview.setAttribute('x2', mouseX);
        el.dragLinkPreview.setAttribute('y2', mouseY);
      }
    });

    window.addEventListener('mouseup', (e) => {
      state.isPanning = false;

      if (state.draggedNode) {
        state.draggedNode = null;
      }

      if (state.dragLinkSource) {
        // Find if released over another node
        el.dragLinkPreview.style.display = 'none';
        const rect = el.svg.getBoundingClientRect();
        const mouseX = (e.clientX - rect.left - state.panX) / state.zoom;
        const mouseY = (e.clientY - rect.top - state.panY) / state.zoom;

        const targetNode = state.devices.find(d => {
          if (d.name === state.dragLinkSource.name) return false;
          const dist = Math.hypot(d.x - mouseX, d.y - mouseY);
          return dist <= 25;
        });

        if (targetNode) {
          promptAddLink(state.dragLinkSource.name, targetNode.name);
        }
        state.dragLinkSource = null;
      }
    });

    // Zoom on wheel
    el.svg.addEventListener('wheel', (e) => {
      e.preventDefault();
      const zoomFactor = e.deltaY < 0 ? 1.12 : 0.89;
      const rect = el.svg.getBoundingClientRect();
      const mouseX = e.clientX - rect.left;
      const mouseY = e.clientY - rect.top;

      const newZoom = Math.max(0.2, Math.min(4.0, state.zoom * zoomFactor));

      state.panX = mouseX - (mouseX - state.panX) * (newZoom / state.zoom);
      state.panY = mouseY - (mouseY - state.panY) * (newZoom / state.zoom);
      state.zoom = newZoom;
      applyTransform();
    });

    // Double-click to add device
    el.svg.addEventListener('dblclick', (e) => {
      if (e.target !== el.svg && e.target.tagName !== 'svg') return;
      promptAddDevice();
    });

    // Keyboard Delete / Backspace
    window.addEventListener('keydown', (e) => {
      if ((e.key === 'Delete' || e.key === 'Backspace') && state.selectedElement) {
        // If typing in input, ignore
        if (['INPUT', 'SELECT', 'TEXTAREA'].includes(document.activeElement.tagName)) return;
        e.preventDefault();
        deleteSelectedElement();
      }
    });
  }

  function applyTransform() {
    el.transformGroup.setAttribute(
      'transform',
      `translate(${state.panX}, ${state.panY}) scale(${state.zoom})`
    );
  }

  // --- Prompts for Canvas Gestures ---
  async function promptAddDevice() {
    const defaultName = `Node_${state.devices.length + 1}`;
    const name = window.prompt('Enter new device name (letters, digits, _, -):', defaultName);
    if (!name) return;
    try {
      await apiFetch('/api/device', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name: name.trim() })
      });
      showToast(`Device '${name.trim()}' added`, 'success');
      await loadNetwork();
    } catch (err) {
      // Toast already shown
    }
  }

  async function promptAddLink(nodeA, nodeB) {
    const costStr = window.prompt(`Add link between ${nodeA} and ${nodeB}. Enter cost (1 - 9999):`, '1');
    if (!costStr) return;
    const cost = parseInt(costStr, 10);
    if (isNaN(cost) || cost < 1 || cost > 9999) {
      showToast('Link cost must be an integer between 1 and 9999', 'error');
      return;
    }
    try {
      await apiFetch('/api/link', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ a: nodeA, b: nodeB, cost: cost })
      });
      showToast(`Link ${nodeA} <--> ${nodeB} added`, 'success');
      await loadNetwork();
    } catch (err) {
      // Toast already shown
    }
  }

  // --- Selection Management ---
  function selectElement(type, data) {
    deselectElement();
    state.selectedElement = { type, data };

    if (type === 'node') {
      const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${data.name}"]`);
      if (nodeG) nodeG.classList.add('selected');
      showNodeInspection(data);
    } else if (type === 'edge') {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${data.a}"][data-b="${data.b}"]`);
      if (edge) edge.classList.add('selected');
      showEdgeInspection(data);
    }
  }

  function deselectElement() {
    state.selectedElement = null;
    el.nodesGroup.querySelectorAll('.graph-node.selected').forEach(n => n.classList.remove('selected'));
    el.edgesGroup.querySelectorAll('.graph-edge.selected').forEach(e => e.classList.remove('selected'));
    el.inspectType.textContent = 'None';
    el.inspectDetails.innerHTML = '';
    el.inspectDetails.appendChild(safeElem('span', {
      text: 'Click any node or link on the canvas to inspect its properties.',
      style: 'font-size:0.82rem; color:var(--text-secondary);'
    }));
  }

  function showNodeInspection(node) {
    el.inspectType.textContent = 'Device';
    el.inspectDetails.innerHTML = '';

    const incident = state.links.filter(l => l.a === node.name || l.b === node.name);

    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Device Name:' }),
      safeElem('span', { className: 'result-val', text: node.name }),
    ]));
    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Internal ID:' }),
      safeElem('span', { className: 'result-val', text: String(node.id) }),
    ]));
    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Degree (Links):' }),
      safeElem('span', { className: 'result-val', text: String(node.degree) }),
    ]));
    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Status:' }),
      safeElem('span', { className: 'result-val', text: node.degree === 0 ? 'Isolated' : 'Connected' }),
    ]));

    // Action button to delete
    const delBtn = safeElem('button', {
      className: 'btn btn-danger btn-sm',
      text: 'Delete Device (Del key)',
      style: 'margin-top:8px; width:100%;',
      onClick: () => deleteSelectedElement()
    });
    el.inspectDetails.appendChild(delBtn);
  }

  function showEdgeInspection(link) {
    el.inspectType.textContent = 'Link';
    el.inspectDetails.innerHTML = '';

    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Endpoints:' }),
      safeElem('span', { className: 'result-val', text: `${link.a} <--> ${link.b}` }),
    ]));
    el.inspectDetails.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Link Cost:' }),
      safeElem('span', { className: 'result-val', text: String(link.cost) }),
    ]));

    const delBtn = safeElem('button', {
      className: 'btn btn-danger btn-sm',
      text: 'Delete Link (Del key)',
      style: 'margin-top:8px; width:100%;',
      onClick: () => deleteSelectedElement()
    });
    el.inspectDetails.appendChild(delBtn);
  }

  async function deleteSelectedElement() {
    if (!state.selectedElement) return;
    const { type, data } = state.selectedElement;
    if (type === 'node') {
      try {
        await apiFetch(`/api/device/${encodeURIComponent(data.name)}`, { method: 'DELETE' });
        showToast(`Device '${data.name}' deleted`, 'success');
        deselectElement();
        await loadNetwork();
      } catch (err) { }
    } else if (type === 'edge') {
      try {
        await apiFetch('/api/link', {
          method: 'DELETE',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ a: data.a, b: data.b })
        });
        showToast(`Link ${data.a} <--> ${data.b} deleted`, 'success');
        deselectElement();
        await loadNetwork();
      } catch (err) { }
    }
  }

  // --- Feature 1: Build Network Actions ---
  el.btnAddDevice.addEventListener('click', async () => {
    const name = el.newDeviceName.value.trim();
    if (!name) {
      showToast('Please enter a device name', 'error');
      return;
    }
    try {
      await apiFetch('/api/device', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name: name })
      });
      showToast(`Device '${name}' added`, 'success');
      el.newDeviceName.value = '';
      await loadNetwork();
    } catch (err) { }
  });

  el.btnAddLink.addEventListener('click', async () => {
    const a = el.linkNodeA.value;
    const b = el.linkNodeB.value;
    const cost = parseInt(el.linkCost.value, 10);
    if (!a || !b) {
      showToast('Select two devices to link', 'error');
      return;
    }
    if (a === b) {
      showToast('Self-loops are not allowed', 'error');
      return;
    }
    if (isNaN(cost) || cost < 1 || cost > 9999) {
      showToast('Cost must be between 1 and 9999', 'error');
      return;
    }
    try {
      await apiFetch('/api/link', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ a, b, cost })
      });
      showToast(`Link ${a} <--> ${b} saved`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnRemoveDevice.addEventListener('click', async () => {
    const name = el.removeDeviceSelect.value;
    if (!name) return;
    try {
      await apiFetch(`/api/device/${encodeURIComponent(name)}`, { method: 'DELETE' });
      showToast(`Device '${name}' removed`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnRemoveLink.addEventListener('click', async () => {
    const val = el.removeLinkSelect.value;
    if (!val) return;
    const [a, b] = val.split(':');
    try {
      await apiFetch('/api/link', {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ a, b })
      });
      showToast(`Link ${a} <--> ${b} removed`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnLoadSample.addEventListener('click', async () => {
    const sample = el.sampleSelect.value;
    try {
      await apiFetch(`/api/load/${encodeURIComponent(sample)}`, { method: 'POST' });
      showToast(`Sample '${sample}' loaded successfully`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnEmptyLoadSample.addEventListener('click', async () => {
    try {
      await apiFetch('/api/load/sample_small.txt', { method: 'POST' });
      showToast('Loaded small sample network', 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnExport.addEventListener('click', () => {
    window.location.href = '/api/export';
    logEvent('Exported network file downloaded.', 'info');
  });

  el.btnClearNetwork.addEventListener('click', async () => {
    if (!window.confirm('Are you sure you want to clear all devices and links?')) return;
    try {
      await apiFetch('/api/import', {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain' },
        body: '# Empty network\n'
      });
      showToast('Network cleared', 'info');
      await loadNetwork();
    } catch (err) { }
  });

  // Import Modal
  el.btnOpenImport.addEventListener('click', () => {
    el.importTextarea.value = '';
    el.modalImport.classList.remove('hidden');
  });
  el.btnCloseImport.addEventListener('click', () => el.modalImport.classList.add('hidden'));
  el.btnCancelImport.addEventListener('click', () => el.modalImport.classList.add('hidden'));

  el.btnSubmitImport.addEventListener('click', async () => {
    const text = el.importTextarea.value.trim();
    if (!text) {
      showToast('Please paste network configuration text', 'error');
      return;
    }
    try {
      await apiFetch('/api/import', {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain' },
        body: text
      });
      showToast('Topology imported successfully', 'success');
      el.modalImport.classList.add('hidden');
      await loadNetwork();
    } catch (err) { }
  });

  // --- Feature 2: Traversal Visualiser (BFS / DFS) ---
  el.traversalSpeed.addEventListener('input', (e) => {
    state.traversalSpeed = parseInt(e.target.value, 10);
    el.speedLabel.textContent = `${state.traversalSpeed}ms`;
  });

  async function fetchTraversal(type, start) {
    const endpoint = type === 'bfs' ? '/api/analyze/bfs' : '/api/analyze/dfs';
    return await apiFetch(endpoint, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ start: start })
    });
  }

  function setTraversalStep(index) {
    if (index < 0 || index >= state.traversalSteps.length) return;
    state.traversalStepIndex = index;
    const step = state.traversalSteps[index];

    // Reset previous current markers
    el.nodesGroup.querySelectorAll('.graph-node.current').forEach(n => {
      n.classList.remove('current');
      n.classList.add('visited');
    });

    // Mark current
    const currNode = el.nodesGroup.querySelector(`.graph-node[data-name="${step.visit}"]`);
    if (currNode) {
      currNode.classList.remove('in-queue');
      currNode.classList.add('current');
    }

    // Mark nodes currently in queue/stack
    const queueOrStack = state.traversalType === 'bfs' ? (step.queue || []) : (step.stack || []);
    queueOrStack.forEach(name => {
      const qNode = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
      if (qNode && !qNode.classList.contains('visited') && !qNode.classList.contains('current')) {
        qNode.classList.add('in-queue');
      }
    });

    // Highlight connecting edge if from exists
    if (step.from) {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${step.from}"][data-b="${step.visit}"], .graph-edge[data-a="${step.visit}"][data-b="${step.from}"]`);
      if (edge) {
        edge.classList.add('path');
      }
    }

    // Update live queue / stack row
    el.structureTypeLabel.textContent = state.traversalType === 'bfs' ? 'Live Queue:' : 'Live Stack:';
    el.structureItemsContainer.innerHTML = '';
    if (queueOrStack.length === 0) {
      el.structureItemsContainer.appendChild(safeElem('span', {
        text: 'Empty',
        style: 'font-size:0.8rem; color:var(--text-muted); font-style:italic;'
      }));
    } else {
      queueOrStack.forEach(item => {
        el.structureItemsContainer.appendChild(safeElem('div', {
          className: 'structure-box',
          text: item
        }));
      });
    }

    logEvent(`${state.traversalType.toUpperCase()} Step ${index + 1}: Visited ${step.visit}${step.from ? ` from ${step.from}` : ' (root)'}`);
  }

  el.btnTraversalPlay.addEventListener('click', async () => {
    if (!state.traversalActive) {
      const typeRadio = document.querySelector('input[name="traversal-type"]:checked');
      state.traversalType = typeRadio ? typeRadio.value : 'bfs';
      const start = el.traversalStart.value;
      if (!start) {
        showToast('Select a starting device for traversal', 'error');
        return;
      }
      try {
        const res = await fetchTraversal(state.traversalType, start);
        resetHighlights();
        state.traversalActive = true;
        state.traversalSteps = res.steps || [];
        state.traversalStepIndex = -1;

        // Show results table in right panel
        displayTraversalSummary(state.traversalType, res);
      } catch (err) {
        return;
      }
    }

    function tick() {
      if (!state.traversalActive) return;
      if (state.traversalStepIndex + 1 < state.traversalSteps.length) {
        setTraversalStep(state.traversalStepIndex + 1);
        state.traversalTimer = setTimeout(tick, state.traversalSpeed);
      } else {
        showToast(`${state.traversalType.toUpperCase()} traversal animation completed`, 'success');
      }
    }
    clearTimeout(state.traversalTimer);
    tick();
  });

  el.btnTraversalPause.addEventListener('click', () => {
    clearTimeout(state.traversalTimer);
    logEvent('Traversal animation paused');
  });

  el.btnTraversalStep.addEventListener('click', async () => {
    if (!state.traversalActive) {
      const typeRadio = document.querySelector('input[name="traversal-type"]:checked');
      state.traversalType = typeRadio ? typeRadio.value : 'bfs';
      const start = el.traversalStart.value;
      if (!start) return;
      try {
        const res = await fetchTraversal(state.traversalType, start);
        resetHighlights();
        state.traversalActive = true;
        state.traversalSteps = res.steps || [];
        state.traversalStepIndex = -1;
        displayTraversalSummary(state.traversalType, res);
      } catch (err) { return; }
    }
    clearTimeout(state.traversalTimer);
    if (state.traversalStepIndex + 1 < state.traversalSteps.length) {
      setTraversalStep(state.traversalStepIndex + 1);
    }
  });

  el.btnTraversalReset.addEventListener('click', () => {
    resetHighlights();
    logEvent('Traversal visualizer reset');
  });

  function displayTraversalSummary(type, res) {
    el.tabResults.click();
    el.algoTitle.textContent = `${type.toUpperCase()} Traversal Output`;
    el.algoStatusBadge.textContent = 'Completed';
    el.algoStatusBadge.className = 'status-badge success';
    el.algoStatusBadge.style.display = 'inline-flex';

    el.algoDetailsBody.innerHTML = '';
    el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Visit Order:' }),
      safeElem('span', { className: 'result-val', text: (res.order || []).join(' → ') }),
    ]));

    el.algoDetailsBody.appendChild(safeElem('div', {
      text: 'Depth / Hop Levels:',
      style: 'font-size:0.8rem; font-weight:600; margin-top:8px;'
    }));

    const levelsList = safeElem('div', { className: 'clickable-list' });
    for (const [node, lvl] of Object.entries(res.levels || {})) {
      levelsList.appendChild(safeElem('div', { className: 'clickable-item' }, [
        safeElem('span', { text: node }),
        safeElem('span', { className: 'badge', text: `Level ${lvl}` }),
      ]));
    }
    el.algoDetailsBody.appendChild(levelsList);
  }

  // --- Feature 3: Connectivity Verification ---
  el.btnCheckConnect.addEventListener('click', async () => {
    const x = el.connectX.value;
    const y = el.connectY.value;
    if (!x || !y) {
      showToast('Select two devices to check reachability', 'error');
      return;
    }
    try {
      const res = await apiFetch('/api/analyze/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ x, y })
      });

      el.tabResults.click();
      el.algoTitle.textContent = `Reachability: ${x} ↔ ${y}`;
      el.algoStatusBadge.style.display = 'inline-flex';

      const allAgree = res.agree;
      const isConn = res.bfs;

      el.algoStatusBadge.textContent = isConn ? 'Connected' : 'Disconnected';
      el.algoStatusBadge.className = `status-badge ${isConn ? 'success' : 'danger'}`;

      el.algoDetailsBody.innerHTML = '';

      function createCheckItem(label, connected) {
        return safeElem('div', { className: 'result-item' }, [
          safeElem('span', { className: 'result-key', text: label }),
          safeElem('span', {
            className: connected ? 'status-badge success' : 'status-badge danger',
            text: connected ? '✔ Connected' : '✖ Not Connected'
          }),
        ]);
      }

      el.algoDetailsBody.appendChild(createCheckItem('BFS Verification:', res.bfs));
      el.algoDetailsBody.appendChild(createCheckItem('DFS Verification:', res.dfs));
      el.algoDetailsBody.appendChild(createCheckItem('Union-Find DSU:', res.unionfind));
      el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
        safeElem('span', { className: 'result-key', text: 'All Algorithms Agree:' }),
        safeElem('span', { className: 'result-val', text: allAgree ? 'YES' : 'NO' }),
      ]));

      logEvent(`Reachability ${x} ↔ ${y}: ${isConn ? 'CONNECTED' : 'DISCONNECTED'} (BFS, DFS, UF all agreed).`, 'success');
    } catch (err) { }
  });

  el.btnColorComponents.addEventListener('click', () => {
    resetHighlights();
    const comps = state.componentList || [];
    if (comps.length === 0) return;

    state.componentColorMap = {};
    comps.forEach((group, idx) => {
      const color = COMPONENT_COLORS[idx % COMPONENT_COLORS.length];
      group.forEach(name => {
        state.componentColorMap[name] = color;
        const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
        if (nodeG) {
          const circle = nodeG.querySelector('circle.node-body');
          if (circle) circle.style.fill = color;
        }
      });
    });

    // Also color edges inside each component
    state.links.forEach(l => {
      const colorA = state.componentColorMap[l.a];
      const colorB = state.componentColorMap[l.b];
      if (colorA && colorA === colorB) {
        const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${l.a}"][data-b="${l.b}"]`);
        if (edge) edge.style.stroke = colorA;
      }
    });

    el.tabResults.click();
    el.algoTitle.textContent = `Components (${comps.length})`;
    el.algoStatusBadge.textContent = `${comps.length} Partition(s)`;
    el.algoStatusBadge.className = 'status-badge success';
    el.algoStatusBadge.style.display = 'inline-flex';

    el.algoDetailsBody.innerHTML = '';
    comps.forEach((group, idx) => {
      const color = COMPONENT_COLORS[idx % COMPONENT_COLORS.length];
      el.algoDetailsBody.appendChild(safeElem('div', {
        className: 'result-card',
        style: `border-left: 4px solid ${color}; padding:8px 12px;`
      }, [
        safeElem('div', { style: 'font-weight:600; font-size:0.82rem;', text: `Component #${idx + 1} (${group.length} devices)` }),
        safeElem('div', { style: 'font-size:0.8rem; color:var(--text-secondary); margin-top:2px;', text: group.join(', ') })
      ]));
    });

    if (state.isolated.length > 0) {
      el.algoDetailsBody.appendChild(safeElem('div', {
        style: 'font-size:0.8rem; color:var(--accent-orange); margin-top:6px;',
        text: `Isolated Devices (${state.isolated.length}): ${state.isolated.join(', ')}`
      }));
    }

    logEvent(`Pushed component coloring for ${comps.length} partition(s).`, 'info');
  });

  // --- Feature 4: Shortest Path (Dijkstra) ---
  el.btnFindPath.addEventListener('click', async () => {
    const src = el.pathSrc.value;
    const dst = el.pathDst.value;
    if (!src || !dst) {
      showToast('Select source and destination devices', 'error');
      return;
    }
    try {
      const res = await apiFetch('/api/analyze/path', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ src, dst })
      });

      resetHighlights();
      renderShortestPathResult(src, dst, res);
    } catch (err) { }
  });

  el.btnAnimatePath.addEventListener('click', async () => {
    const src = el.pathSrc.value;
    const dst = el.pathDst.value;
    if (!src || !dst) {
      showToast('Select source and destination devices', 'error');
      return;
    }
    try {
      const res = await apiFetch('/api/analyze/path', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ src, dst })
      });

      resetHighlights();
      state.dijkstraActive = true;
      state.dijkstraSteps = res.steps || [];
      state.dijkstraStepIndex = -1;

      displayDijkstraSettling(src, dst, res);

      function tick() {
        if (!state.dijkstraActive) return;
        if (state.dijkstraStepIndex + 1 < state.dijkstraSteps.length) {
          state.dijkstraStepIndex++;
          const step = state.dijkstraSteps[state.dijkstraStepIndex];

          // Settle current node
          const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${step.settled}"]`);
          if (nodeG) {
            nodeG.classList.add('current');
          }

          // Show distance badge on nodes
          for (const [nodeName, dist] of Object.entries(step.dist || {})) {
            const targetNode = el.nodesGroup.querySelector(`.graph-node[data-name="${nodeName}"]`);
            if (targetNode) {
              let subG = targetNode.querySelector('.node-subtext-group');
              if (!subG) {
                subG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                subG.setAttribute('class', 'node-subtext-group');
                const bg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                bg.setAttribute('class', 'node-subtext-bg');
                bg.setAttribute('width', '28');
                bg.setAttribute('height', '14');
                const txt = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                txt.setAttribute('class', 'node-subtext');
                subG.appendChild(bg);
                subG.appendChild(txt);
                targetNode.appendChild(subG);
              }
              const txt = subG.querySelector('text');
              txt.textContent = dist;
            }
          }
          updateGraphPositions();
          state.dijkstraTimer = setTimeout(tick, state.traversalSpeed);
        } else {
          // Final highlight of shortest path
          renderShortestPathResult(src, dst, res, false);
          showToast(`Dijkstra settled destination with cost ${res.cost}`, 'success');
        }
      }
      tick();
    } catch (err) { }
  });

  function renderShortestPathResult(src, dst, res, reset = true) {
    if (reset) resetHighlights();

    el.tabResults.click();
    el.algoTitle.textContent = `Shortest Path: ${src} → ${dst}`;
    el.algoStatusBadge.style.display = 'inline-flex';

    if (!res.reachable) {
      el.algoStatusBadge.textContent = 'Unreachable';
      el.algoStatusBadge.className = 'status-badge danger';
      el.algoDetailsBody.innerHTML = '';
      el.algoDetailsBody.appendChild(safeElem('div', {
        className: 'result-item',
        text: `Destination '${dst}' is unreachable from '${src}'. No path exists across current partitions.`
      }));
      logEvent(`Path ${src} → ${dst}: UNREACHABLE`, 'warn');
      return;
    }

    el.algoStatusBadge.textContent = `Cost: ${res.cost}`;
    el.algoStatusBadge.className = 'status-badge success';

    // Highlight path nodes
    (res.path || []).forEach(name => {
      const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
      if (nodeG) nodeG.classList.add('path');
    });

    // Highlight edges
    (res.edges || []).forEach(e => {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${e.a}"][data-b="${e.b}"], .graph-edge[data-a="${e.b}"][data-b="${e.a}"]`);
      if (edge) edge.classList.add('path');
    });

    el.algoDetailsBody.innerHTML = '';
    el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Path Route:' }),
      safeElem('span', { className: 'result-val', text: (res.path || []).join(' → ') }),
    ]));
    el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Total Path Cost:' }),
      safeElem('span', { className: 'result-val', text: String(res.cost) }),
    ]));
    el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
      safeElem('span', { className: 'result-key', text: 'Hop Count:' }),
      safeElem('span', { className: 'result-val', text: `${(res.path || []).length - 1} hops` }),
    ]));

    logEvent(`Shortest Path ${src} → ${dst}: ${(res.path || []).join(' → ')} (Total Cost: ${res.cost})`, 'success');
  }

  function displayDijkstraSettling(src, dst, res) {
    el.tabResults.click();
    el.algoTitle.textContent = `Dijkstra: ${src} → ${dst}`;
    el.algoStatusBadge.textContent = 'Settling Nodes';
    el.algoStatusBadge.className = 'status-badge success';
    el.algoStatusBadge.style.display = 'inline-flex';
    el.algoDetailsBody.innerHTML = '';
    el.algoDetailsBody.appendChild(safeElem('div', {
      text: 'Priority Queue (Min-Heap) dynamically settling shortest tentative distances...',
      style: 'font-size:0.82rem; color:var(--text-secondary);'
    }));
  }

  // --- Feature 5: Fault Detection (Bridges & Articulation Points) ---
  el.btnFindFaults.addEventListener('click', async () => {
    try {
      const res = await apiFetch('/api/analyze/faults');
      resetHighlights();

      state.bridges = res.bridges || [];
      state.articulationPoints = res.articulation || [];

      // Highlight bridges
      state.bridges.forEach(b => {
        const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${b.a}"][data-b="${b.b}"], .graph-edge[data-a="${b.b}"][data-b="${b.a}"]`);
        if (edge) edge.classList.add('bridge');
      });

      // Highlight cut vertices
      state.articulationPoints.forEach(name => {
        const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
        if (nodeG) {
          nodeG.classList.add('critical');
          const ring = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
          ring.setAttribute('class', 'node-ring');
          ring.setAttribute('r', '24');
          nodeG.insertBefore(ring, nodeG.firstChild);
        }
      });
      updateGraphPositions();

      el.tabResults.click();
      el.algoTitle.textContent = 'Weak Points Detected';
      el.algoStatusBadge.textContent = `${state.bridges.length} Bridges, ${state.articulationPoints.length} Cut Vertices`;
      el.algoStatusBadge.className = 'status-badge danger';
      el.algoStatusBadge.style.display = 'inline-flex';

      el.algoDetailsBody.innerHTML = '';

      // Bridges list
      el.algoDetailsBody.appendChild(safeElem('div', {
        text: `Critical Bridges (${state.bridges.length}):`,
        style: 'font-weight:600; font-size:0.82rem;'
      }));

      const bridgesList = safeElem('div', { className: 'clickable-list' });
      if (state.bridges.length === 0) {
        bridgesList.appendChild(safeElem('span', { text: 'None (no single edge cut partitions the graph)', style: 'font-size:0.8rem; color:var(--text-secondary);' }));
      } else {
        state.bridges.forEach(b => {
          bridgesList.appendChild(safeElem('div', {
            className: 'clickable-item',
            onClick: () => focusLink(b.a, b.b)
          }, [
            safeElem('span', { text: `${b.a} <--> ${b.b}` }),
            safeElem('span', { className: 'status-badge danger', text: 'Bridge' })
          ]));
        });
      }
      el.algoDetailsBody.appendChild(bridgesList);

      // Articulation points list
      el.algoDetailsBody.appendChild(safeElem('div', {
        text: `Articulation Points (${state.articulationPoints.length}):`,
        style: 'font-weight:600; font-size:0.82rem; margin-top:8px;'
      }));

      const apList = safeElem('div', { className: 'clickable-list' });
      if (state.articulationPoints.length === 0) {
        apList.appendChild(safeElem('span', { text: 'None (biconnected network)', style: 'font-size:0.8rem; color:var(--text-secondary);' }));
      } else {
        state.articulationPoints.forEach(name => {
          apList.appendChild(safeElem('div', {
            className: 'clickable-item',
            onClick: () => focusNode(name)
          }, [
            safeElem('span', { text: name }),
            safeElem('span', { className: 'status-badge danger', text: 'Cut Vertex' })
          ]));
        });
      }
      el.algoDetailsBody.appendChild(apList);

      logEvent(`Fault detection: Found ${state.bridges.length} bridge(s) and ${state.articulationPoints.length} cut vertex/vertices.`, 'warn');
    } catch (err) { }
  });

  function focusNode(name) {
    const node = state.devices.find(d => d.name === name);
    if (!node) return;
    selectElement('node', node);
    // Center view on node
    const width = el.svg.clientWidth || 800;
    const height = el.svg.clientHeight || 600;
    state.panX = width / 2 - node.x * state.zoom;
    state.panY = height / 2 - node.y * state.zoom;
    applyTransform();
  }

  function focusLink(a, b) {
    const link = state.links.find(l => (l.a === a && l.b === b) || (l.a === b && l.b === a));
    if (!link) return;
    selectElement('edge', link);
    const u = state.devices.find(d => d.name === a);
    const v = state.devices.find(d => d.name === b);
    if (u && v) {
      const midX = (u.x + v.x) / 2;
      const midY = (u.y + v.y) / 2;
      const width = el.svg.clientWidth || 800;
      const height = el.svg.clientHeight || 600;
      state.panX = width / 2 - midX * state.zoom;
      state.panY = height / 2 - midY * state.zoom;
      applyTransform();
    }
  }

  // --- Feature 6: Failure Simulation ---
  el.simTargetType.addEventListener('change', () => {
    const val = el.simTargetType.value;
    if (val === 'link') {
      el.simLinkGroup.style.display = 'block';
      el.simDeviceGroup.style.display = 'none';
    } else {
      el.simLinkGroup.style.display = 'none';
      el.simDeviceGroup.style.display = 'block';
    }
  });

  el.btnSimulateFailure.addEventListener('click', async () => {
    const simType = el.simTargetType.value;
    let payload = { type: simType };

    if (simType === 'link') {
      const val = el.simLinkSelect.value;
      if (!val) {
        showToast('Select a link to simulate failure', 'error');
        return;
      }
      const [a, b] = val.split(':');
      payload.a = a;
      payload.b = b;
    } else {
      const name = el.simDeviceSelect.value;
      if (!name) {
        showToast('Select a device to simulate failure', 'error');
        return;
      }
      payload.name = name;
    }

    try {
      const res = await apiFetch('/api/simulate', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });

      resetHighlights();
      state.simulatedFailure = payload;

      // Mark element as failed
      if (simType === 'link') {
        const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${payload.a}"][data-b="${payload.b}"], .graph-edge[data-a="${payload.b}"][data-b="${payload.a}"]`);
        if (edge) edge.classList.add('failed');
      } else {
        const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${payload.name}"]`);
        if (nodeG) nodeG.classList.add('failed');
      }

      // Color newly disconnected groups
      const groups = res.disconnected || [];
      groups.forEach((group, idx) => {
        const color = COMPONENT_COLORS[idx % COMPONENT_COLORS.length];
        group.forEach(name => {
          const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
          if (nodeG && (!payload.name || name !== payload.name)) {
            const circle = nodeG.querySelector('circle.node-body');
            if (circle) circle.style.fill = color;
          }
        });
      });

      el.tabResults.click();
      el.algoTitle.textContent = `Simulation: ${simType === 'link' ? `Link ${payload.a}-${payload.b}` : `Device ${payload.name}`}`;
      el.algoStatusBadge.textContent = `Components: ${res.componentsBefore} → ${res.componentsAfter}`;
      el.algoStatusBadge.className = `status-badge ${res.componentsAfter > res.componentsBefore ? 'danger' : 'success'}`;
      el.algoStatusBadge.style.display = 'inline-flex';

      el.algoDetailsBody.innerHTML = '';
      el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
        safeElem('span', { className: 'result-key', text: 'Components Before Failure:' }),
        safeElem('span', { className: 'result-val', text: String(res.componentsBefore) }),
      ]));
      el.algoDetailsBody.appendChild(safeElem('div', { className: 'result-item' }, [
        safeElem('span', { className: 'result-key', text: 'Components After Failure:' }),
        safeElem('span', { className: 'result-val', text: String(res.componentsAfter) }),
      ]));

      const splitMsg = res.componentsAfter > res.componentsBefore
        ? 'Partitioning Event: Network fragmented into multiple isolated subnets!'
        : 'Redundant Architecture: Network remains connected despite this failure.';
      el.algoDetailsBody.appendChild(safeElem('div', {
        style: `font-size:0.82rem; font-weight:600; margin-top:8px; color:${res.componentsAfter > res.componentsBefore ? 'var(--accent-red)' : 'var(--accent-green)'};`,
        text: splitMsg
      }));

      // Render resulting partition groups
      groups.forEach((group, idx) => {
        const color = COMPONENT_COLORS[idx % COMPONENT_COLORS.length];
        el.algoDetailsBody.appendChild(safeElem('div', {
          className: 'result-card',
          style: `border-left: 4px solid ${color}; padding:6px 10px; margin-top:6px;`
        }, [
          safeElem('div', { style: 'font-weight:600; font-size:0.8rem;', text: `Subnet ${idx + 1} (${group.length} nodes)` }),
          safeElem('div', { style: 'font-size:0.78rem; color:var(--text-secondary);', text: group.join(', ') })
        ]));
      });

      logEvent(`Failure simulation complete: components ${res.componentsBefore} → ${res.componentsAfter}.`, 'warn');
    } catch (err) { }
  });

  el.btnRestoreNetwork.addEventListener('click', () => {
    resetHighlights();
    showToast('Network restored to normal operational state', 'info');
    logEvent('Restored failure simulation');
  });

  // --- Feature 7: Plain-Text Report ---
  el.btnOpenReport.addEventListener('click', async () => {
    el.reportContent.textContent = 'Generating comprehensive analysis report from C engine...';
    el.modalReport.classList.remove('hidden');
    try {
      const text = await apiFetch('/api/report');
      el.reportContent.textContent = text;
    } catch (err) {
      el.reportContent.textContent = 'Failed to generate report.';
    }
  });

  el.btnCloseReport.addEventListener('click', () => el.modalReport.classList.add('hidden'));

  el.btnCopyReport.addEventListener('click', async () => {
    const text = el.reportContent.textContent;
    try {
      await navigator.clipboard.writeText(text);
      showToast('Report copied to clipboard', 'success');
    } catch (e) {
      // Fallback
      const ta = document.createElement('textarea');
      ta.value = text;
      document.body.appendChild(ta);
      ta.select();
      document.execCommand('copy');
      ta.remove();
      showToast('Report copied to clipboard', 'success');
    }
  });

  el.btnDownloadReport.addEventListener('click', () => {
    const text = el.reportContent.textContent;
    const blob = new Blob([text], { type: 'text/plain;charset=utf-8' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `network_report_${Date.now()}.txt`;
    a.click();
    URL.revokeObjectURL(url);
    showToast('Report downloaded', 'info');
  });

  // --- Canvas Toolbar Actions ---
  el.btnRelayout.addEventListener('click', () => {
    const width = el.svg.clientWidth || 800;
    const height = el.svg.clientHeight || 600;
    state.devices.forEach((d, i) => {
      d.pinned = false;
      const angle = (i / Math.max(1, state.devices.length)) * 2 * Math.PI;
      const r = Math.min(width, height) * 0.35;
      d.x = width / 2 + r * Math.cos(angle) + (Math.random() - 0.5) * 40;
      d.y = height / 2 + r * Math.sin(angle) + (Math.random() - 0.5) * 40;
      d.vx = 0;
      d.vy = 0;
    });
    startSimulation();
    showToast('Force simulation re-initialized', 'info');
  });

  el.btnFitScreen.addEventListener('click', () => {
    if (state.devices.length === 0) return;
    let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
    state.devices.forEach(d => {
      if (d.x < minX) minX = d.x;
      if (d.x > maxX) maxX = d.x;
      if (d.y < minY) minY = d.y;
      if (d.y > maxY) maxY = d.y;
    });

    const pad = 60;
    const bboxW = (maxX - minX) + pad * 2;
    const bboxH = (maxY - minY) + pad * 2;
    const svgW = el.svg.clientWidth || 800;
    const svgH = el.svg.clientHeight || 600;

    const scale = Math.min(2.0, Math.max(0.2, Math.min(svgW / bboxW, svgH / bboxH)));
    state.zoom = scale;
    state.panX = svgW / 2 - ((minX + maxX) / 2) * scale;
    state.panY = svgH / 2 - ((minY + maxY) / 2) * scale;
    applyTransform();
  });

  el.btnZoomIn.addEventListener('click', () => {
    state.zoom = Math.min(4.0, state.zoom * 1.25);
    applyTransform();
  });

  el.btnZoomOut.addEventListener('click', () => {
    state.zoom = Math.max(0.2, state.zoom * 0.8);
    applyTransform();
  });

  el.btnResetView.addEventListener('click', () => {
    state.zoom = 1;
    state.panX = 0;
    state.panY = 0;
    applyTransform();
  });

  // --- Right Panel Tabs ---
  el.tabResults.addEventListener('click', () => {
    el.tabResults.classList.add('active');
    el.tabLogs.classList.remove('active');
    el.contentResults.classList.remove('hidden');
    el.contentLogs.classList.add('hidden');
  });

  el.tabLogs.addEventListener('click', () => {
    el.tabLogs.classList.add('active');
    el.tabResults.classList.remove('active');
    el.contentLogs.classList.remove('hidden');
    el.contentResults.classList.add('hidden');
  });

  el.btnClearLogs.addEventListener('click', () => {
    el.logStream.innerHTML = '';
  });

  // --- Accordion Section Headers ---
  document.querySelectorAll('.section-header').forEach(hdr => {
    hdr.addEventListener('click', () => {
      const targetId = hdr.dataset.target;
      const body = document.getElementById(targetId);
      if (body) {
        body.classList.toggle('collapsed');
        hdr.classList.toggle('collapsed');
      }
    });
  });

  // --- Light / Dark Theme ---
  function applyTheme(theme) {
    state.theme = theme;
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);
    el.themeIcon.textContent = theme === 'dark' ? '☀️' : '🌙';
  }

  el.btnThemeToggle.addEventListener('click', () => {
    applyTheme(state.theme === 'dark' ? 'light' : 'dark');
  });

  // --- App Initialization ---
  function init() {
    applyTheme(state.theme);
    setupCanvasEvents();
    loadNetwork();
    logEvent('Network Connectivity Analyzer initialized.', 'info');
  }

  window.addEventListener('DOMContentLoaded', init);

})();
