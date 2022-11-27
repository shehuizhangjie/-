gcc -c *.c
gcc -o IterativeServer IterativeServer.o helper.o
gcc -o MultiProcessServer MultiProcessServer.o helper.o
gcc -o MultiThreadServer MultiThreadServer.o helper.o -pthread
gcc -o PipelineServer PipelineServer.o helper.o
gcc -o SelectServer SelectServer.o helper.o
gcc -o ThreadPoolServer ThreadPoolServer.o helper.o threadpool.o -pthread