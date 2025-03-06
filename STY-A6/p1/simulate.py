#!/usr/bin/env python3
import subprocess
import matplotlib.pyplot as plt

# Define the policies to simulate and the cache sizes to test.
policies = ['FIFO', 'LRU', 'RAND', 'OPT']
cache_sizes = range(1, 21)  # Example: cache sizes from 1 to 20

# Trace file (must exist and contain one integer per line)
trace_file = "testoutput.txt"

# Dictionary to store results: key = policy, value = list of misses for each cache size.
results = {policy: [] for policy in policies}

# Loop over policies and cache sizes.
for policy in policies:
    for cs in cache_sizes:
        # Build the command string.
        # This replicates a command like:
        #   ./paging-policy.py --addressfile=testoutput.txt --policy=FIFO --cachesize=3 --compute
        command = (
            f"python3 paging-policy.py --addressfile={trace_file} "
            f"--policy={policy} --cachesize={cs} --compute"
        )
        try:
            output = subprocess.check_output(command, shell=True, text=True)
            # Parse the output to find the line with FINALSTATS.
            # For example, the output line might be:
            # "FINALSTATS hits 10   misses 20   hitrate 33.3333"
            for line in output.splitlines():
                if "FINALSTATS" in line:
                    parts = line.split()
                    # parts[0]="FINALSTATS", parts[1]="hits", parts[2]=<hits>,
                    # parts[3]="misses", parts[4]=<misses>, parts[5]="hitrate", parts[6]=<hitrate>
                    misses = int(parts[4])
                    results[policy].append(misses)
                    break
        except subprocess.CalledProcessError as e:
            print(f"Error running command: {command}")
            print(f"Error message: {e.output}")
            results[policy].append(None)

# Print out the results.
for policy in policies:
    print(f"{policy}: {results[policy]}")

# Plot the results.
plt.figure(figsize=(10, 6))
for policy in policies:
    plt.plot(list(cache_sizes), results[policy], marker='o', label=policy)
plt.xlabel("Number of Physical Pages (Cache Size)")
plt.ylabel("Number of Page Faults")
plt.yscale("log")  # Log scale for Y-axis
plt.title("Page Faults vs. Cache Size for Various Replacement Policies")
plt.legend()
plt.grid(True)

output_filename = "page_faults.png"
plt.savefig(output_filename, dpi=300)
plt.close()
