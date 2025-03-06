#!/usr/bin/env python3
import random
import matplotlib.pyplot as plt

def simulate_page_faults(page_trace, cachesize, policy):
    """
    Simulate the page replacement algorithm for a given page trace, cache size, and policy.
    
    Parameters:
      page_trace : list of int
          The ordered list of page accesses.
      cachesize : int
          Number of pages in physical memory.
      policy : str
          Replacement policy: "FIFO", "LRU", "RAND", or "OPT".
    
    Returns:
      faults : int
          The number of page faults during the simulation.
    """
    faults = 0
    memory = []
    
    if policy == 'FIFO':
        for page in page_trace:
            if page in memory:
                continue
            faults += 1
            if len(memory) < cachesize:
                memory.append(page)
            else:
                memory.pop(0)  # remove the oldest page
                memory.append(page)
                
    elif policy == 'LRU':
        for page in page_trace:
            if page in memory:
                memory.remove(page)
                memory.append(page)
            else:
                faults += 1
                if len(memory) < cachesize:
                    memory.append(page)
                else:
                    memory.pop(0)  # least recently used is at index 0
                    memory.append(page)
                    
    elif policy == 'RAND':
        for page in page_trace:
            if page in memory:
                continue
            faults += 1
            if len(memory) < cachesize:
                memory.append(page)
            else:
                idx = random.randrange(len(memory))
                memory.pop(idx)
                memory.append(page)
                
    elif policy == 'OPT':
        n = len(page_trace)
        for i, page in enumerate(page_trace):
            if page in memory:
                continue
            faults += 1
            if len(memory) < cachesize:
                memory.append(page)
            else:
                # For each page in memory, find its next use.
                next_use = {}
                for m in memory:
                    try:
                        next_index = page_trace.index(m, i+1)
                    except ValueError:
                        next_index = float('inf')  # Not used again.
                    next_use[m] = next_index
                # Remove the page that is used farthest in the future.
                victim = max(memory, key=lambda m: next_use[m])
                memory.remove(victim)
                memory.append(page)
    else:
        raise ValueError("Unknown policy: " + policy)
        
    return faults

import matplotlib.pyplot as plt
import numpy as np

def plot_memory_access(page_list, pngfile=None):
    # Normalize page numbers
    unique_pages = sorted(set(page_list))
    page_to_index = {page: i for i, page in enumerate(unique_pages)}
    normalized_pages = [page_to_index[page] for page in page_list]

    # Create 1000 bins
    num_bins = 1000
    bin_size = len(page_list) // num_bins
    access_matrix = np.zeros((num_bins, len(unique_pages)))

    for i in range(num_bins):
        start = i * bin_size
        end = start + bin_size
        for page in normalized_pages[start:end]:
            access_matrix[i][page] = 1

    # Plot
    plt.imshow(access_matrix.T, cmap='hot', aspect='auto')
    plt.xlabel("Time (bins)")
    plt.ylabel("Page Number")
    plt.title("Memory Access Pattern")
    if pngfile:
        plt.savefig(pngfile)
    else:
        plt.show()