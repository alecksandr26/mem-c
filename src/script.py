import re
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures

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

# Step 3: Apply Polynomial Features (degree 2 for example)
poly = PolynomialFeatures(degree=2)
X_time_poly = poly.fit_transform(alloc_times.reshape(-1, 1))

# Step 4: Perform Polynomial Regression for time (allocs * size vs. time)
model_time_poly = LinearRegression()
model_time_poly.fit(X_time_poly, elapsed_time)

# Step 5: Perform Polynomial Regression for non-used memory
model_memory_poly = LinearRegression()
model_memory_poly.fit(X_time_poly, non_used_memory)

# Step 6: Perform Polynomial Regression for non-used memory percentage
model_memory_percent_poly = LinearRegression()
model_memory_percent_poly.fit(X_time_poly, non_used_memory_percent)

# Step 7: Perform Polynomial Regression for freeded chunks percentage
model_freeded_chunks_poly = LinearRegression()
model_freeded_chunks_poly.fit(X_time_poly, freeded_chunks_percent)

# Step 8: Perform Polynomial Regression for non-freeded chunks percentage
model_non_freeded_chunks_poly = LinearRegression()
model_non_freeded_chunks_poly.fit(X_time_poly, non_freeded_chunks_percent)

# Visualize the results
plt.figure(figsize=(18, 8))

# Plot 1: allocs * alloc size vs. elapsed time (Polynomial fit)
plt.subplot(2, 3, 1)
plt.scatter(alloc_times, elapsed_time, color='blue')
plt.plot(alloc_times, model_time_poly.predict(X_time_poly), color='red', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Elapsed Time')
plt.title('Allocations * Alloc Size vs Elapsed Time (Polynomial)')

# Plot 2: allocs * alloc size vs. non-used memory by user (Polynomial fit)
plt.subplot(2, 3, 2)
plt.scatter(alloc_times, non_used_memory, color='green')
plt.plot(alloc_times, model_memory_poly.predict(X_time_poly), color='orange', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Used Memory by User')
plt.title('Allocations * Alloc Size vs Non Used Memory (Polynomial)')

# Plot 3: allocs * alloc size vs. non-used memory percentage (Polynomial fit)
plt.subplot(2, 3, 3)
plt.scatter(alloc_times, non_used_memory_percent, color='purple')
plt.plot(alloc_times, model_memory_percent_poly.predict(X_time_poly), color='pink', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Used Memory Percentage')
plt.title('Allocations * Alloc Size vs Non Used Memory Percentage (Polynomial)')

# Plot 4: allocs * alloc size vs. freeded chunks percentage (Polynomial fit)
plt.subplot(2, 3, 4)
plt.scatter(alloc_times, freeded_chunks_percent, color='cyan')
plt.plot(alloc_times, model_freeded_chunks_poly.predict(X_time_poly), color='blue', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Freeded Chunks Percentage')
plt.title('Allocations * Alloc Size vs Freeded Chunks Percentage (Polynomial)')

# Plot 5: allocs * alloc size vs. non-freeded chunks percentage (Polynomial fit)
plt.subplot(2, 3, 5)
plt.scatter(alloc_times, non_freeded_chunks_percent, color='magenta')
plt.plot(alloc_times, model_non_freeded_chunks_poly.predict(X_time_poly), color='yellow', linewidth=2)
plt.xlabel('Allocations * Alloc Size')
plt.ylabel('Non Freeded Chunks Percentage')
plt.title('Allocations * Alloc Size vs Non Freeded Chunks Percentage (Polynomial)')

plt.tight_layout()

# Print the regression equations
print(f"Elapsed Time = Polynomial Fit")
print(f"Non Used Memory = Polynomial Fit")
print(f"Non Used Memory Percentage = Polynomial Fit")
print(f"Freeded Chunks Percentage = Polynomial Fit")
print(f"Non Freeded Chunks Percentage = Polynomial Fit")

plt.show()
