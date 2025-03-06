#!/usr/bin/env python3
#
# Put your solution into the three functions in this file
#

def get_page_list(filename):
    # Expected functionality: Read content of file (valgrind/lackey output), and then
    # - find all lines containing memory access:
    #   Line has I, L, M, S at the beginning (first two columns), then a space
    #   After that an address in hex notation
    #   Finally, a comma and an access size in byte
    # - construct an ordered list of memory pages accessed (based on the address)
    # - construct an set of memory pages that contain instructions (based on address in 'I' lines)
    page_access_list = []
    instruction_page_set = set()
    # (I:code, L:load, S:store, M:modify),
    with open(filename, "r") as file:
        for line in file:
            # We have things to ignore at the start and at the end of the file
            parts = line.split()
            # This will skip any of the text at the start and end
            if parts[0] not in ["I", "L", "M", "S"]:
                continue
            # We have an address token in hex notation
            subparts = parts[1].split(",")
            address = subparts[0]
            sizeBytes = subparts[1]
            
            # Transform the hex address into bytes to calculate what page number it is
            addressBytes = int(address, 16)
            
            # Calculate the page number
            pageSize = 4096 # bytes
            pageNumber = addressBytes // pageSize
            if parts[0] == "I":
                instruction_page_set.add(pageNumber)

            page_access_list.append(pageNumber)

    return page_access_list, instruction_page_set


def export_page_trace(page_access_list, output_file):
    new_list = []
    for index in range(len(page_access_list) - 1):
        if page_access_list[index] == page_access_list[index+1]:
            continue
        else:
            new_list.append(page_access_list[index])
    new_list.append(page_access_list[-1])
    
    with open(output_file, "w") as output:
        for page in new_list:
            output.write(str(page) + "\n")


def plot_memory_access(page_access_list, png_file=None, instruction_page_set=None):

    # TODO: Implement (remove this comment before submission if you implemented somthing)

    return


