# Streaming Lockfree Stack

An attempt at implementing a lockfree stack by using streaming writes to memory rather than interlocked compare-exchange instruction. Interlocked instructions require a full memory barrier, which doesn't seem to be necessary for correctness of the stack. Instead we can bypass the cache and use main memory as the source of truth, rather than relying on cache coherency protocol. 

## TODO
 - Remove standard allocator in the Push method.
 - Figure out how to free the nodes once they are used up (MWCAS trickery may be useful here) 
