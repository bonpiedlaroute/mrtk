## Multicore Real-Time Kernel (MRTK)

MRTK is a multicore real-time operating systems designed, and developped by bonpiedlaroute.

Obviously MRTK can also be run on uniprocessor real-time systems, but has been developped to suit multicore systems with shared memory.

MRTK implements partitioned EDF (Earliest Deadline First) scheduling policy. This means that, after partitioning your tasks through cpu cores, MRTK will always schedule on each core the task with the nearest deadline.

MRTK can be used without the developer having to choose on which processor each task must run, what priority assign to a task. MRTK does this for you, in fact you just need to specify execution time and deadline of each task, then launch MRTK, it will find a partition of tasks on the differents processors, test the real-time scheduling of yours tasks and run yours tasks if everything is correct. That's cool hein :-)

MRTK technology allows you also to specify in configuration the processor utilization usage you want to achieve. You may find out that your application can met its real-time constraints when using only 60% of each processor, then reduce frequency of each processor, this could be helpful for battery life save.
