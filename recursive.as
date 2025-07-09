	lw	0 	1 	n	load n into r1
        lw	0	2	r	load r into r2
        lw	0	6	Faddr	load func address into r6
        jalr	6 	7		jump to function, store return in r7
        halt              		halt after function
comb    lw	0	6	one	load 1 into r6
	sw	5	7	Stack	store return address on stack
	add	5	6	5	increment stack
	sw	5	1	Stack	store n onto stack
	add	5	6	5	increment stack
	sw	5	2	Stack	store r onto stack
	add	5	6	5	increment stack
	sw	5	4	Stack	
	noop				^we will use r4 for current address, storing
	noop				space now
	add	5	6	5	increment stack
	beq	2	0	base	base case 1: r==0
	beq	1	2	base	base case 2: n==r
	lw	0	6	neg1	loas -1 into r6
	add	1	6	1	n--
	lw	0	4	Faddr	load start of function address into r4
	jalr	4	7		jump to start of function
	noop				^this calls comb(n-1,r)
	lw	0	6	neg1	load -1 into r6
	add	2	6	2	r--
	sw	5	3	Stack	save r3 onto stack (result)
	lw	0	6	one	load 1 into r6
	add	5	6	5	increment stack
	jalr	4	7		jump to start of function
	noop				^comb(n-1,r-1)
	lw	0	6	neg1	load -1 into r6
	add	5	6	5	decrement stack
	lw	5	4	Stack	load function result from stack
	add	4	3	3	add result to total(r3)
	beq	0	0	clean	jump to cleaning up registers
base    lw	0	6	one	base case: set result to 1
	add	0	6	3	result = 0+1
	beq	0	0	clean	jump to cleaning up registers
clean	lw	0	6	neg1	load -1 into r6
	add	5	6	5	decrement stack
	lw	5	4	Stack	load original function address from stack
	add	5	6	5	decrement stack
	lw	5	2	Stack	load r from stack
	add	5	6	5	decrement stack
	lw	5	1	Stack	load n from stack
	add	5	6	5	decrement stack
	lw	5	7	Stack	load return address from stack
	jalr	7	6		branch back to return address
n       .fill	7
r       .fill	3
neg1    .fill	-1
one     .fill	1
Faddr   .fill	comb
