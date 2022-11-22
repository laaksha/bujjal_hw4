1. Run '<filename.c>' file.
   gcc -Wall -o <filename> <filename.c> -lpthread

2. Run '<filename>' executable file.
   ./<filename> <No. of jobs>
   Ex: ./<filename> 2

3. Enter following commands:
    submit <program> <arguments> 
	for ex: submit sleep 20
	New process will be created with specified arguments

    Showjobs
	It will list the all processes which are either running or waiting.

    submithistory
	It will list all the processes which were executed.
