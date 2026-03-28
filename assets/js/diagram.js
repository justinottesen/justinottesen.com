// Loads the draw.io SVG diagram and wires up interactivity.
//
// The SVG (assets/img/architecture.svg) is designed in draw.io and exported
// as SVG. Each interactive node has a semantic id set via draw.io's object
// wrapper, which produces a data-cell-id attribute in the SVG output.
//
// data/architecture.json holds metadata (description, category, links) keyed
// by those same ids. This file only handles wiring - layout lives in the SVG,
// data lives in the JSON.

const CATEGORIES = {
  'my-own':              { label: 'my own',              fill: '#dcfce7', stroke: '#16a34a', text: '#14532d' },
  'in-progress':         { label: 'in progress',         fill: '#fef9c3', stroke: '#ca8a04', text: '#713f12' },
  'external-dependency': { label: 'external dependency', fill: '#dbeafe', stroke: '#2563eb', text: '#1e3a8a' },
  'planned':             { label: 'planned',             fill: '#f3f4f6', stroke: '#9ca3af', text: '#374151' },
  'retired':             { label: 'retired',             fill: '#f9fafb', stroke: '#d1d5db', text: '#9ca3af' },
};

// Padding added around draw.io's tightly-cropped viewBox (in SVG units)
const PADDING = 24;

async function loadDiagram() {
  const [svgText, data] = await Promise.all([
    fetch('assets/img/architecture.svg').then(r => r.text()),
    fetch('data/architecture.json').then(r => r.json()),
  ]);

  const container = document.getElementById('diagram-container');
  container.innerHTML = svgText;

  const svg = container.querySelector('svg');

  // Scale to container width; height follows from the viewBox aspect ratio
  svg.removeAttribute('width');
  svg.removeAttribute('height');
  svg.style.width       = '100%';
  svg.style.display     = 'block';
  svg.style.colorScheme = 'light';

  // draw.io crops tightly to content - expand the viewBox to add breathing room
  const [x, y, w, h] = svg.getAttribute('viewBox').split(' ').map(Number);
  svg.setAttribute('viewBox', `${x - PADDING} ${y - PADDING} ${w + PADDING * 2} ${h + PADDING * 2}`);

  renderLegend(data);
  wireInteractivity(svg, data);
  setupPanZoom(svg);
}

// -- Legend --------------------------------------------------------------------

// Only show categories that are actually present in the current diagram.
function renderLegend(data) {
  const present   = new Set(Object.values(data).map(n => n.category));
  const container = document.getElementById('diagram-legend');
  for (const [key, cat] of Object.entries(CATEGORIES)) {
    if (!present.has(key)) continue;
    const span = document.createElement('span');
    span.className = 'legend-item';
    span.textContent = cat.label;
    span.style.backgroundColor = cat.fill;
    span.style.color            = cat.text;
    span.style.borderColor      = cat.stroke;
    container.appendChild(span);
  }
}

// -- Interactivity -------------------------------------------------------------

// Set by the pan handler when a drag occurs; prevents the subsequent click
// event from opening the detail panel.
let didDrag = false;

function wireInteractivity(svg, data) {
  for (const [id, node] of Object.entries(data)) {
    const cell = svg.querySelector(`[data-cell-id="${id}"]`);
    if (!cell) continue;

    const cat  = CATEGORIES[node.category] ?? CATEGORIES['planned'];
    const rect = cell.querySelector('rect');

    if (rect) {
      rect.style.fill   = cat.fill;
      rect.style.stroke = cat.stroke;

      cell.addEventListener('mouseenter', () => { rect.style.filter = 'brightness(0.92)'; });
      cell.addEventListener('mouseleave', () => { rect.style.filter = ''; });
    }

    cell.style.cursor = 'pointer';
    cell.addEventListener('click', () => {
      if (didDrag) return;
      showDetail(node, cat);
    });
  }
}

// -- Pan + zoom ----------------------------------------------------------------

function setupPanZoom(svg) {
  const [vbX, vbY, vbW, vbH] = svg.getAttribute('viewBox').split(' ').map(Number);
  const state = { scale: 1, tx: 0, ty: 0 };
  let drag = null;
  let lastPinchDist = null;

  // Prevent the browser from handling touch gestures (scroll, pinch-zoom) on
  // the SVG so our own handlers receive them uninterrupted
  svg.style.touchAction = 'none';

  // Wrap all SVG content in a viewport group so transforms don't affect the
  // viewBox coordinate system itself
  const viewport = document.createElementNS('http://www.w3.org/2000/svg', 'g');
  viewport.id = 'viewport';
  while (svg.firstChild) viewport.appendChild(svg.firstChild);
  svg.appendChild(viewport);

  function applyTransform() {
    viewport.setAttribute('transform',
      `translate(${state.tx}, ${state.ty}) scale(${state.scale})`);
  }

  // Convert a screen point to SVG viewBox coordinates
  function toVB(clientX, clientY) {
    const r = svg.getBoundingClientRect();
    return {
      x: vbX + (clientX - r.left) / r.width  * vbW,
      y: vbY + (clientY - r.top)  / r.height * vbH,
    };
  }

  function applyZoom(factor, pivotX, pivotY) {
    const { x, y } = toVB(pivotX, pivotY);
    const newScale  = Math.max(0.3, Math.min(5, state.scale * factor));
    const k         = newScale / state.scale;
    state.tx    = x - (x - state.tx) * k;
    state.ty    = y - (y - state.ty) * k;
    state.scale = newScale;
    applyTransform();
  }

  // Zoom toward the cursor
  svg.addEventListener('wheel', e => {
    e.preventDefault();
    applyZoom(e.deltaY > 0 ? 0.9 : 1.1, e.clientX, e.clientY);
  }, { passive: false });

  // -- Mouse drag to pan -------------------------------------------------------

  svg.addEventListener('mousedown', e => {
    if (e.button !== 0) return;
    drag = { startX: e.clientX, startY: e.clientY, tx: state.tx, ty: state.ty, moved: false };
    svg.style.cursor = 'grab';
  });

  window.addEventListener('mousemove', e => {
    if (!drag) return;
    const r  = svg.getBoundingClientRect();
    const dx = (e.clientX - drag.startX) / r.width  * vbW;
    const dy = (e.clientY - drag.startY) / r.height * vbH;
    if (!drag.moved && (Math.abs(dx) > 3 || Math.abs(dy) > 3)) {
      drag.moved = true;
      svg.style.cursor = 'grabbing';
    }
    if (drag.moved) {
      state.tx = drag.tx + dx;
      state.ty = drag.ty + dy;
      applyTransform();
    }
  });

  window.addEventListener('mouseup', () => {
    didDrag = drag?.moved ?? false;
    drag    = null;
    svg.style.cursor = '';
  });

  // -- Touch: single-finger pan, two-finger pinch-zoom ------------------------

  svg.addEventListener('touchstart', e => {
    if (e.touches.length === 1) {
      const t = e.touches[0];
      drag = { startX: t.clientX, startY: t.clientY, tx: state.tx, ty: state.ty, moved: false };
      lastPinchDist = null;
    } else if (e.touches.length === 2) {
      drag = null;
      lastPinchDist = Math.hypot(
        e.touches[0].clientX - e.touches[1].clientX,
        e.touches[0].clientY - e.touches[1].clientY,
      );
    }
  }, { passive: true });

  svg.addEventListener('touchmove', e => {
    e.preventDefault();
    if (e.touches.length === 1 && drag) {
      const t  = e.touches[0];
      const r  = svg.getBoundingClientRect();
      const dx = (t.clientX - drag.startX) / r.width  * vbW;
      const dy = (t.clientY - drag.startY) / r.height * vbH;
      if (!drag.moved && (Math.abs(dx) > 3 || Math.abs(dy) > 3)) drag.moved = true;
      if (drag.moved) {
        state.tx = drag.tx + dx;
        state.ty = drag.ty + dy;
        applyTransform();
      }
    } else if (e.touches.length === 2 && lastPinchDist !== null) {
      const dist  = Math.hypot(
        e.touches[0].clientX - e.touches[1].clientX,
        e.touches[0].clientY - e.touches[1].clientY,
      );
      const midX  = (e.touches[0].clientX + e.touches[1].clientX) / 2;
      const midY  = (e.touches[0].clientY + e.touches[1].clientY) / 2;
      applyZoom(dist / lastPinchDist, midX, midY);
      lastPinchDist = dist;
    }
  }, { passive: false });

  svg.addEventListener('touchend', () => {
    didDrag       = drag?.moved ?? false;
    drag          = null;
    lastPinchDist = null;
  });

  // Double-click to reset view
  svg.addEventListener('dblclick', () => {
    state.tx = 0; state.ty = 0; state.scale = 1;
    applyTransform();
  });
}

// -- Detail panel --------------------------------------------------------------

function showDetail(node, cat) {
  document.getElementById('detail-label').textContent = node.label;

  const badge = document.getElementById('detail-category');
  badge.textContent           = cat.label;
  badge.style.backgroundColor = cat.fill;
  badge.style.color           = cat.text;
  badge.style.borderColor     = cat.stroke;

  document.getElementById('detail-description').textContent = node.description;

  const linksEl = document.getElementById('detail-links');
  linksEl.innerHTML = '';
  for (const link of (node.links ?? [])) {
    linksEl.appendChild(makeLink(link.label, link.url));
  }

  document.getElementById('node-detail').classList.remove('hidden');
}

function makeLink(label, url) {
  const a = document.createElement('a');
  a.href        = url;
  a.textContent = label;
  a.target      = '_blank';
  a.rel         = 'noopener noreferrer';
  return a;
}

// -----------------------------------------------------------------------------

loadDiagram();
