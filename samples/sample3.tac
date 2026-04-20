# Sample 3: Mixed optimizations
# Tests all 4 passes together

x = 10
y = 3 + 7
z = x * 1
t1 = z + 0
t2 = y * 0
t3 = t1 + t2
dead1 = 5 * 6
dead2 = dead1 + 1
return t3


