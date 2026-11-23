# Design Notes: The Transit Map Redesign

## 1. Design Tokens

### 1.1 Color Palette
The color scheme shifts away from generic dark navy cards to an engineering drawing sheet metaphor. The default is a crisp light sheet, paired with an intentionally tuned dark variant for low-light environments.

| Token | Light Theme (Default) | Dark Theme (Tuned Alternate) | Purpose |
|---|---|---|---|
| `--bg-sheet` | `#F1F2EE` (cool chalk) | `#16191E` (deep neutral ink) | Main canvas and application background |
| `--bg-panel` | `#FAFAF7` (pure paper) | `#1D2128` (elevated dark surface) | Task drawer, dock, inspector |
| `--bg-panel-hover` | `#ECEEE8` | `#252A33` | Interactive row hovers, subtle highlights |
| `--color-ink` | `#14181F` (deep carbon) | `#ECEFF4` (clean crisp text) | Primary typography, borders, station outlines |
| `--color-ink-muted`| `#606773` | `#8E95A2` | Secondary labels, captions, metadata |
| `--rule-hairline` | `#C8CBC4` (drafting rule) | `#2E3440` (subtle hairline) | Borders, splitters, table dividers |
| `--rule-halo` | `#F1F2EE` | `#16191E` | Label halo stroke (`paint-order: stroke fill`) |
| `--accent-danger` | `#D6342C` (rail warning red) | `#E04B43` | Critical warnings, destructive actions, closed lines |
| `--accent-focus` | `#1F5FBF` (signal blue) | `#5A99E6` | 2px accessible focus ring |

### 1.2 Component Line Colors (Transit Lines)
Each connected component receives a distinct transit line color. To ensure full accessibility and 3:1 contrast against both light and dark sheets without relying on color alone, each line is also differentiated by line pattern or badge:

- **Line 1 (Alpha)**: `#1F5FBF` (Blue) — solid
- **Line 2 (Beta)**: `#D6342C` (Red) — dash pattern `[12, 4]`
- **Line 3 (Gamma)**: `#1E9A5A` (Green) — dash pattern `[8, 4, 2, 4]`
- **Line 4 (Delta)**: `#D98E04` (Amber) — dash pattern `[16, 4]`
- **Line 5 (Epsilon)**: `#7A4FB5` (Violet) — dash pattern `[6, 4]`
- **Line 6 (Zeta)**: `#0E8A94` (Teal) — dash pattern `[10, 3, 2, 3]`

### 1.3 Typography Scale & Font
- **Primary Font Family**: `Archivo`, `"Helvetica Neue"`, Arial, sans-serif
- **Self-Hosted Font**: `web/fonts/archivo-variable.woff2` (rendered with `@font-face` and `font-display: swap`)
- **Number Treatment**: `font-variant-numeric: tabular-nums` for all metrics, costs, coordinates, degrees, and timers
- **Type Scale**:
  - `28px` / 1.2 / 700: Hero numbers (e.g., Shortest route total cost)
  - `20px` / 1.3 / 600: Panel section titles, dialog headers
  - `16px` / 1.4 / 600: Station labels, major button labels, tab titles
  - `14px` / 1.4 / 400 & 500: Primary UI body, form inputs, table rows, station canvas labels
  - `13px` / 1.4 / 400: Metadata, helper text, legend descriptions, algorithm step entries
  - `12px` / 1.2 / 500 (tabular): Canvas edge costs, stop distance badges
- **Case Rule**: Strict sentence case across all labels, buttons, headers, and toasts. No ALL CAPS tracking.

### 1.4 Spacing & Geometry
- **Border Radii**:
  - `4px`: Buttons, text fields, select dropdowns, floating inspector cards, result tags
  - `0px`: Application frame, top bar, vertical task rail, task panel, bottom dock
  - `50%` (Circle): Transit stations, stop badges, icon roundels
- **Depths & Elevation**: Flat drafting style with hairline borders (`1px solid var(--rule-hairline)`). No heavy box shadows; only a subtle, crisp 2px elevation for floating popovers (`box-shadow: 0 2px 6px rgba(0,0,0,0.12)`).

---

## 2. Layout Structure & Wireframe

```
+-------------------------------------------------------------------------------------------------------------------------+
| [O=O Network Analyzer]                             [ 4 stations, 3 lines, 1 network ]   [Report (Text)]   [Theme: Light] | (48px Top Bar)
+----+----------------------------+---------------------------------------------------------------------------------------+
|    |                            | [Zoom +] [Zoom -] [Fit] [Re-layout]                              (Floating Tools TR)  |
| B  | TASK PANEL (320px)         |                                                                                       |
| U  | Single task active at once |                     (A)===============(B)                                             |
| I  |                            |                     //                 \\                                             |
| L  | [Add device]               |                    //                   \\                                            |
| D  | [Connect devices]          |                  (C)                     (D)                                          |
|    | [Load sample...]           |                                                                                       |
| E  | [Import / Export]          |                                                                                       |
| X  | [Clear network]            |                                                                                       |
| P  |                            |                                                                                       |
| L  |                            |                                                                                       |
| .  |                            |   +------------------------------------+                                              |
|    |                            |   | LEGEND (Transit Signs - Collapsed) |                                              |
| R  |                            |   | (O) Station   ((O)) Interchange    |                                              |
| T  |                            |   | -+- Single track   (.) Out of svc  |                                              |
|    |                            |   +------------------------------------+                                              |
+----+----------------------------+---------------------------------------------------------------------------------------+
| BOTTOM DOCK (~170px collapsible)                                                            [Results Tab] [Log Tab] [-] |
| ----------------------------------------------------------------------------------------------------------------------- |
| Shortest Route from A to D: Total Cost 14                                                                               |
| Path: [ Station A (0) ] ---> [ Station B (6) ] ---> [ Station D (14) ]                                                 |
+-------------------------------------------------------------------------------------------------------------------------+
```

### Key Structural Invariants:
1. **Top Bar (48px fixed height)**:
   - Left: Transit glyph `O=O` and brand text `Network Connectivity Analyzer`.
   - Right: Real-time network telemetry as natural language (`4 stations, 3 lines, 1 network`), text action `Download report`, and clean theme toggle. No overflowing badges.
2. **Left Task Rail (64px fixed width)**:
   - Houses 5 vertical icon buttons: `Build`, `Explore`, `Route`, `Weak points`, `What if`.
   - Acts as an accordion-mutex selector: Clicking an active item collapses the panel; clicking another slides open that single 320px drawer. The rest of the window belongs entirely to the canvas.
3. **Hero Map Canvas**:
   - Takes all remaining horizontal and vertical space.
   - Drawing sheet aesthetic with SVG subtle grid pattern (`rect fill="url(#dot-grid)"`).
   - Clean corner floating toolbar (top-right) for Zoom In, Zoom Out, Fit View, Re-layout.
   - Collapsible transit legend anchored at the bottom-left of the canvas.
4. **Floating Element Inspector**:
   - Appears anchored adjacent to clicked station or edge.
   - Displays station name, degree, component ID, or edge cost and bridge flag.
   - Includes a direct verb action: `Remove device` or `Remove link`. Eliminates the clunky fixed 3rd column!
5. **Bottom Dock (~170px fixed, collapsible)**:
   - Displays the detailed results of the active operation:
     - **BFS/DFS**: Ticket queue/stack badges showing sequential departure order, step visit logs.
     - **Route**: Prominent total cost readout with sequenced stop chips.
     - **Weak points**: Clickable pills for bridges and articulation points that pan/focus the map.
     - **What if**: Clear breakdown of disconnected networks resulting from the simulated failure.
   - Houses the `Algorithm log` as a secondary tab, preserving canvas width.

---

## 3. Transit Map Symbol Specification

| Concept | Visual Sign | Transit Metaphor & Rendering Details |
|---|---|---|
| **Device (Normal)** | `(O)` White circle, ink stroke | Standard station stop: `r=14`, fill `white`, stroke `var(--color-ink)`, stroke-width `3px`. Station label beside node with a 4px background halo so line crossings never obscure typography. |
| **Critical Device** (Articulation point) | `((O))` Double ring | Major interchange station: Outer concentric ring `r=20`, inner ring `r=12`. Visually signals a pivotal transfer hub whose removal splits lines. |
| **Link (Edge)** | Thick track line | Transit track line: `stroke-width: 6px`, `stroke-linecap: round`. Default colored by connected component line palette. |
| **Bridge (Weak Point)** | `-+-` Single track | Single-track crossing: 6px line with an ink stroke border plus a perpendicular crossbar tick (`length: 18px`, `width: 3px`) centered across the link midpoint. Plain tooltip: "Single track: only route between parts". |
| **Isolated Device** | `( . )` Dashed ring | Out-of-service / abandoned station: `r=12`, fill `transparent`, stroke `var(--color-ink-muted)`, stroke-dasharray `3, 3`. Label rendered in muted tone. |
| **Shortest Path** | Solid ink route | The active transit itinerary: Non-path lines drop to `opacity: 0.22`. Path edges rendered in bold ink (`stroke-width: 8px`). Stations along route feature sequential cost markers: `[Stop 0] -> [Stop +4] -> [Stop +9]`. |
| **BFS / DFS Traversal** | Filled ink stations & order rings | Sequential station exploration: Visited stations fill with solid ink (`fill: var(--color-ink)`, text white). Currently active station has an accent beacon ring. Waiting queue/stack items display small badge numbers `(1)`, `(2)`, `(3)`. |
| **Failure Simulation** | Closed line / station | Closed station/segment: Element fades to `opacity: 0.25` and is overlaid with a transit closure cross mark `X`. Newly fragmented subnets are instantly re-colored into separate component palettes. |
| **Link Cost** | Tabular halo numeral | Distance / transit fare: Crisp tabular numeral centered directly on line midpoint with a sheet-colored halo background (`paint-order: stroke fill; stroke: var(--bg-sheet); stroke-width: 4px; font-size: 12px; font-weight: 600`). |

---

## 4. Self-Review Against "Why It Looks Generic"

| Generic Flaw in Original UI | Specific Redesign Fix |
|---|---|
| 1. Near-black navy background with default blue & solid red buttons | Replaced with drafting sheet palette (`#F1F2EE` cool chalk sheet, `#FAFAF7` paper panel, `#14181F` ink). Buttons are ink filled (primary), hairline outlined (secondary), or red outlined text (destructive). |
| 2. Everything is the same rounded card with the same border and shadow | Eliminated card soup. Hierarchy is established strictly through hairline rules (`1px solid var(--rule-hairline)`), padding, and typographic scale. Radii are strictly constrained to 4px for inputs/buttons and 0px for structural panels. |
| 3. Numbered ALL-CAPS section labels (`1. BUILD NETWORK`) with stacked accordions | Replaced with a 64px vertical task rail containing 5 clear tasks. Only ONE task panel opens at a time (320px). All headings use natural sentence case. |
| 4. Monospace pill badges everywhere (`None`, `Completed`, `Level 0`) | Monospace pills completely removed. Metrics are presented in tabular Archivo numerals with semantic text labels. BFS levels are displayed directly on stations or in a structured table. |
| 5. Native unstyled inputs, ASCII arrows (`A <--> B (2)`), truncated text | Custom 36px inputs with styled inline SVG chevrons. Full link options formatted as `A to B, cost 2`. Device and link inputs have dedicated labels with no ellipsis truncation. |
| 6. Layout bugs & overflowing panels cutting off badges and sidebar bottoms | Eliminated the 3rd column sidebar. Top bar right side simplified to clean plain-text metrics. Bottom dock has a fixed 170px height with internal scrolling tabs, ensuring zero screen clipping at 1024x768 up to 1920x1080. |
| 7. Graph canvas is plain blue circles with grey link boxes | Replaced by the comprehensive Transit Map engine: thick 6px component colored lines, white station discs with paper halos, double-ring interchange stations, single-track bridge crossbars, and ticket-style queue docks. |

---

## 5. Accessibility & Motion Commitment
- **Accessible Focus Rings**: Every button, input, and interactive map element features an explicit `outline: 2px solid var(--accent-focus); outline-offset: 2px;`.
- **Contrast Ratios**: All text and component colors meet WCAG AA standards (minimum 4.5:1 for normal text, 3:1 for graphical lines) on both light and dark sheets.
- **Color-Blindness Independence**: Line styles (solid, dash patterns) and transit symbols uniquely identify components, weak points, and bridges independent of color.
- **Motion Constraints**: Respects `prefers-reduced-motion: reduce` by suppressing transition steps and immediately snapping to final calculation states.
