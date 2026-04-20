# Sample 1: Basic arithmetic with constants
# Tests: Constant Folding, Constant Propagation, Dead Code Elimination

t1 = 2 + 3
t2 = t1 * 4
t3 = 10 - 2
x = t2 + t3
unused = 99 * 0
return x
