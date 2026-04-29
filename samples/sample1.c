// Sample 1: Basic arithmetic with constants
// Tests: Constant Folding, Constant Propagation, Dead Code Elimination
 
int x;
int y;
int z;
int unused;
 
x = 2 + 3;
y = x * 4;
z = 10 - 2;
unused = 99 * 0;
return y;
 