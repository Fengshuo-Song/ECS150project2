# User-level Thread Library
## Group Members
Fengshuo Song
Hengze Ye

## Summary
Our project is a user-level thread library that supports multiple threads to\
run within the same process. It manages the execution of threads in a\
round-robin fashion that schedules tasks to run in a short period of time.\
It needs a timer to interrupt the current thread to allow another thread to\
run. In addition, the user can also manage the thread executions by calling\
yield and join functions.

## Implementation
The implementation of this program can be divided into 4 parts.
1.	Start the program by calling uthread_start() function. The user can\
configure whether to use the preemption. If the parameter of the function\
is 0, then the program does not use preemption; otherwise, preemption will\
be used.
2.	Create new threads by calling uthread_create() function. This function\
gives a unique thread ID, which can be used later to manage the thread.
3.	Manage the execution, return value and termination of the threads. In\
this step, users can control the scheduling of different threads, get the\
current thread ID and obtain a particular value returned by a thread.
4.	End the program by calling uthread_start() function. This frees the\
memory space created in previous steps and restore the previous environment\
variables(signal action and timer configuration).(Part 2 and Part 3 can be\
repeated in different orders.)

## Limitation
1.	The maximum number of threads that a program could create is defined by\
the macro 'USHRT_MAX', which is 65536. Overflowing this number will cause a\
failure.
2.	Any thread's stack can store data within 32768 bytes, as defined by the\
macro 'UTHREAD_STACK_SIZE'.
3.	There is no deadlock detection in this library meaning that a user cannot\
let different threads wait for each other (i.e. A joins B and B joins A; or A\
joins B, B joins C and C joins A)
4.	It does not support threads with parameters. All threads must have void\
parameter.
5.	As soon as the main thread terminates, all other threads cannot proceed\
any more.
6.	When it comes to handling errors, the program simply returns NULL pointer\
or some special values such as -1, and leave the error to the user.
7.	The timer gives same amount of maximum time that each thread can run at a\
time, which is not good when there are some very important and urgent thread.

## Testing
We have created testing files for queue, uthread and preempt. In the queue\
tester, we use unit testing, which implement several testing functions that\
will trigger some part of our programs. In the uthread tester, we also create\
multiple testing functions. However, since some values in the thread cannot be\
collected by the main thread easily, it is difficult to do unit testing in the\
main thread. Instead, we print both expected and actual outputs on the screen\
and check if they are the same. In the preempt tester, we print different\
messages in different threads. In the console, we can see when the threads\
begin and end, and how the program switches among different threads due to the\
timer interrupt. 

## Extensibility 
1.	Users can set priorities to the threads and the thread with higher\
priority has a higher chance to be elected to run.
2.	The timer can be configured such that more important thread can have more\
time to run before it gets interrupted.
3.	Users can add some callback functions, which are triggered when a specific\
event happens. For example, the program can call some functions during the\
creation of a thread, the first time a thread is running, the time a thread\
uses CPU again, etc.
4.	The program can have some functions that is able to deal with some errors\
by itself.

## Sources

[*Queue*](https://www.geeksforgeeks.org/queue-data-structure/)

[*Signal Action*](https://www.gnu.org/software/libc/manual/html_mono/libc.html
#Signal-Actions)

[*Alarm*](https://www.gnu.org/software/libc/manual/html_mono/libc.html
#Setting-an-Alarm)
