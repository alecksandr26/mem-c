import re
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

# Sample log data with 'non used memory by user'
log_data = open("data.txt").read()

# Step 1: Extract the data
n_allocs = []
alloc_sizes = []
times = []
non_used_memories = []
non_used_mem_percentages = []
freeded_chunks_percentages = []
non_freeded_chunks_percentages = []  # Add lists for the new percentages

# Regular expression to match the log line format, including freeded and non-freeded chunks
pattern = re.compile(r"n allocs: (\d+), alloc size: (\d+), non used memory by user: (\d+), non used memory by user percetnage: ([\d.]+), elapsed time: ([\d.]+), freeded chunks percentage: ([\d.]+), non freeded chunks percetnage: ([\d.]+)")

for line in log_data.splitlines():
    match = pattern.search(line)
    if match:
        n_alloc = int(match.group(1))
        alloc_size = int(match.group(2))
        non_used_memory = int(match.group(3))
        non_used_memory_percent = float(match.group(4))
        elapsed_time = float(match.group(5))
        freeded_chunks_percentage = float(match.group(6))
        non_freeded_chunks_percentage = float(match.group(7))

        n_allocs.append(n_alloc)
        alloc_sizes.append(alloc_size)
        non_used_memories.append(non_used_memory)
        non_used_mem_percentages.append(non_used_memory_percent)
        times.append(elapsed_time)
        freeded_chunks_percentages.append(freeded_chunks_percentage)  # Append to the list
        non_freeded_chunks_percentages.append(non_freeded_chunks_percentage)  # Append to the list
    else:
        print(f"No match found for line: {line}")  # Debugging output

# Check if any data was collected
print(f"Number of entries matched: {len(n_allocs)}")
if len(n_allocs) == 0:
    raise ValueError("No valid log entries found. Please check the log format.")

# Step 2: Prepare the data
alloc_times = np.array([n * size for n, size in zip(n_allocs, alloc_sizes)])
elapsed_time = np.array(times)
non_used_memory = np.array(non_used_memories)
non_used_memory_percent = np.array(non_used_mem_percentages)
freeded_chunks_percent = np.array(freeded_chunks_percentages)
non_freeded_chunks_percent = np.array(non_freeded_chunks_percentages)

# Step 3: Perform Linear Regression for time (allocs * size vs. time)
X_time = alloc_times.reshape(-1, 1)
y_time = elapsed_time

model_time = LinearRegression()
model_time.fit(X_time, y_time)

slope_time = model_time.coef_[0]
intercept_time = model_time.intercept_

# Step 4: Perform Linear Regression for non-used memory (allocs * size vs. non-used memory)
X_memory = alloc_times.reshape(-1, 1)
y_memory = non_used_memory

model_memory = LinearRegression()
model_memory.fit(X_memory, y_memory)

slope_memory = model_memory.coef_[0]
intercept_memory = model_memory.intercept_

# Step 5: Perform Linear Regression for non-used memory percentage (allocs * size vs. non-used memory percentage)
y_memory_percent = non_used_memory_percent
model_memory_percent = LinearRegression()
model_memory_percent.fit(X_memory, y_memory_percent)

slope_memory_percent = model_memory_percent.coef_[0]
intercept_memory_percent = model_memory_percent.intercept_

# Step 6: Perform Linear Regression for freeded chunks percentage
y_freeded_chunks = freeded_chunks_percent
model_freeded_chunks = LinearRegression()
model_freeded_chunks.fit(X_memory, y_freeded_chunks)

slope_freeded_chunks = model_freeded_chunks.coef_[0]
intercept_freeded_chunks = model_freeded_chunks.intercept_

# Step 7: Perform Linear Regression for non-freeded chunks percentage
y_non_freeded_chunks = non_freeded_chunks_percent
model_non_freeded_chunks = LinearRegression()
model_non_freeded_chunks.fit(X_memory, y_non_freeded_chunks)

slope_non_freeded_chunks = model_non_freeded_chunks.coef_[0]
intercept_non_freeded_chunks = model_non_freeded_chunks.intercept_

# Visualize the results
plt.figure(figsize=(18, 8))

# Plot 1: allocs * alloc size vs. elapsed time
plt.subplot(2, 3, 1)
plt.scatter(alloc_times, elapsed_time, color='blue')
plt.plot(alloc_times, model_time.predict(X_time), color='red', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Elapsed Time')
plt.title('Allocations * Alloc Size vs Elapsed Time')

# Plot 2: allocs * alloc size vs. non-used memory by user
plt.subplot(2, 3, 2)
plt.scatter(alloc_times, non_used_memory, color='green')
plt.plot(alloc_times, model_memory.predict(X_memory), color='orange', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Used Memory by User')
plt.title('Allocations * Alloc Size vs Non Used Memory')

# Plot 3: allocs * alloc size vs. non-used memory percentage
plt.subplot(2, 3, 3)
plt.scatter(alloc_times, non_used_memory_percent, color='purple')
plt.plot(alloc_times, model_memory_percent.predict(X_memory), color='pink', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Used Memory Percentage')
plt.title('Allocations * Alloc Size vs Non Used Memory Percentage')

# Plot 4: allocs * alloc size vs. freeded chunks percentage
plt.subplot(2, 3, 4)
plt.scatter(alloc_times, freeded_chunks_percent, color='cyan')
plt.plot(alloc_times, model_freeded_chunks.predict(X_memory), color='blue', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Freeded Chunks Percentage')
plt.title('Allocations * Alloc Size vs Freeded Chunks Percentage')

# Plot 5: allocs * alloc size vs. non-freeded chunks percentage
plt.subplot(2, 3, 5)
plt.scatter(alloc_times, non_freeded_chunks_percent, color='magenta')
plt.plot(alloc_times, model_non_freeded_chunks.predict(X_memory), color='yellow', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Freeded Chunks Percentage')
plt.title('Allocations * Alloc Size vs Non Freeded Chunks Percentage')

plt.tight_layout()

# Print the regression equations
print(f"Elapsed Time = {slope_time:.4f} * (n allocs * alloc size) + {intercept_time:.4f}")
print(f"Non Used Memory = {slope_memory:.4f} * (n allocs * alloc size) + {intercept_memory:.4f}")
print(f"Non Used Memory Percentage = {slope_memory_percent:.4f} * (n allocs * alloc size) + {intercept_memory_percent:.4f}")
print(f"Freeded Chunks Percentage = {slope_freeded_chunks:.4f} * (n allocs * alloc size) + {intercept_freeded_chunks:.4f}")
print(f"Non Freeded Chunks Percentage = {slope_non_freeded_chunks:.4f} * (n allocs * alloc size) + {intercept_non_freeded_chunks:.4f}")

plt.show()
