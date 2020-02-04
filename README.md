# Memory-Management-API---Phase-2
The part of the operating system that manages (part of) the memory hierarchy is called the memory manager. Its job is to efﬁciently manage memory: keep track of which parts of memory are in use, allocate memory to processes when they need it, and deallocate it when they are done. In general terms, there are two ways to keep track of memory usage: bitmaps and free lists. With a bitmap, memory is divided into allocation units as small as a few words and as large as several kilobytes. Corresponding to each allocation unit is a bit in the bitmap, which is 0 if the unit is free and 1 if it is occupied (or vice versa). Another way of keeping track of memory is to maintain a linked list of allocated and free memory segments, where a segment either contains a process or is an empty hole between two processes. In this homework you are expected to implement a memory manager which maintains a linked list for organizing the memory allocation.
