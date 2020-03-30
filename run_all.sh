import os
import sys
import subprocess

loops_range = [10**x for x in range(6, 10)] #from 1,000,000 up to 1,000,000,000
threads_range = [2**x for x in range(0, 6)] #from 1 up to 32 threads
total_bytes_range = [2**x for x in range(13, 31)] #from 8KB up to 1GB

if (os.system('make clean all')):
    print("Something went wrong during compilation...Exiting")
    sys.exit()

# for mode in range(2):
#     for total_bytes in total_bytes_range:
#         for loops in loops_range:
#             print("Mode: " + str(mode) + "\nTotal bytes: " + str(total_bytes) + "\nLoops: " + str(loops))
#             for threads in threads_range:
#                 args = "./main -t " + str(threads) + " -n " + str(loops) + " -b " + str(total_bytes) + " -m " + str(mode)
#                 subprocess.call(args, shell=True)

loops = 100000000
for mode in range(2):
    for total_bytes in total_bytes_range:
        print("Mode: " + str(mode) + "\nTotal bytes: " + str(total_bytes) + "\nLoops: " + str(loops))
        for threads in threads_range:
            args = "./main -t " + str(threads) + " -n " + str(loops) + " -b " + str(total_bytes) + " -m " + str(mode)
            subprocess.call(args, shell=True)