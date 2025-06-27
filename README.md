# CPU Scheduling Algorithms Simulator

This project is a CPU Scheduling Algorithms Simulator implemented in C++. It supports multiple scheduling algorithms and outputs either detailed process execution timelines or summary statistics.

---

## Features

- Supports the following CPU scheduling algorithms:
  - First Come First Serve (FCFS)
  - Round Robin (RR) with configurable quantum
  - Shortest Process Next (SPN)
  - Shortest Remaining Time (SRT)
  - Highest Response Ratio Next (HRRN)
  - Feedback Queues (FB-1, FB-2i)
  - Aging

- Two operation modes:
  - `trace`: Prints detailed timeline of process execution
  - `stats`: Prints summary statistics including finish time, turnaround time, normalized turnaround time

- Input-driven design allowing flexible scheduling tests

---

## How to Compile

Use the following command to compile the project:

```bash
g++ main.cpp -o scheduler.exe -std=c++11

## Input Format

<operation_mode> <algorithm_list> <last_instant> <process_count>
<process_name>,<arrival_time>,<service_time>
<process_name>,<arrival_time>,<service_time>




- **operation_mode**: `trace` (to print timeline) or `stats` (to print summary statistics)
- **algorithm_list**: Comma-separated list of algorithms with optional quantum, e.g., `1,2-4,3`

### Algorithm IDs:

| ID | Algorithm                   | Notes                         |
|-----|-----------------------------|-------------------------------|
| 1   | FCFS                        | First Come First Serve         |
| 2   | RR                          | Round Robin (requires quantum, e.g. `2-4` means quantum = 4) |
| 3   | SPN                         | Shortest Process Next          |
| 4   | SRT                         | Shortest Remaining Time        |
| 5   | HRRN                        | Highest Response Ratio Next    |
| 6   | FB-1                        | Feedback Queue 1               |
| 7   | FB-2i                       | Feedback Queue 2i              |
| 8   | Aging                       | Aging algorithm                |

- **last_instant**: Total simulation time units (integer)
- **process_count**: Number of processes (integer)

Each process line consists of:

- Process name (string)
- Arrival time (integer)
- Service time (integer)

---

### Sample Input

trace 1,2-3,3 20 4
P1,0,4
P2,1,5
P3,2,2
P4,3,1


---

## Running the Program

Run the executable and provide the input file via redirection:

```bash
./scheduler.exe < input.txt


Output
If the operation mode is trace, the program prints the timeline of each process’ execution.

If the operation mode is stats, the program prints summary statistics including finish times, turnaround times, and normalized turnaround times for each process.

Project Structure
main.cpp — Main program with scheduling algorithms

parser.h — Input parsing and global data management

Author
Jiya Ranjan
Computer Science Undergraduate
Graphic Era University, Dehradun
