/**
 * Network Connectivity Analyzer - The Transit Map Frontend Application
 * Fully offline, vanilla JavaScript, no external dependencies or frameworks.
 * Strict textContent-only rendering for user-supplied data (XSS Free).
 */

(function () {
  'use strict';

  // --- Transit Component Color Palettes (Accessible 3:1 Contrast) ---
  const COMPONENT_PALETTES = {
    light: ['#1F5FBF', '#D6342C', '#1E9A5A', '#C47B00', '#7A4FB5', '#0A828B'],
    dark: ['#3D7FE0', '#E24A42', '#2CB36F', '#EBB036', '#9A6ED9', '#1CB5C2']
  };

  // --- State ---
  const state = {
    devices: [],      // [{ id, name, degree, x, y, vx, vy, pinned }]
    links: [],        // [{ a, b, cost }]
    components: 1,
    isolated: [],
    componentList: [],
    componentColorMap: {}, // device name -> color hex

    activeTask: 'build',  // 'build' | 'explore' | 'route' | 'faults' | 'simulation'
    drawerOpen: true,
    selectedElement: null, // { type: 'node' | 'edge', data: ... }
    theme: localStorage.getItem('theme') || 'light',

    // Viewport transform (Pan & Zoom)
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

    // Simulation & Physics (Force-directed layout)
    simulationRunning: false,
    alpha: 1.0,

    // Traversal Animation (BFS / DFS)
    traversalTimer: null,
    traversalSteps: [],
    traversalStepIndex: -1,
    traversalType: 'bfs',
    traversalActive: false,
    traversalSpeed: 600,

    // Shortest Path (Dijkstra)
    dijkstraTimer: null,
    dijkstraSteps: [],
    dijkstraStepIndex: -1,
    dijkstraActive: false,
    shortestPathResult: null,

    // Weak points & simulation states
    bridges: [],
    articulationPoints: [],
    simulatedFailure: null, // { type: 'link'|'device', target: ... }
  };

  // --- DOM Elements ---
  const el = {
    // Top Bar & Branding
    statDevices: document.getElementById('stat-devices'),
    statLinks: document.getElementById('stat-links'),
    statComponents: document.getElementById('stat-components'),
    networkSummary: document.getElementById('network-summary'),
    btnOpenReport: document.getElementById('btn-open-report'),
    btnThemeToggle: document.getElementById('btn-theme-toggle'),
    themeIcon: document.getElementById('theme-icon'),

    // Task Rail & Drawer
    taskRail: document.querySelector('.task-rail'),
    railItems: document.querySelectorAll('.rail-item'),
    taskDrawer: document.getElementById('task-drawer'),
    drawerTitle: document.getElementById('drawer-title'),
    btnCloseDrawer: document.getElementById('btn-close-drawer'),
    taskPanels: document.querySelectorAll('.task-panel'),

    // Build Task Panel
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
    btnOpenImport: document.getElementById('btn-open-import'),
    btnExport: document.getElementById('btn-export'),
    btnClearNetwork: document.getElementById('btn-clear-network'),

    // Explore Task Panel
    traversalStart: document.getElementById('traversal-start'),
    btnTraversalPlay: document.getElementById('btn-traversal-play'),
    btnTraversalPause: document.getElementById('btn-traversal-pause'),
    btnTraversalStep: document.getElementById('btn-traversal-step'),
    btnTraversalReset: document.getElementById('btn-traversal-reset'),
    traversalSpeed: document.getElementById('traversal-speed'),
    speedLabel: document.getElementById('speed-label'),
    connectX: document.getElementById('connect-x'),
    connectY: document.getElementById('connect-y'),
    btnCheckConnect: document.getElementById('btn-check-connect'),
    btnColorComponents: document.getElementById('btn-color-components'),
    connectivityResultsBox: document.getElementById('connectivity-results-box'),

    // Route Task Panel
    pathSrc: document.getElementById('path-src'),
    pathDst: document.getElementById('path-dst'),
    btnFindPath: document.getElementById('btn-find-path'),
    btnAnimatePath: document.getElementById('btn-animate-path'),

    // Weak Points Task Panel
    btnFindFaults: document.getElementById('btn-find-faults'),

    // What if (Simulation) Task Panel
    simTargetType: document.getElementById('sim-target-type'),
    simLinkGroup: document.getElementById('sim-link-group'),
    simDeviceGroup: document.getElementById('sim-device-group'),
    simLinkSelect: document.getElementById('sim-link-select'),
    simDeviceSelect: document.getElementById('sim-device-select'),
    btnSimulateFailure: document.getElementById('btn-simulate-failure'),
    btnRestoreNetwork: document.getElementById('btn-restore-network'),

    // Canvas & Toolbar
    canvasArea: document.getElementById('canvas-area'),
    svg: document.getElementById('graph-svg'),
    transformGroup: document.getElementById('canvas-transform-group'),
    edgesGroup: document.getElementById('svg-edges-group'),
    nodesGroup: document.getElementById('svg-nodes-group'),
    overlaysGroup: document.getElementById('svg-overlays-group'),
    dragLinkPreview: document.getElementById('drag-link-preview'),
    emptyState: document.getElementById('empty-state'),
    btnEmptyLoadSample: document.getElementById('btn-empty-load-sample'),
    loadingOverlay: document.getElementById('loading-overlay'),
    btnZoomIn: document.getElementById('btn-zoom-in'),
    btnZoomOut: document.getElementById('btn-zoom-out'),
    btnFitScreen: document.getElementById('btn-fit-screen'),
    btnRelayout: document.getElementById('btn-relayout'),
    btnResetView: document.getElementById('btn-reset-view'),

    // Floating Inspector
    floatingInspector: document.getElementById('floating-inspector'),
    inspectType: document.getElementById('inspect-type'),
    inspectDetails: document.getElementById('inspect-details'),
    btnCloseInspector: document.getElementById('btn-close-inspector'),
    btnInspectorRemove: document.getElementById('btn-inspector-remove'),

    // Transit Legend
    transitLegend: document.getElementById('transit-legend'),
    btnLegendToggle: document.getElementById('btn-legend-toggle'),

    // Bottom Dock
    bottomDock: document.getElementById('bottom-dock'),
    btnDockToggle: document.getElementById('btn-dock-toggle'),
    tabResults: document.getElementById('tab-results'),
    tabLogs: document.getElementById('tab-logs'),
    contentResults: document.getElementById('content-results'),
    contentLogs: document.getElementById('content-logs'),
    algoIndicatorDot: document.getElementById('algo-indicator-dot'),
    algoStatusText: document.getElementById('algo-status-text'),
    structureTypeLabel: document.getElementById('structure-type-label'),
    structureMetaText: document.getElementById('structure-meta-text'),
    structureItemsContainer: document.getElementById('structure-items-container'),
    cardAlgoResults: document.getElementById('card-algo-results'),
    algoTitle: document.getElementById('algo-title'),
    algoStatusBadge: document.getElementById('algo-status-badge'),
    algoDetailsBody: document.getElementById('algo-details-body'),
    logStream: document.getElementById('log-stream'),
    btnClearLogs: document.getElementById('btn-clear-logs'),

    // Modals & Notifications
    modalReport: document.getElementById('modal-report'),
    reportContent: document.getElementById('report-modal-content'),
    btnCloseReport: document.getElementById('btn-close-report'),
    btnCopyReport: document.getElementById('btn-copy-report'),
    btnDownloadReport: document.getElementById('btn-download-report'),
    modalImport: document.getElementById('modal-import'),
    importTextarea: document.getElementById('import-textarea'),
    btnCloseImport: document.getElementById('btn-close-import'),
    btnCancelImport: document.getElementById('btn-cancel-import'),
    btnSubmitImport: document.getElementById('btn-submit-import'),
    toastContainer: document.getElementById('toast-container'),
  };

  // --- Safe DOM Creation (Strict XSS Free) ---
  function safeElem(tag, props = {}, children = []) {
    const el = document.createElement(tag);
    for (const [key, val] of Object.entries(props)) {
      if (key === 'text') {
        el.textContent = val;
      } else if (key === 'className') {
        el.className = val;
      } else if (key.startsWith('on') && typeof val === 'function') {
        el.addEventListener(key.slice(2).toLowerCase(), val);
      } else {
        el.setAttribute(key, val);
      }
    }
    for (const child of children) {
      if (typeof child === 'string' || typeof child === 'number') {
        el.appendChild(document.createTextNode(String(child)));
      } else if (child instanceof Node) {
        el.appendChild(child);
      }
    }
    return el;
  }

  // --- Toast Notifications (Single Line Actionable) ---
  function showToast(message, type = 'info') {
    const toast = safeElem('div', { className: `toast ${type === 'error' ? 'toast-error' : ''}` }, [
      safeElem('span', { text: message })
    ]);
    el.toastContainer.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(8px)';
      toast.style.transition = 'all 0.25s ease';
      setTimeout(() => toast.remove(), 250);
    }, 3800);
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
          logEvent(`Error [${endpoint}]: ${errMsg}`, 'error');
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
      throw err;
    } finally {
      setLoading(false);
    }
  }

  // --- Color Palette Resolution ---
  function getComponentColor(componentIndex) {
    const palette = COMPONENT_PALETTES[state.theme] || COMPONENT_PALETTES.light;
    return palette[componentIndex % palette.length];
  }

  // --- Load Topology ---
  async function loadNetwork() {
    try {
      const data = await apiFetch('/api/network');
      state.components = data.components || 1;
      state.isolated = data.isolated || [];
      state.componentList = data.componentList || [];

      // Build component coloring map (default mode is Color By Component)
      state.componentColorMap = {};
      state.componentList.forEach((group, idx) => {
        const color = getComponentColor(idx);
        group.forEach(name => {
          state.componentColorMap[name] = color;
        });
      });

      // Preserve existing station positions where possible
      const oldPosMap = new Map();
      state.devices.forEach(d => oldPosMap.set(d.name, { x: d.x, y: d.y, pinned: d.pinned }));

      const rect = el.canvasArea.getBoundingClientRect();
      const width = rect.width || 800;
      const height = rect.height || 600;

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
      renderGraph();

      if (state.devices.length > 0) {
        el.emptyState.style.display = 'none';
        startSimulation();
        setTimeout(autoFitGraph, 350);
      } else {
        el.emptyState.style.display = 'block';
      }

      logEvent(`Loaded network: ${state.devices.length} stations, ${state.links.length} lines, ${state.components} network(s).`);
    } catch (err) {
      logEvent(`Failed to load network: ${err.message}`, 'error');
    }
  }

  function updateStats() {
    el.statDevices.textContent = String(state.devices.length);
    el.statLinks.textContent = String(state.links.length);
    el.statComponents.textContent = String(state.components);
  }

  function populateDropdowns() {
    const devNames = state.devices.map(d => d.name);

    function fillSelect(sel, items, defaultVal = '') {
      if (!sel) return;
      sel.innerHTML = '';
      if (items.length === 0) {
        sel.appendChild(safeElem('option', { value: '', text: 'No options available' }));
        return;
      }
      items.forEach(item => {
        const val = typeof item === 'object' ? item.value : item;
        const txt = typeof item === 'object' ? item.text : item;
        const opt = safeElem('option', { value: val, text: txt });
        if (val === defaultVal) opt.selected = true;
        sel.appendChild(opt);
      });
    }

    fillSelect(el.linkNodeA, devNames);
    fillSelect(el.linkNodeB, devNames);
    if (devNames.length > 1 && el.linkNodeB.options.length > 1) {
      el.linkNodeB.selectedIndex = 1;
    }

    fillSelect(el.removeDeviceSelect, devNames);
    fillSelect(el.traversalStart, devNames);
    fillSelect(el.connectX, devNames);
    fillSelect(el.connectY, devNames);
    if (devNames.length > 1 && el.connectY.options.length > 1) {
      el.connectY.selectedIndex = 1;
    }

    fillSelect(el.pathSrc, devNames);
    fillSelect(el.pathDst, devNames);
    if (devNames.length > 1 && el.pathDst.options.length > 1) {
      el.pathDst.selectedIndex = 1;
    }

    fillSelect(el.simDeviceSelect, devNames);

    // Format links as natural language: "A to B, cost 2"
    const linkItems = state.links.map(l => ({
      value: `${l.a}:${l.b}`,
      text: `${l.a} to ${l.b}, cost ${l.cost}`
    }));
    fillSelect(el.removeLinkSelect, linkItems);
    fillSelect(el.simLinkSelect, linkItems);
  }

  function resetHighlights() {
    clearTimeout(state.traversalTimer);
    clearTimeout(state.dijkstraTimer);
    state.traversalActive = false;
    state.dijkstraActive = false;
    state.simulatedFailure = null;
    state.shortestPathResult = null;

    // Reset nodes
    const nodes = el.nodesGroup.querySelectorAll('.graph-node');
    nodes.forEach(n => {
      n.className.baseVal = 'graph-node';
      const disc = n.querySelector('.station-disc');
      if (disc) disc.style.fill = '';
      const ring = n.querySelector('.interchange-outer-ring');
      if (ring) ring.remove();
      const beacon = n.querySelector('.current-beacon-ring');
      if (beacon) beacon.remove();
      const badge = n.querySelector('.queue-badge-group');
      if (badge) badge.remove();
      const cross = n.querySelector('.failure-cross-group');
      if (cross) cross.remove();
    });

    // Reset edges
    const edges = el.edgesGroup.querySelectorAll('.graph-edge');
    edges.forEach(e => {
      e.className.baseVal = 'graph-edge';
      e.style.stroke = '';
      e.style.opacity = '';
    });

    const crossbars = el.edgesGroup.querySelectorAll('.bridge-crossbar');
    crossbars.forEach(c => c.remove());

    // Clear overlays
    el.overlaysGroup.innerHTML = '';

    // Re-apply component colors
    applyComponentColors();

    el.structureItemsContainer.innerHTML = '';
    el.structureItemsContainer.appendChild(safeElem('span', {
      className: 'empty-ticket-hint',
      text: 'Idle. Run a traversal to view live queue or stack items.'
    }));
    el.structureMetaText.textContent = 'Idle';
    el.algoIndicatorDot.style.backgroundColor = 'var(--accent-success)';
    el.algoStatusText.textContent = 'Ready';
  }

  function applyComponentColors() {
    // Color each edge with its component color
    state.links.forEach(l => {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${l.a}"][data-b="${l.b}"], .graph-edge[data-a="${l.b}"][data-b="${l.a}"]`);
      if (edge) {
        const color = state.componentColorMap[l.a] || state.componentColorMap[l.b] || getComponentColor(0);
        edge.style.stroke = color;
      }
    });

    // Re-check isolated devices
    state.devices.forEach(d => {
      const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${d.name}"]`);
      if (nodeG && d.degree === 0) {
        nodeG.classList.add('is-isolated');
      }
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

    const rect = el.canvasArea.getBoundingClientRect();
    const width = rect.width || 800;
    const height = rect.height || 600;
    const centerX = width / 2;
    const centerY = height / 2;

    const kRepel = n > 50 ? 1400 : (n <= 6 ? 4500 : 3200);
    const kSpring = 0.045;
    const linkLength = n > 50 ? 60 : (n <= 6 ? 140 : 110);
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
      if (!v.pinned) { v.vx += fx; v.vy += fy; }
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

    state.alpha *= 0.985;
    updateGraphPositions();
    requestAnimationFrame(simulationTick);
  }

  // --- SVG Transit Map Rendering ---
  function renderGraph() {
    el.edgesGroup.innerHTML = '';
    el.nodesGroup.innerHTML = '';
    el.overlaysGroup.innerHTML = '';

    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    // 1. Render Lines (Edges)
    state.links.forEach((l) => {
      const u = nodeMap.get(l.a);
      const v = nodeMap.get(l.b);
      if (!u || !v) return;

      const edgeG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
      edgeG.setAttribute('class', 'graph-edge-group');
      edgeG.dataset.a = l.a;
      edgeG.dataset.b = l.b;

      // Line
      const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
      line.setAttribute('class', 'graph-edge');
      line.dataset.a = l.a;
      line.dataset.b = l.b;

      const compColor = state.componentColorMap[l.a] || state.componentColorMap[l.b] || getComponentColor(0);
      line.style.stroke = compColor;

      // Cost label: tabular numeral with paper halo (no grey box)
      const costText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
      costText.setAttribute('class', 'edge-cost-text');
      costText.textContent = String(l.cost);

      edgeG.appendChild(line);
      edgeG.appendChild(costText);
      el.edgesGroup.appendChild(edgeG);

      // Edge click selection -> Floating Inspector
      edgeG.addEventListener('click', (e) => {
        e.stopPropagation();
        selectElement('edge', l, e);
      });
    });

    // 2. Render Stations (Nodes)
    state.devices.forEach((d) => {
      const nodeG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
      nodeG.setAttribute('class', 'graph-node');
      nodeG.dataset.name = d.name;

      if (d.degree === 0) {
        nodeG.classList.add('is-isolated');
      }

      // Station disc: white circle, 3px ink stroke
      const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      circle.setAttribute('class', 'station-disc');
      circle.setAttribute('r', '16');

      // Station Label: 14px medium beside station with paper halo
      const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
      text.setAttribute('class', 'station-label');
      text.textContent = d.name;

      nodeG.appendChild(circle);
      nodeG.appendChild(text);
      el.nodesGroup.appendChild(nodeG);

      setupNodeEvents(nodeG, d);
    });

    updateGraphPositions();
  }

  function updateGraphPositions() {
    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    // Update lines and costs
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
      const dx = v.x - u.x;
      const dy = v.y - u.y;
      const len = Math.hypot(dx, dy) || 1;
      const nx = -dy / len;
      const ny = dx / len;

      const costText = g.querySelector('.edge-cost-text');
      if (costText) {
        costText.setAttribute('x', midX - nx * 10);
        costText.setAttribute('y', midY - ny * 10);
      }

      // Update bridge crossbar if present
      const crossbar = g.querySelector('.bridge-crossbar');
      if (crossbar) {
        const tickLen = 13;
        crossbar.setAttribute('x1', midX - nx * tickLen);
        crossbar.setAttribute('y1', midY - ny * tickLen);
        crossbar.setAttribute('x2', midX + nx * tickLen);
        crossbar.setAttribute('y2', midY + ny * tickLen);
      }
    });

    // Update stations
    const nodeGroups = el.nodesGroup.querySelectorAll('.graph-node');
    nodeGroups.forEach(g => {
      const name = g.dataset.name;
      const d = nodeMap.get(name);
      if (!d) return;

      const circle = g.querySelector('.station-disc');
      circle.setAttribute('cx', d.x);
      circle.setAttribute('cy', d.y);

      const text = g.querySelector('.station-label');
      text.setAttribute('x', d.x + 20);
      text.setAttribute('y', d.y + 5);

      const outerRing = g.querySelector('.interchange-outer-ring');
      if (outerRing) {
        outerRing.setAttribute('cx', d.x);
        outerRing.setAttribute('cy', d.y);
      }

      const beacon = g.querySelector('.current-beacon-ring');
      if (beacon) {
        beacon.setAttribute('cx', d.x);
        beacon.setAttribute('cy', d.y);
      }

      const badgeGroup = g.querySelector('.queue-badge-group');
      if (badgeGroup) {
        const bCircle = badgeGroup.querySelector('circle');
        const bText = badgeGroup.querySelector('text');
        if (bCircle) {
          bCircle.setAttribute('cx', d.x + 12);
          bCircle.setAttribute('cy', d.y - 12);
        }
        if (bText) {
          bText.setAttribute('x', d.x + 12);
          bText.setAttribute('y', d.y - 12);
        }
      }

      const crossGroup = g.querySelector('.failure-cross-group');
      if (crossGroup) {
        const l1 = crossGroup.querySelector('.l1');
        const l2 = crossGroup.querySelector('.l2');
        if (l1 && l2) {
          l1.setAttribute('x1', d.x - 9);
          l1.setAttribute('y1', d.y - 9);
          l1.setAttribute('x2', d.x + 9);
          l1.setAttribute('y2', d.y + 9);
          l2.setAttribute('x1', d.x - 9);
          l2.setAttribute('y1', d.y + 9);
          l2.setAttribute('x2', d.x + 9);
          l2.setAttribute('y2', d.y - 9);
        }
      }
    });

    // Update overlays (Stop cost chips along shortest route)
    updateOverlays();

    // If inspector is open, update its screen anchor position
    if (state.selectedElement) {
      updateInspectorPosition();
    }
  }

  function updateOverlays() {
    if (!state.shortestPathResult || !state.shortestPathResult.reachable) return;
    const path = state.shortestPathResult.path || [];
    const nodeMap = new Map();
    state.devices.forEach(d => nodeMap.set(d.name, d));

    const chips = el.overlaysGroup.querySelectorAll('.stop-chip-overlay');
    chips.forEach(chip => {
      const name = chip.dataset.node;
      const d = nodeMap.get(name);
      if (!d) return;
      const bg = chip.querySelector('rect');
      const txt = chip.querySelector('text');
      bg.setAttribute('x', d.x - 14);
      bg.setAttribute('y', d.y + 17);
      txt.setAttribute('x', d.x);
      txt.setAttribute('y', d.y + 25);
    });
  }

  // --- Node Dragging & Drag-to-Link ---
  function setupNodeEvents(nodeG, device) {
    nodeG.addEventListener('mousedown', (e) => {
      if (e.button !== 0) return;
      e.stopPropagation();

      if (e.shiftKey) {
        // Shift + Drag for Drag-to-Link
        state.dragLinkSource = device;
        el.dragLinkPreview.style.display = 'block';
        el.dragLinkPreview.setAttribute('x1', device.x);
        el.dragLinkPreview.setAttribute('y1', device.y);
        el.dragLinkPreview.setAttribute('x2', device.x);
        el.dragLinkPreview.setAttribute('y2', device.y);
        return;
      }

      state.draggedNode = device;
      state.dragMoved = false;
      device.pinned = true;
    });

    nodeG.addEventListener('click', (e) => {
      e.stopPropagation();
      if (!state.dragMoved) {
        selectElement('node', device, e);
      }
    });
  }

  // --- Canvas Interaction (Pan, Zoom, Add Node, Delete) ---
  function setupCanvasEvents() {
    el.svg.addEventListener('mousedown', (e) => {
      if (e.button === 0 && (e.target === el.svg || e.target.classList.contains('grid-sheet-bg'))) {
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

    window.addEventListener('mouseup', () => {
      state.isPanning = false;

      if (state.draggedNode) {
        state.draggedNode = null;
      }

      if (state.dragLinkSource) {
        el.dragLinkPreview.style.display = 'none';
        const rect = el.svg.getBoundingClientRect();
        const mouseX = parseFloat(el.dragLinkPreview.getAttribute('x2'));
        const mouseY = parseFloat(el.dragLinkPreview.getAttribute('y2'));

        const targetNode = state.devices.find(d => {
          if (d.name === state.dragLinkSource.name) return false;
          return Math.hypot(d.x - mouseX, d.y - mouseY) <= 24;
        });

        if (targetNode) {
          promptAddLink(state.dragLinkSource.name, targetNode.name);
        }
        state.dragLinkSource = null;
      }
    });

    // Zoom via Mouse Wheel
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
      if (e.target !== el.svg && !e.target.classList.contains('grid-sheet-bg')) return;
      promptAddDevice();
    });

    // Keyboard Delete / Backspace
    window.addEventListener('keydown', (e) => {
      if ((e.key === 'Delete' || e.key === 'Backspace') && state.selectedElement) {
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
    if (state.selectedElement) {
      updateInspectorPosition();
    }
  }

  // --- Prompts for Canvas Gestures ---
  async function promptAddDevice() {
    const defaultName = `Station_${state.devices.length + 1}`;
    const name = window.prompt('Enter new station name (letters, numbers, _, -):', defaultName);
    if (!name) return;
    try {
      await apiFetch('/api/device', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name: name.trim() })
      });
      showToast(`Station '${name.trim()}' added`, 'success');
      await loadNetwork();
    } catch (err) { }
  }

  async function promptAddLink(nodeA, nodeB) {
    const costStr = window.prompt(`Connect station ${nodeA} to ${nodeB}. Enter cost (1 - 9999):`, '1');
    if (!costStr) return;
    const cost = parseInt(costStr, 10);
    if (isNaN(cost) || cost < 1 || cost > 9999) {
      showToast('Line cost must be an integer between 1 and 9999', 'error');
      return;
    }
    try {
      await apiFetch('/api/link', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ a: nodeA, b: nodeB, cost: cost })
      });
      showToast(`Line connected between ${nodeA} and ${nodeB}`, 'success');
      await loadNetwork();
    } catch (err) { }
  }

  // --- Floating Inspector Management ---
  function selectElement(type, data) {
    deselectElement();
    state.selectedElement = { type, data };

    el.floatingInspector.style.display = 'block';

    if (type === 'node') {
      const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${data.name}"]`);
      if (nodeG) {
        const disc = nodeG.querySelector('.station-disc');
        if (disc) disc.style.stroke = 'var(--accent-focus)';
      }

      el.inspectType.textContent = 'Station';
      el.inspectDetails.innerHTML = '';
      el.inspectDetails.appendChild(safeElem('div', { className: 'station-name', text: data.name }));
      el.inspectDetails.appendChild(safeElem('div', { className: 'detail-row' }, [
        safeElem('span', { text: 'Incident lines' }),
        safeElem('span', { text: String(data.degree) })
      ]));
      el.inspectDetails.appendChild(safeElem('div', { className: 'detail-row' }, [
        safeElem('span', { text: 'Network' }),
        safeElem('span', { text: state.componentColorMap[data.name] ? 'Connected' : 'Isolated' })
      ]));
      el.btnInspectorRemove.textContent = 'Remove station';

    } else if (type === 'edge') {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${data.a}"][data-b="${data.b}"], .graph-edge[data-a="${data.b}"][data-b="${data.a}"]`);
      if (edge) edge.style.stroke = 'var(--accent-focus)';

      const isBridge = state.bridges.some(b => (b.a === data.a && b.b === data.b) || (b.a === data.b && b.b === data.a));

      el.inspectType.textContent = 'Line';
      el.inspectDetails.innerHTML = '';
      el.inspectDetails.appendChild(safeElem('div', { className: 'station-name', text: `${data.a} ↔ ${data.b}` }));
      el.inspectDetails.appendChild(safeElem('div', { className: 'detail-row' }, [
        safeElem('span', { text: 'Cost' }),
        safeElem('span', { text: String(data.cost) })
      ]));
      el.inspectDetails.appendChild(safeElem('div', { className: 'detail-row' }, [
        safeElem('span', { text: 'Single track' }),
        safeElem('span', { text: isBridge ? 'Yes (Bridge)' : 'No' })
      ]));
      el.btnInspectorRemove.textContent = 'Remove line';
    }

    updateInspectorPosition();
  }

  function updateInspectorPosition() {
    if (!state.selectedElement) return;
    const { type, data } = state.selectedElement;

    let worldX = 0, worldY = 0;
    if (type === 'node') {
      const node = state.devices.find(d => d.name === data.name);
      if (!node) return;
      worldX = node.x;
      worldY = node.y;
    } else {
      const u = state.devices.find(d => d.name === data.a);
      const v = state.devices.find(d => d.name === data.b);
      if (!u || !v) return;
      worldX = (u.x + v.x) / 2;
      worldY = (u.y + v.y) / 2;
    }

    // Convert world to screen space inside canvas container
    const screenX = state.panX + worldX * state.zoom + 22;
    const screenY = state.panY + worldY * state.zoom - 20;

    const areaRect = el.canvasArea.getBoundingClientRect();
    const inspW = 210;
    const inspH = 130;

    const clampedX = Math.max(10, Math.min(areaRect.width - inspW - 10, screenX));
    const clampedY = Math.max(10, Math.min(areaRect.height - inspH - 10, screenY));

    el.floatingInspector.style.left = `${clampedX}px`;
    el.floatingInspector.style.top = `${clampedY}px`;
  }

  function deselectElement() {
    state.selectedElement = null;
    el.floatingInspector.style.display = 'none';

    // Reset node strokes
    const nodes = el.nodesGroup.querySelectorAll('.graph-node');
    nodes.forEach(n => {
      const disc = n.querySelector('.station-disc');
      if (disc) disc.style.stroke = '';
    });

    // Reset edge strokes
    applyComponentColors();
  }

  async function deleteSelectedElement() {
    if (!state.selectedElement) return;
    const { type, data } = state.selectedElement;
    if (type === 'node') {
      try {
        await apiFetch(`/api/device/${encodeURIComponent(data.name)}`, { method: 'DELETE' });
        showToast(`Station '${data.name}' removed`, 'success');
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
        showToast(`Line ${data.a} to ${data.b} removed`, 'success');
        deselectElement();
        await loadNetwork();
      } catch (err) { }
    }
  }

  el.btnCloseInspector.addEventListener('click', deselectElement);
  el.btnInspectorRemove.addEventListener('click', deleteSelectedElement);

  // --- Task Rail & Drawer Accordion Navigation ---
  const taskTitles = {
    build: 'Build network',
    explore: 'Explore traversal & reachability',
    route: 'Route: Shortest path',
    faults: 'Weak points & vulnerability',
    simulation: 'Failure simulation'
  };

  function setActiveTask(taskId) {
    if (state.activeTask === taskId && state.drawerOpen) {
      // Toggle closed if clicking active rail item
      setDrawerOpen(false);
      return;
    }

    state.activeTask = taskId;
    setDrawerOpen(true);

    el.railItems.forEach(item => {
      if (item.dataset.task === taskId) {
        item.classList.add('active');
      } else {
        item.classList.remove('active');
      }
    });

    el.drawerTitle.textContent = taskTitles[taskId] || 'Task';

    el.taskPanels.forEach(panel => {
      if (panel.dataset.panel === taskId) {
        panel.classList.add('active');
      } else {
        panel.classList.remove('active');
      }
    });
  }

  function setDrawerOpen(open) {
    state.drawerOpen = open;
    if (open) {
      el.taskDrawer.classList.remove('closed');
    } else {
      el.taskDrawer.classList.add('closed');
      el.railItems.forEach(item => item.classList.remove('active'));
    }
  }

  el.railItems.forEach(item => {
    item.addEventListener('click', () => {
      const taskId = item.dataset.task;
      setActiveTask(taskId);
    });
  });

  el.btnCloseDrawer.addEventListener('click', () => {
    setDrawerOpen(false);
  });

  // --- Bottom Dock Collapse Toggle ---
  el.btnDockToggle.addEventListener('click', () => {
    const isCollapsed = el.bottomDock.classList.toggle('collapsed');
    document.body.classList.toggle('dock-collapsed', isCollapsed);
  });

  // --- Transit Legend Toggle ---
  el.btnLegendToggle.addEventListener('click', () => {
    el.transitLegend.classList.toggle('collapsed');
  });

  // --- Feature 1: Build Network Actions ---
  el.btnAddDevice.addEventListener('click', async () => {
    const name = el.newDeviceName.value.trim();
    if (!name) {
      showToast('Enter a station name', 'error');
      return;
    }
    try {
      await apiFetch('/api/device', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name: name })
      });
      showToast(`Station '${name}' added`, 'success');
      el.newDeviceName.value = '';
      await loadNetwork();
    } catch (err) { }
  });

  el.btnAddLink.addEventListener('click', async () => {
    const a = el.linkNodeA.value;
    const b = el.linkNodeB.value;
    const cost = parseInt(el.linkCost.value, 10);
    if (!a || !b) {
      showToast('Select both stations to connect', 'error');
      return;
    }
    if (a === b) {
      showToast('Self-loops are not permitted. Connect two distinct stations.', 'error');
      return;
    }
    if (isNaN(cost) || cost < 1 || cost > 9999) {
      showToast('Cost must be an integer between 1 and 9999', 'error');
      return;
    }
    try {
      await apiFetch('/api/link', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ a, b, cost })
      });
      showToast(`Line connected between ${a} and ${b}`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnRemoveDevice.addEventListener('click', async () => {
    const name = el.removeDeviceSelect.value;
    if (!name) return;
    try {
      await apiFetch(`/api/device/${encodeURIComponent(name)}`, { method: 'DELETE' });
      showToast(`Station '${name}' removed`, 'success');
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
      showToast(`Line ${a} to ${b} removed`, 'success');
      await loadNetwork();
    } catch (err) { }
  });

  el.btnLoadSample.addEventListener('click', async () => {
    const sample = el.sampleSelect.value;
    if (!sample) return;
    try {
      await apiFetch(`/api/load/${encodeURIComponent(sample)}`, { method: 'POST' });
      showToast(`Topology sample '${sample}' loaded`, 'success');
      resetHighlights();
      await loadNetwork();
    } catch (err) { }
  });

  el.btnEmptyLoadSample.addEventListener('click', () => {
    el.sampleSelect.value = 'sample_small.txt';
    el.btnLoadSample.click();
  });

  el.btnClearNetwork.addEventListener('click', async () => {
    if (!window.confirm('Clear all stations and lines from the current network?')) return;
    const names = state.devices.map(d => d.name);
    for (const name of names) {
      try {
        await apiFetch(`/api/device/${encodeURIComponent(name)}`, { method: 'DELETE' });
      } catch (e) { }
    }
    deselectElement();
    resetHighlights();
    showToast('Network cleared', 'info');
    await loadNetwork();
  });

  el.btnExport.addEventListener('click', async () => {
    try {
      const text = await apiFetch('/api/export');
      const blob = new Blob([text], { type: 'text/plain;charset=utf-8' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `network_topology_${Date.now()}.txt`;
      a.click();
      URL.revokeObjectURL(url);
      showToast('Network topology exported', 'info');
    } catch (err) { }
  });

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
      showToast('Network topology imported successfully', 'success');
      el.modalImport.classList.add('hidden');
      resetHighlights();
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

    // Reset previous current beacon rings
    el.nodesGroup.querySelectorAll('.graph-node').forEach(n => {
      const beacon = n.querySelector('.current-beacon-ring');
      if (beacon) beacon.remove();
    });

    // Mark current node
    const currNode = el.nodesGroup.querySelector(`.graph-node[data-name="${step.visit}"]`);
    if (currNode) {
      currNode.classList.add('is-visited');

      // Add accent beacon ring to current station
      const beacon = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      beacon.setAttribute('class', 'current-beacon-ring');
      beacon.setAttribute('cx', currNode.querySelector('.station-disc').getAttribute('cx'));
      beacon.setAttribute('cy', currNode.querySelector('.station-disc').getAttribute('cy'));
      beacon.setAttribute('r', '20');
      currNode.appendChild(beacon);
    }

    // Mark nodes waiting in queue / stack with order badge
    const queueOrStack = state.traversalType === 'bfs' ? (step.queue || []) : (step.stack || []);
    queueOrStack.forEach((name, pos) => {
      const qNode = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
      if (qNode && !qNode.classList.contains('is-visited')) {
        let badgeGroup = qNode.querySelector('.queue-badge-group');
        if (!badgeGroup) {
          badgeGroup = document.createElementNS('http://www.w3.org/2000/svg', 'g');
          badgeGroup.setAttribute('class', 'queue-badge-group');
          const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
          circle.setAttribute('class', 'queue-pos-badge');
          circle.setAttribute('r', '7');
          const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
          text.setAttribute('class', 'queue-pos-text');
          badgeGroup.appendChild(circle);
          badgeGroup.appendChild(text);
          qNode.appendChild(badgeGroup);
        }
        badgeGroup.querySelector('text').textContent = String(pos + 1);
      }
    });

    // Render Ticket-style Queue/Stack Row in Dock
    el.structureTypeLabel.textContent = state.traversalType === 'bfs' ? 'Queue (FIFO)' : 'Stack (LIFO)';
    el.structureMetaText.textContent = `Step ${index + 1} of ${state.traversalSteps.length}`;
    el.structureItemsContainer.innerHTML = '';

    if (queueOrStack.length === 0) {
      el.structureItemsContainer.appendChild(safeElem('span', {
        className: 'empty-ticket-hint',
        text: 'Queue / Stack is empty'
      }));
    } else {
      queueOrStack.forEach((item, pos) => {
        const isNextOut = (state.traversalType === 'bfs' && pos === 0) || (state.traversalType === 'dfs' && pos === queueOrStack.length - 1);
        const ticket = safeElem('div', {
          className: `structure-ticket ${isNextOut ? 'next-out' : ''}`,
          text: item
        });
        el.structureItemsContainer.appendChild(ticket);
      });
    }

    logEvent(`${state.traversalType.toUpperCase()} step ${index + 1}: Visited ${step.visit}${step.from ? ` from ${step.from}` : ' (origin)'}`);
  }

  el.btnTraversalPlay.addEventListener('click', async () => {
    if (!state.traversalActive) {
      const typeRadio = document.querySelector('input[name="traversal-type"]:checked');
      state.traversalType = typeRadio ? typeRadio.value : 'bfs';
      const start = el.traversalStart.value;
      if (!start) {
        showToast('Select a starting station for traversal', 'error');
        return;
      }
      try {
        const res = await fetchTraversal(state.traversalType, start);
        resetHighlights();
        state.traversalActive = true;
        state.traversalSteps = res.steps || [];
        state.traversalStepIndex = -1;
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
        el.algoStatusBadge.textContent = 'Completed';
        el.algoIndicatorDot.style.backgroundColor = 'var(--accent-success)';
        el.algoStatusText.textContent = 'Finished';
      }
    }
    clearTimeout(state.traversalTimer);
    tick();
  });

  el.btnTraversalPause.addEventListener('click', () => {
    clearTimeout(state.traversalTimer);
    el.algoStatusText.textContent = 'Paused';
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
    logEvent('Traversal reset');
  });

  function displayTraversalSummary(type, res) {
    el.tabResults.click();
    el.algoTitle.textContent = `${type.toUpperCase()} traversal result`;
    el.algoStatusBadge.textContent = 'In progress';
    el.algoStatusBadge.style.display = 'inline-block';
    el.algoIndicatorDot.style.backgroundColor = 'var(--accent-focus)';
    el.algoStatusText.textContent = 'Animating';

    el.algoDetailsBody.innerHTML = '';
    const visitRow = safeElem('div', { className: 'route-stop-chain' });
    (res.order || []).forEach((name, i) => {
      visitRow.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
        safeElem('span', { text: `${i + 1}. ${name}` })
      ]));
      if (i < res.order.length - 1) {
        visitRow.appendChild(safeElem('span', { style: 'color:var(--color-ink-subtle);', text: '→' }));
      }
    });

    el.algoDetailsBody.appendChild(safeElem('div', {
      style: 'font-size:12px; font-weight:600; margin-bottom:4px;',
      text: `Visit sequence (${(res.order || []).length} stations):`
    }));
    el.algoDetailsBody.appendChild(visitRow);

    if (res.levels) {
      const levelRow = safeElem('div', { className: 'route-stop-chain', style: 'margin-top:8px;' });
      for (const [st, lvl] of Object.entries(res.levels)) {
        levelRow.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
          safeElem('span', { text: st }),
          safeElem('span', { className: 'route-stop-cost', text: `lvl ${lvl}` })
        ]));
      }
      el.algoDetailsBody.appendChild(safeElem('div', {
        style: 'font-size:12px; font-weight:600; margin-top:8px; margin-bottom:4px;',
        text: 'Hop levels:'
      }));
      el.algoDetailsBody.appendChild(levelRow);
    }
  }

  // --- Feature 3: Reachability Verification ---
  el.btnCheckConnect.addEventListener('click', async () => {
    const x = el.connectX.value;
    const y = el.connectY.value;
    if (!x || !y) {
      showToast('Select two stations to check reachability', 'error');
      return;
    }
    try {
      const res = await apiFetch('/api/analyze/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ x, y })
      });

      el.tabResults.click();
      el.algoTitle.textContent = `Reachability: ${x} to ${y}`;
      el.algoStatusBadge.style.display = 'inline-block';

      const isConn = res.bfs;
      el.algoStatusBadge.textContent = isConn ? 'Reachable' : 'Disconnected';
      el.algoIndicatorDot.style.backgroundColor = isConn ? 'var(--accent-success)' : 'var(--accent-danger)';
      el.algoStatusText.textContent = isConn ? 'Connected' : 'Separated';

      el.algoDetailsBody.innerHTML = '';
      const consensusText = isConn
        ? `Stations ${x} and ${y} are in the same network and can reach each other.`
        : `Stations ${x} and ${y} are in separate disconnected networks.`;

      el.algoDetailsBody.appendChild(safeElem('p', {
        style: 'font-size:13px; font-weight:500; margin-bottom:8px;',
        text: consensusText
      }));

      const checkList = safeElem('div', { className: 'route-stop-chain' });
      checkList.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
        safeElem('span', { text: `BFS: ${res.bfs ? 'Connected' : 'Not reachable'}` })
      ]));
      checkList.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
        safeElem('span', { text: `DFS: ${res.dfs ? 'Connected' : 'Not reachable'}` })
      ]));
      checkList.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
        safeElem('span', { text: `Union-Find: ${res.unionfind ? 'Connected' : 'Not reachable'}` })
      ]));
      el.algoDetailsBody.appendChild(checkList);

      logEvent(`Reachability ${x} to ${y}: ${isConn ? 'REACHABLE' : 'UNREACHABLE'} (BFS, DFS, UF all agreed).`);
    } catch (err) { }
  });

  el.btnColorComponents.addEventListener('click', () => {
    resetHighlights();
    showToast('Reset component lines', 'info');
  });

  // --- Feature 4: Shortest Route (Dijkstra) ---
  el.btnFindPath.addEventListener('click', async () => {
    const src = el.pathSrc.value;
    const dst = el.pathDst.value;
    if (!src || !dst) {
      showToast('Select origin and destination stations', 'error');
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
      showToast('Select origin and destination stations', 'error');
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

      el.tabResults.click();
      el.algoTitle.textContent = `Dijkstra: ${src} to ${dst}`;
      el.algoStatusBadge.textContent = 'Settling stations';
      el.algoStatusBadge.style.display = 'inline-block';
      el.algoIndicatorDot.style.backgroundColor = 'var(--accent-focus)';
      el.algoStatusText.textContent = 'Calculating route';

      function tick() {
        if (!state.dijkstraActive) return;
        if (state.dijkstraStepIndex + 1 < state.dijkstraSteps.length) {
          state.dijkstraStepIndex++;
          const step = state.dijkstraSteps[state.dijkstraStepIndex];

          const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${step.settled}"]`);
          if (nodeG) {
            nodeG.classList.add('is-visited');
          }
          state.dijkstraTimer = setTimeout(tick, state.traversalSpeed);
        } else {
          renderShortestPathResult(src, dst, res, false);
          showToast(`Destination reached with total cost ${res.cost}`, 'success');
        }
      }
      tick();
    } catch (err) { }
  });

  function renderShortestPathResult(src, dst, res, reset = true) {
    if (reset) resetHighlights();
    state.shortestPathResult = res;

    el.tabResults.click();
    el.algoTitle.textContent = `Shortest route: ${src} to ${dst}`;
    el.algoStatusBadge.style.display = 'inline-block';

    if (!res.reachable) {
      el.algoStatusBadge.textContent = 'Unreachable';
      el.algoIndicatorDot.style.backgroundColor = 'var(--accent-danger)';
      el.algoStatusText.textContent = 'Unreachable';
      el.algoDetailsBody.innerHTML = '';
      el.algoDetailsBody.appendChild(safeElem('p', {
        style: 'font-size:13px; color:var(--accent-danger);',
        text: `Destination '${dst}' is unreachable from '${src}'. No line exists between their networks.`
      }));
      logEvent(`Route ${src} to ${dst}: UNREACHABLE`, 'warn');
      return;
    }

    el.algoStatusBadge.textContent = `Cost ${res.cost}`;
    el.algoIndicatorDot.style.backgroundColor = 'var(--accent-success)';
    el.algoStatusText.textContent = 'Route found';

    // Dim all lines to 22% opacity
    el.edgesGroup.querySelectorAll('.graph-edge').forEach(e => e.classList.add('dimmed'));

    // Highlight route edges in bold solid ink
    (res.edges || []).forEach(e => {
      const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${e.a}"][data-b="${e.b}"], .graph-edge[data-a="${e.b}"][data-b="${e.a}"]`);
      if (edge) {
        edge.classList.remove('dimmed');
        edge.classList.add('is-route');
      }
    });

    // Render cumulative cost stop markers along route
    el.overlaysGroup.innerHTML = '';
    const path = res.path || [];
    let cumCost = 0;
    const costMap = new Map();
    costMap.set(src, 0);

    for (let i = 0; i < path.length - 1; i++) {
      const u = path[i];
      const v = path[i + 1];
      const edge = (res.edges || []).find(e => (e.a === u && e.b === v) || (e.a === v && e.b === u));
      cumCost += edge ? edge.cost : 0;
      costMap.set(v, cumCost);
    }

    path.forEach(name => {
      const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
      if (nodeG) nodeG.classList.add('is-visited');

      const d = state.devices.find(dev => dev.name === name);
      if (d) {
        const overlay = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        overlay.setAttribute('class', 'stop-chip-overlay');
        overlay.dataset.node = name;

        const bg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        bg.setAttribute('class', 'stop-cost-badge-bg');
        bg.setAttribute('width', '28');
        bg.setAttribute('height', '15');
        bg.setAttribute('x', d.x - 14);
        bg.setAttribute('y', d.y + 17);

        const txt = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        txt.setAttribute('class', 'stop-cost-badge-text');
        txt.setAttribute('x', d.x);
        txt.setAttribute('y', d.y + 25);
        txt.textContent = String(costMap.get(name) || 0);

        overlay.appendChild(bg);
        overlay.appendChild(txt);
        el.overlaysGroup.appendChild(overlay);
      }
    });

    // Show Hero Total Cost (28px) in Dock
    el.algoDetailsBody.innerHTML = '';
    const heroBox = safeElem('div', { className: 'route-hero-total' }, [
      safeElem('span', { className: 'route-hero-num', text: String(res.cost) }),
      safeElem('span', { className: 'route-hero-label', text: `total cost across ${path.length - 1} stops` })
    ]);
    el.algoDetailsBody.appendChild(heroBox);

    // Stop Sequence Chain
    const chainBox = safeElem('div', { className: 'route-stop-chain' });
    path.forEach((name, i) => {
      chainBox.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
        safeElem('span', { text: name }),
        safeElem('span', { className: 'route-stop-cost', text: `(${costMap.get(name)})` })
      ]));
      if (i < path.length - 1) {
        chainBox.appendChild(safeElem('span', { style: 'color:var(--color-ink-subtle);', text: '→' }));
      }
    });
    el.algoDetailsBody.appendChild(chainBox);

    logEvent(`Route ${src} to ${dst}: ${path.join(' → ')} (Total cost: ${res.cost})`, 'success');
  }

  // --- Feature 5: Weak Points (Bridges & Articulation Points) ---
  el.btnFindFaults.addEventListener('click', async () => {
    try {
      const res = await apiFetch('/api/analyze/faults');
      resetHighlights();

      state.bridges = res.bridges || [];
      state.articulationPoints = res.articulation || [];

      // Highlight single track bridges with perpendicular crossbar ticks
      state.bridges.forEach(b => {
        const edgeGroup = el.edgesGroup.querySelector(`.graph-edge-group[data-a="${b.a}"][data-b="${b.b}"], .graph-edge-group[data-a="${b.b}"][data-b="${b.a}"]`);
        if (edgeGroup) {
          const line = edgeGroup.querySelector('line.graph-edge');
          if (line) line.classList.add('is-bridge');

          let crossbar = edgeGroup.querySelector('.bridge-crossbar');
          if (!crossbar) {
            crossbar = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            crossbar.setAttribute('class', 'bridge-crossbar');
            edgeGroup.appendChild(crossbar);
          }
        }
      });

      // Highlight critical stations (interchange double rings)
      state.articulationPoints.forEach(name => {
        const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
        if (nodeG) {
          let ring = nodeG.querySelector('.interchange-outer-ring');
          if (!ring) {
            ring = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            ring.setAttribute('class', 'interchange-outer-ring');
            ring.setAttribute('r', '24');
            nodeG.insertBefore(ring, nodeG.firstChild);
          }
        }
      });
      updateGraphPositions();

      el.tabResults.click();
      el.algoTitle.textContent = 'Weak points detected';
      el.algoStatusBadge.textContent = `${state.bridges.length} bridges, ${state.articulationPoints.length} cut vertices`;
      el.algoStatusBadge.style.display = 'inline-block';
      el.algoIndicatorDot.style.backgroundColor = (state.bridges.length > 0 || state.articulationPoints.length > 0)
        ? 'var(--accent-danger)' : 'var(--accent-success)';
      el.algoStatusText.textContent = 'Vulnerabilities';

      el.algoDetailsBody.innerHTML = '';

      // Bridges list
      el.algoDetailsBody.appendChild(safeElem('div', {
        style: 'font-size:12px; font-weight:600; margin-bottom:4px;',
        text: `Single track bridges (${state.bridges.length}):`
      }));
      const bridgeRow = safeElem('div', { className: 'weak-chip-list' });
      if (state.bridges.length === 0) {
        bridgeRow.appendChild(safeElem('span', { className: 'empty-ticket-hint', text: 'No bridges found (all paths redundant)' }));
      } else {
        state.bridges.forEach(b => {
          bridgeRow.appendChild(safeElem('div', {
            className: 'weak-chip',
            onClick: () => focusLink(b.a, b.b)
          }, [
            safeElem('span', { text: `${b.a} to ${b.b}` })
          ]));
        });
      }
      el.algoDetailsBody.appendChild(bridgeRow);

      // Articulation points list
      el.algoDetailsBody.appendChild(safeElem('div', {
        style: 'font-size:12px; font-weight:600; margin-top:10px; margin-bottom:4px;',
        text: `Interchange critical stations (${state.articulationPoints.length}):`
      }));
      const apRow = safeElem('div', { className: 'weak-chip-list' });
      if (state.articulationPoints.length === 0) {
        apRow.appendChild(safeElem('span', { className: 'empty-ticket-hint', text: 'No critical stations found (biconnected network)' }));
      } else {
        state.articulationPoints.forEach(name => {
          apRow.appendChild(safeElem('div', {
            className: 'weak-chip',
            onClick: () => focusNode(name)
          }, [
            safeElem('span', { text: name })
          ]));
        });
      }
      el.algoDetailsBody.appendChild(apRow);

      logEvent(`Found ${state.bridges.length} bridge(s) and ${state.articulationPoints.length} critical station(s).`, 'warn');
    } catch (err) { }
  });

  function focusNode(name) {
    const node = state.devices.find(d => d.name === name);
    if (!node) return;
    selectElement('node', node);
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
        showToast('Select a line to simulate closure', 'error');
        return;
      }
      const [a, b] = val.split(':');
      payload.a = a;
      payload.b = b;
    } else {
      const name = el.simDeviceSelect.value;
      if (!name) {
        showToast('Select a station to simulate closure', 'error');
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

      // Mark target as closed
      if (simType === 'link') {
        const edge = el.edgesGroup.querySelector(`.graph-edge[data-a="${payload.a}"][data-b="${payload.b}"], .graph-edge[data-a="${payload.b}"][data-b="${payload.a}"]`);
        if (edge) edge.classList.add('is-failed');
      } else {
        const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${payload.name}"]`);
        if (nodeG) {
          nodeG.classList.add('is-failed');
          let cross = nodeG.querySelector('.failure-cross-group');
          if (!cross) {
            cross = document.createElementNS('http://www.w3.org/2000/svg', 'g');
            cross.setAttribute('class', 'failure-cross-group');
            const l1 = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            l1.setAttribute('class', 'failure-cross l1');
            const l2 = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            l2.setAttribute('class', 'failure-cross l2');
            cross.appendChild(l1);
            cross.appendChild(l2);
            nodeG.appendChild(cross);
          }
        }
      }

      // Re-color newly separated subnets with distinct line colors
      const groups = res.disconnected || [];
      groups.forEach((group, idx) => {
        const color = getComponentColor(idx);
        group.forEach(name => {
          state.componentColorMap[name] = color;
          const nodeG = el.nodesGroup.querySelector(`.graph-node[data-name="${name}"]`);
          if (nodeG && (!payload.name || name !== payload.name)) {
            const disc = nodeG.querySelector('.station-disc');
            if (disc) disc.style.stroke = color;
          }
        });
      });
      applyComponentColors();
      updateGraphPositions();

      el.tabResults.click();
      el.algoTitle.textContent = `Simulation: ${simType === 'link' ? `Line ${payload.a} to ${payload.b}` : `Station ${payload.name}`}`;
      el.algoStatusBadge.textContent = `${res.componentsBefore} → ${res.componentsAfter} networks`;
      el.algoStatusBadge.style.display = 'inline-block';
      el.algoIndicatorDot.style.backgroundColor = res.componentsAfter > res.componentsBefore
        ? 'var(--accent-danger)' : 'var(--accent-success)';
      el.algoStatusText.textContent = res.componentsAfter > res.componentsBefore ? 'Partitioned' : 'Intact';

      el.algoDetailsBody.innerHTML = '';
      const splitMsg = res.componentsAfter > res.componentsBefore
        ? 'Network partitioned into multiple isolated subnets.'
        : 'Network remains connected via alternate routes.';
      el.algoDetailsBody.appendChild(safeElem('p', {
        style: `font-size:13px; font-weight:600; margin-bottom:8px; color:${res.componentsAfter > res.componentsBefore ? 'var(--accent-danger)' : 'var(--accent-success)'};`,
        text: splitMsg
      }));

      // Render resulting partition subnet chips
      const subnetList = safeElem('div', { className: 'weak-chip-list' });
      groups.forEach((group, idx) => {
        subnetList.appendChild(safeElem('div', { className: 'route-stop-chip' }, [
          safeElem('span', { text: `Network ${idx + 1}: ${group.join(', ')}` })
        ]));
      });
      el.algoDetailsBody.appendChild(subnetList);

      logEvent(`Failure simulation complete: networks ${res.componentsBefore} → ${res.componentsAfter}.`, 'warn');
    } catch (err) { }
  });

  el.btnRestoreNetwork.addEventListener('click', () => {
    resetHighlights();
    showToast('Network restored to normal operational state', 'info');
    logEvent('Restored failure simulation');
  });

  // --- Feature 7: Plain-Text Report ---
  el.btnOpenReport.addEventListener('click', async () => {
    el.reportContent.textContent = 'Generating comprehensive analysis report...';
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

  // --- Canvas Floating Tools ---
  el.btnRelayout.addEventListener('click', () => {
    const width = el.svg.clientWidth || 800;
    const height = el.svg.clientHeight || 600;
    state.devices.forEach((d, i) => {
      d.pinned = false;
      const angle = (i / Math.max(1, state.devices.length)) * 2 * Math.PI;
      const r = Math.min(width, height) * 0.32;
      d.x = width / 2 + r * Math.cos(angle) + (Math.random() - 0.5) * 40;
      d.y = height / 2 + r * Math.sin(angle) + (Math.random() - 0.5) * 40;
      d.vx = 0;
      d.vy = 0;
    });
    startSimulation();
    showToast('Re-laying out network', 'info');
  });

  function autoFitGraph() {
    if (state.devices.length === 0) return;
    let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
    state.devices.forEach(d => {
      if (d.x < minX) minX = d.x;
      if (d.x > maxX) maxX = d.x;
      if (d.y < minY) minY = d.y;
      if (d.y > maxY) maxY = d.y;
    });

    const pad = 100;
    const bboxW = Math.max(140, (maxX - minX) + pad * 2);
    const bboxH = Math.max(140, (maxY - minY) + pad * 2);
    const rect = el.canvasArea.getBoundingClientRect();
    const svgW = rect.width || 800;
    const svgH = rect.height || 600;

    const scale = Math.min(1.5, Math.max(0.65, Math.min(svgW / bboxW, svgH / bboxH)));
    state.zoom = scale;
    state.panX = svgW / 2 - ((minX + maxX) / 2) * scale;
    state.panY = svgH / 2 - ((minY + maxY) / 2) * scale;
    applyTransform();
  }

  el.btnFitScreen.addEventListener('click', autoFitGraph);

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

  // --- Bottom Dock Tabs ---
  el.tabResults.addEventListener('click', () => {
    el.tabResults.classList.add('active');
    el.tabResults.setAttribute('aria-selected', 'true');
    el.tabLogs.classList.remove('active');
    el.tabLogs.setAttribute('aria-selected', 'false');
    el.contentResults.classList.add('active');
    el.contentResults.classList.remove('hidden');
    el.contentLogs.classList.add('hidden');
    el.contentLogs.classList.remove('active');
  });

  el.tabLogs.addEventListener('click', () => {
    el.tabLogs.classList.add('active');
    el.tabLogs.setAttribute('aria-selected', 'true');
    el.tabResults.classList.remove('active');
    el.tabResults.setAttribute('aria-selected', 'false');
    el.contentLogs.classList.add('active');
    el.contentLogs.classList.remove('hidden');
    el.contentResults.classList.add('hidden');
    el.contentResults.classList.remove('active');
  });

  el.btnClearLogs.addEventListener('click', () => {
    el.logStream.innerHTML = '';
  });

  // --- Light / Dark Theme ---
  function applyTheme(theme) {
    state.theme = theme;
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);

    // Update Sun / Moon Icon
    if (theme === 'dark') {
      el.themeIcon.innerHTML = `
        <svg class="icon-moon" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
          <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path>
        </svg>`;
    } else {
      el.themeIcon.innerHTML = `
        <svg class="icon-sun" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
          <circle cx="12" cy="12" r="5"></circle>
          <line x1="12" y1="1" x2="12" y2="3"></line>
          <line x1="12" y1="21" x2="12" y2="23"></line>
          <line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line>
          <line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line>
          <line x1="1" y1="12" x2="3" y2="12"></line>
          <line x1="21" y1="12" x2="23" y2="12"></line>
          <line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line>
          <line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line>
        </svg>`;
    }

    // Refresh component colors on lines and nodes
    if (state.componentList.length > 0) {
      state.componentColorMap = {};
      state.componentList.forEach((group, idx) => {
        const color = getComponentColor(idx);
        group.forEach(name => {
          state.componentColorMap[name] = color;
        });
      });
      applyComponentColors();
    }
  }

  el.btnThemeToggle.addEventListener('click', () => {
    applyTheme(state.theme === 'dark' ? 'light' : 'dark');
  });

  // --- App Initialization ---
  function init() {
    // Light theme is the primary default
    applyTheme('light');
    setupCanvasEvents();
    loadNetwork();
    logEvent('Network Connectivity Analyzer initialized.', 'info');
  }

  window.addEventListener('DOMContentLoaded', init);

})();
