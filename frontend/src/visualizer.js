let data = null;
let activeFilters = new Set();

document.getElementById('jsonUpload').addEventListener('change', function(e) {
  const file = e.target.files[0];
  if (!file) return;

  document.getElementById('fileName').textContent = file.name;

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


function render(data) {
  document.getElementById('emptyState').style.display   = 'none';
  document.getElementById('statsBar').style.display     = 'grid';
  document.getElementById('irSection').style.display    = 'grid';
  document.getElementById('logSection').style.display   = 'block';


  const changedOriginal  = new Set();
  const changedOptimized = new Set();

  data.logs.forEach(log => {
    changedOriginal.add(log.lineIndex);
    changedOptimized.add(log.lineIndex);
  });

  renderStats(data, changedOriginal);
  renderIR('originalCode',  data.original,  changedOriginal,  false);
  renderIR('optimizedCode', data.optimized, changedOptimized, true);
  renderFilters(data.logs);
  renderLogs(data.logs);
}


function renderStats(data, changedSet) {
  const removed = data.optimized.filter(i => i.op === 'NOP').length;
  document.getElementById('numOriginal').textContent  = data.original.length;
  document.getElementById('numOptimized').textContent = data.optimized.length;
  document.getElementById('numChanges').textContent   = data.logs.length;
  document.getElementById('numReduced').textContent   = removed;
}


function getOpClass(op) {
  switch(op) {
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

function renderIR(containerId, instructions, highlightSet, isOptimized) {
  const container = document.getElementById(containerId);
  container.innerHTML = '';

  instructions.forEach((instr, idx) => {
    const div = document.createElement('div');
    div.className = 'ir-line';
    div.dataset.index = idx;

    // Determine highlight class
    if (instr.op === 'NOP' && isOptimized) {
      div.classList.add('line-removed');
    } else if (highlightSet.has(idx) && isOptimized) {
      div.classList.add('line-added');
    } else if (highlightSet.has(idx) && !isOptimized) {
      div.classList.add('line-changed');
    }

    const lineNum = document.createElement('span');
    lineNum.className = 'line-num';
    lineNum.textContent = idx + 1;

    const lineText = document.createElement('span');
    lineText.className = 'line-text ' + getOpClass(instr.op);

    // Format text nicely
    if (instr.op === 'NOP') {
      lineText.textContent = '(removed)';
    } else {
      lineText.textContent = instr.text || '???';
    }

    div.appendChild(lineNum);
    div.appendChild(lineText);
    container.appendChild(div);
  });
}

const PASS_COLORS = {
  'Constant Folding':       'badge-cf',
  'Constant Propagation':   'badge-cp',
  'Dead Code Elimination':  'badge-dce',
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

function escapeHTML(str) {
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}