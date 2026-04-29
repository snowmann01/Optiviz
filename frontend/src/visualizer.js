// ── State ─────────────────────────────────────────────────────────────────────
let data          = null;
let activeFilters = new Set();
let cSourceText   = null;

// ── C File Upload ─────────────────────────────────────────────────────────────
document.getElementById('cUpload').addEventListener('change', function(e) {
  const file = e.target.files[0];
  if (!file) return;

  document.getElementById('cFileName').textContent = file.name;
  document.getElementById('compileHint').innerHTML =
    `<span class="hint-icon">💡</span> Now run: <code>compiler.exe ${file.name} output.json</code>`;

  const reader = new FileReader();
  reader.onload = function(ev) {
    cSourceText = ev.target.result;
    renderCSource(cSourceText);

    // If JSON already loaded, make sure panels are visible
    if (data) showPanels();
  };
  reader.readAsText(file);
});

// ── JSON Upload ───────────────────────────────────────────────────────────────
document.getElementById('jsonUpload').addEventListener('change', function(e) {
  const file = e.target.files[0];
  if (!file) return;

  document.getElementById('jsonFileName').textContent = file.name;

  const reader = new FileReader();
  reader.onload = function(ev) {
    try {
      data = JSON.parse(ev.target.result);
      render(data);
    } catch (err) {
      alert('Failed to parse JSON: ' + err.message);
    }
  };
  reader.readAsText(file);
});

// ── Render C Source Panel ─────────────────────────────────────────────────────
function renderCSource(source) {
  const panel = document.getElementById('cSourcePanel');
  panel.innerHTML = ''; // clear "load a .c file" message

  // Split source into lines and render with line numbers
  const lines = source.split('\n');
  lines.forEach((line, idx) => {
    const div = document.createElement('div');
    div.className = 'ir-line';

    const lineNum = document.createElement('span');
    lineNum.className = 'line-num';
    lineNum.textContent = idx + 1;

    const lineText = document.createElement('span');
    lineText.className = 'line-text c-line';
    lineText.textContent = line || ' ';

    // Basic syntax coloring
    const trimmed = line.trim();
    if (trimmed.startsWith('//')) {
      lineText.className = 'line-text c-comment';
    } else if (/^\s*(int|if|else|while|return)\b/.test(line)) {
      lineText.className = 'line-text c-keyword';
    } else if (trimmed === '{' || trimmed === '}') {
      lineText.className = 'line-text c-brace';
    }

    div.appendChild(lineNum);
    div.appendChild(lineText);
    panel.appendChild(div);
  });
}

// ── Show Panels ───────────────────────────────────────────────────────────────
function showPanels() {
  document.getElementById('emptyState').style.display     = 'none';
  document.getElementById('pipelineBanner').style.display = 'flex';
  document.getElementById('statsBar').style.display       = 'grid';
  document.getElementById('panelsSection').style.display  = 'grid';
  document.getElementById('logSection').style.display     = 'block';
}

// ── Main Render ───────────────────────────────────────────────────────────────
function render(data) {
  showPanels();

  // If C source was loaded, render it now
  if (cSourceText) {
    renderCSource(cSourceText);
  }

  const changedLines = new Set(data.logs.map(l => l.lineIndex));

  renderStats(data);
  renderIR('originalCode',  data.original,  changedLines, false);
  renderIR('optimizedCode', data.optimized, changedLines, true);
  renderFilters(data.logs);
  renderLogs(data.logs);
}

// ── Stats ─────────────────────────────────────────────────────────────────────
function renderStats(data) {
  const removed = data.optimized.filter(i => i.op === 'NOP').length;
  document.getElementById('numOriginal').textContent  = data.original.length;
  document.getElementById('numOptimized').textContent = data.optimized.length;
  document.getElementById('numChanges').textContent   = data.logs.length;
  document.getElementById('numReduced').textContent   = removed;
}

// ── IR Rendering ──────────────────────────────────────────────────────────────
function getOpClass(op) {
  switch (op) {
    case 'ASSIGN':  return 'op-assign';
    case 'ADD': case 'SUB': case 'MUL': case 'DIV': return 'op-arith';
    case 'LABEL':   return 'op-label';
    case 'GOTO':    return 'op-goto';
    case 'IF_GOTO': return 'op-ifgoto';
    case 'RETURN':  return 'op-return';
    case 'NOP':     return 'op-nop';
    default:        return '';
  }
}

function renderIR(containerId, instructions, changedLines, isOptimized) {
  const container = document.getElementById(containerId);
  container.innerHTML = '';

  instructions.forEach((instr, idx) => {
    const div = document.createElement('div');
    div.className = 'ir-line';

    if (isOptimized && instr.op === 'NOP') {
      div.classList.add('line-removed');
    } else if (isOptimized && changedLines.has(idx)) {
      div.classList.add('line-added');
    } else if (!isOptimized && changedLines.has(idx)) {
      div.classList.add('line-changed');
    }

    const lineNum = document.createElement('span');
    lineNum.className = 'line-num';
    lineNum.textContent = idx + 1;

    const lineText = document.createElement('span');
    lineText.className = 'line-text ' + getOpClass(instr.op);
    lineText.textContent = instr.op === 'NOP' ? '(removed)' : (instr.text || '???');

    div.appendChild(lineNum);
    div.appendChild(lineText);
    container.appendChild(div);
  });
}

// ── Pass Filters ──────────────────────────────────────────────────────────────
const PASS_COLORS = {
  'Constant Folding':         'badge-cf',
  'Constant Propagation':     'badge-cp',
  'Dead Code Elimination':    'badge-dce',
  'Algebraic Simplification': 'badge-as',
};

function renderFilters(logs) {
  const passes = [...new Set(logs.map(l => l.pass))];
  activeFilters = new Set(passes);

  const container = document.getElementById('passFilters');
  container.innerHTML = '';

  passes.forEach(pass => {
    const badge = document.createElement('span');
    badge.className = 'pass-badge ' + (PASS_COLORS[pass] || '');
    badge.textContent = pass;
    badge.dataset.pass = pass;
    badge.addEventListener('click', () => toggleFilter(pass, badge));
    container.appendChild(badge);
  });
}

function toggleFilter(pass, badge) {
  if (activeFilters.has(pass)) {
    activeFilters.delete(pass);
    badge.classList.add('inactive');
  } else {
    activeFilters.add(pass);
    badge.classList.remove('inactive');
  }
  renderLogs(data.logs);
}

// ── Log Table ─────────────────────────────────────────────────────────────────
function renderLogs(logs) {
  const tbody = document.getElementById('logBody');
  tbody.innerHTML = '';

  const filtered = logs.filter(l => activeFilters.has(l.pass));

  if (filtered.length === 0) {
    const tr = document.createElement('tr');
    tr.innerHTML = '<td colspan="4" style="text-align:center;color:var(--muted);padding:24px">No entries for selected passes</td>';
    tbody.appendChild(tr);
    return;
  }

  filtered.forEach(log => {
    const tr = document.createElement('tr');
    const isRemoved = log.after === 'REMOVED';
    tr.innerHTML = `
      <td>${log.lineIndex + 1}</td>
      <td><span class="pass-badge ${PASS_COLORS[log.pass] || ''}">${log.pass}</span></td>
      <td>${escapeHTML(log.before)}</td>
      <td>${isRemoved
        ? '<span class="removed-tag">REMOVED</span>'
        : escapeHTML(log.after)}</td>
    `;
    tbody.appendChild(tr);
  });
}

// ── Utility ───────────────────────────────────────────────────────────────────
function escapeHTML(str) {
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}