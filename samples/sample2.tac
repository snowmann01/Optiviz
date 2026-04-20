# Sample 2: Algebraic simplification + control flow
# Tests: Algebraic Simplification, branches

a = 5
b = a * 1
c = b + 0
d = 0 * c
if a > 3 goto L1
goto L2
L1:
result = a + 1
return result
L2:
result = 0
return result