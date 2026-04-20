# TAC Optimizer — Compiler Project

A C++ compiler backend that reads Three Address Code (TAC), applies
optimization passes, and exports JSON for a browser-based visualizer.

---

## Project Structure

```
compiler-project/
│
├── backend/                  ← C++ compiler backend
│   ├── main.cpp              ← Entry point
│   ├── tac.h / tac.cpp       ← TAC instruction data structures
│   ├── tac_parser.h          ← Reads .tac files into TACProgram
│   ├── optimizer.h / .cpp    ← All 4 optimization passes
│   ├── json_exporter.h       ← Exports original + optimized IR to JSON
│   └── Makefile              ← Build script
│
├── frontend/                 ← JavaScript visualizer
│   ├── index.html            ← Main page
│   ├── src/
│   │   └── visualizer.js     ← Rendering and interaction logic
│   └── styles/
│       └── main.css          ← Dark theme styles
│
├── samples/                  ← Example .tac input files
│   ├── sample1.tac           ← Basic constant folding/propagation
│   ├── sample2.tac           ← Algebraic simplification + branches
│   └── sample3.tac           ← All passes combined
│
└── README.md
```

---

## Step 1 — Build the Backend (C++)

```bash
cd backend
make
```

This produces a `compiler` executable.

---

## Step 2 — Run the Compiler on a TAC File

```bash
./compiler ../samples/sample1.tac ../frontend/output.json
```

This reads `sample1.tac`, runs all optimizations, and writes `output.json`
into the frontend folder.

---

## Step 3 — Open the Visualizer

Open `frontend/index.html` in any browser.
Click **Load output.json** and select the file produced in Step 2.

The visualizer will show:
- Original IR vs Optimized IR side by side
- Color-highlighted changed/removed lines
- Optimization log table with pass filters

---

## TAC File Format

Write `.tac` files using this syntax:

```
# Comments start with #
x = 5               # Assignment
t1 = x + 3          # Binary op: +, -, *, /
t2 = t1 * 1         # Algebraic simplification candidate
if t2 > 0 goto L1   # Conditional branch
goto L2             # Unconditional branch
L1:                 # Label
return t2           # Return
```

---

## Optimization Passes

| Pass                    | Example                        |
|-------------------------|--------------------------------|
| Constant Folding        | `t1 = 2 + 3`  →  `t1 = 5`     |
| Constant Propagation    | `x = 5; y = x + 1` → `y = 5 + 1` |
| Algebraic Simplification| `x * 1` → `x`, `x + 0` → `x` |
| Dead Code Elimination   | Removes assignments never used |

---

## Coming Later (Phase 2)

- Lexer (tokenizer for subset-of-C)
- Recursive Descent Parser
- AST generation
- Automatic TAC generation from C source code