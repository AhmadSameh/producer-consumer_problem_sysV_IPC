# System V IPC Exercise

This is an exercise on the producer/consumer problem using System V IPC

### The producer:
  To run the producer, use the following arguments. <br />
  `./producer {COMMODITY_NAME} {MEAN} {STD_DEV} {SLEEP_INTERVAL} {BUF_SIZE}` <br />
  e.g. <br />
  `./producer NATURALGAS 7.1 0.5 200 40` would run a producer that declares:<br />
  1- The current price of Natural Gas<br />
  2- Is to be declared every 200ms<br />
  3- The change is according to a normal distribution with parameters (mean=0.5 and variance=0.25).<br />
  4- The size of the shared bounded buffer is 40 entries.<br />
  
### The Consumer:
  The consumer simply prints the price of each commodity with the text color changing with the price and it takes the size of its buffer as argument. <br />
  It should be something like this:
  ![image](https://github.com/AhmadSameh/producer-consumer_problem_sysV_IPC/assets/65093152/50a75d4b-43bc-447c-984c-18f28c55ab32)

### The Results:
  This terminal is called terminology, used to split the screen into multiplte terminals to easily access each running terminal. <br />
  This is an example of running 11 different producer processes and 1 consumer process, all of which are accessing the same memory.
![image](https://github.com/AhmadSameh/producer-consumer_problem_sysV_IPC/assets/65093152/7056348c-9563-4ad0-8126-015c76e0aef7)
