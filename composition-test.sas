begin_version
3
end_version
begin_metric
0
end_metric
5
begin_variable
loc(truck)
-1
2
Atom at(a, truck)
Atom at(b, truck)
end_variable
begin_variable
loc(p1)
-1
3
Atom at(a, p1)
Atom at(b, p1)
<none of those>
end_variable
begin_variable
loc(p2)
-1
3
Atom at(a, p2)
Atom at(b, p2)
<none of those>
end_variable
begin_variable
in(truck,p1)
-1
2
Atom empty(truck)
Atom in(truck, p1)
end_variable
begin_variable
in(truck,p2)
-1
2
Atom empty(truck)
Atom in(truck, p2)
end_variable
0
begin_state
0
0
0
0
0
end_state
begin_goal
2
1 1
2 1
end_goal
10
begin_operator
drive truck a b
0
1
0 0 0 1
1
end_operator
begin_operator
drive truck b a
0
1
0 0 1 0
1
end_operator
begin_operator
drop truck a p1
1
0 0
2
0 1 2 0
0 3 1 0
1
end_operator
begin_operator
drop truck a p2
1
0 0
2
0 2 2 0
0 4 1 0
1
end_operator
begin_operator
drop truck b p1
1
0 1
2
0 1 2 1
0 3 1 0
1
end_operator
begin_operator
drop truck b p2
1
0 1
2
0 2 2 1
0 4 1 0
1
end_operator
begin_operator
pick-up truck a p1
2
0 0
4 0
2
0 1 0 2
0 3 0 1
1
end_operator
begin_operator
pick-up truck a p2
2
0 0
3 0
2
0 2 0 2
0 4 0 1
1
end_operator
begin_operator
pick-up truck b p1
2
0 1
4 0
2
0 1 1 2
0 3 0 1
1
end_operator
begin_operator
pick-up truck b p2
2
0 1
3 0
2
0 2 1 2
0 4 0 1
1
end_operator
0
