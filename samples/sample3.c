// Sample 3: While loop + all optimization passes
// Tests: All 4 passes, WHILE loop TAC generation
 
int x;
int y;
int z;
int sum;
int dead;
 
x = 10;
y = 3 + 7;
z = x * 1;
sum = z + 0;
dead = y * 0;
 
while (x > 0) {
    sum = sum + 1;
    x = x - 1;
}
 
return sum;