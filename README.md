
# Downloaded gguf file from: 
https://huggingface.co/TheBloke/deepseek-coder-6.7B-instruct-GGUF


# actually_smart_pointer

> The only smart pointer that truly deserves the name.

Unlike `std::shared_ptr`, which blindly follows reference counts, `actually_smart_pointer` is *actually smart*.
It uses a local LLM (via [llama.cpp](https://github.com/ggerganov/llama.cpp)) to monitor pointer usage, learn your coding patterns, and decide intelligently when memory should be freed.

---

You may ask: why should I use your pointer while it sucks every possible way to the standard smart pointers? 
I will argue: are your "smart" pointers actually smart?

Can you "smart pointer" write a symphony?


```c++
int main() {
    asp::actually_smart_pointer<int> p(new int(5));
    std::cout << p.ask("Write a symphony")<< std::endl;
}
```

Response:
```
```


Can your "smart" pointer turn a canvas into a masterpiece?


Can your "smart" pointer write a bubble sort?

```c++
int main() {
    asp::actually_smart_pointer<int> p(new int(5));
    std::cout << p.ask("Write bubble sort in c++.") << std::endl;
}
```

Response:
```
A: Here is a simple implementation of Bubble Sort in C++:

#include<iostream>
using namespace std;

void bubbleSort(int arr[], int n) {
   for(int i = 0; i < n-1; i++) {
       for (int j = 0; j < n-i-1; j++) {
           if (arr[j] > arr[j+1]) {
              // Swap arr[j] and arr[j+1]
              int temp = arr[j];
              arr[j] = arr[j+1];
              arr[j+1] = temp;
           }
       }
   }
}

void printArray(int arr[], int size) {
   for (int i = 0; i < size; i++)
       cout<<arr[i]<<" ";
   cout<<endl;
}

int main() {
   int data[] = {-2, 45, 0, 11, -9};
   int size = sizeof(data)/sizeof(data[0]);
   bubbleSort(data, size);
   cout<<"Sorted Array in Ascending Order:\n";
   printArray(data, size);
}
```

gnu-time:
```
Command being timed: "./example"
...
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:17.41
...
Maximum resident set size (kbytes): 357220
...
```

(all of these are done using the small 4GB model described below. As for the hardware mbp M2 is used).
## Features

- **LLM-Powered Deallocation**: decides whether to free memory based on a conversation history of method calls (`copy()`, `release()`, etc.).
- **Pointer with memory**: remembers past operations and adapts its behavior accordingly.
- **Debug Logging**: see all LLM prompts and responses per pointer (optional).
- **Benchmarking**: compare against `std::shared_ptr` in time and memory.

---

## Getting Started

### Clone the Repo
```bash
git clone https://github.com/lll-phill-lll/actually_smart_pointer.git
cd actually_smart_pointer
```

### 💡 Prerequisites
- CMake 3.16+
- C++17 compiler
- Python 3 (for visualization)
- [gguf-format LLM model](https://huggingface.co/collections/ggml/gguf-65c37ccab1d3c35d6976c582)

We recommend [deepseek-coder-6.7b-instruct.Q4_K_M.gguf](https://huggingface.co/deepseek-ai/deepseek-coder-6.7B-instruct-GGUF).

Download using `wget` or `curl`:
```bash
mkdir -p models
wget -O models/deepseek-coder-6.7b-instruct.Q4_K_M.gguf \
  https://huggingface.co/deepseek-ai/deepseek-coder-6.7B-instruct-GGUF/resolve/main/deepseek-coder-6.7b-instruct.Q4_K_M.gguf
```

I used this model. It gives pretty poor results controlling the refcounting (say hi to memleaks and segfaults) but it uses only about 300Mb of RAM and 4GB of disk space. 

---

## Build the Project

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

To run the example:
```bash
./example
```

---

## Enable Debug Logs

To see all LLM prompts and responses (per-pointer):
```bash
cmake .. -DLLM_DEBUG_LOG=ON
cmake --build .
./example
```

This logs each pointer's lifecycle. Model responses if deleting is required:
```
[LLM][0x7ffd6a1c40a0] actually_smart_pointer::copy_constructor
[LLM][0x7ffd6a1c40a0] Prompt:
Q: copy()
A: false
```

---

## Run Benchmarks

Build and run the two benchmarks:
```bash
cmake --build . --target benchmark_std benchmark_asp
./benchmark_std
./benchmark_asp
```

This generates:
```
benchmark_std.csv
benchmark_asp.csv
```

---

## Visualize Results

Run the plotting script from the project root:
```bash
python3 ../plot_benchmark.py
```

It will generate:
- `benchmark_time.png`
- `benchmark_memory.png`

---

## Design Notes

Each `actually_smart_pointer` contains a `control_block` shared by copies. The block:
- Stores the actual object
- Maintains a string history of all method calls (e.g. `copy()`, `release()`)
- Sends that history to the LLM to decide whether to delete

The LLM prompt looks like:
```
You act as a custom shared_ptr implementation responsible for managing memory via reference counting.
I will report the names of methods called on the smart pointer.
Based on the sequence of operations, respond only with "true" if the memory should be deleted, or "false" if it should not.
```

---

## Why?

Because it's time our smart pointers were... actually smart.

This project is a satire and a tool to explore human-like memory management logic, deterministic prompt engineering, and llama.cpp integration.



