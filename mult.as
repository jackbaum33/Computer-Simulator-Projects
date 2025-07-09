        lw      0  1  mcand             
        lw      0  2  mplier
        lw      0  3  zero            
        lw      0  4  count        
        lw      0  5  mask        
        lw      0  7  neg1
loop    nor     5  5  6        
        nor     2  2  7        
        nor     6  7  6        
        lw      0  7  neg1        
        beq     6  0  skip        
        add     1  3  3
skip    add     1  1  1        
        add     5  5  5        
        add     4  7  4        
        beq     4  0  done        
        beq     0  0  loop
done    halt
mcand   .fill   6203
mplier  .fill   1429
zero    .fill   0
count   .fill   15
mask    .fill   1
neg1    .fill   -1
