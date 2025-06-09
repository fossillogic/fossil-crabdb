import glob

# Collect all .c and .cpp files directly in the cases directory (not subdirectories)
source_files = glob.glob("cases/*.c") + glob.glob("cases/*.cpp")

# Sort files from a to z
source_files.sort()

# Print each file name with "cases/" prefix
for file in source_files:
    print(file)