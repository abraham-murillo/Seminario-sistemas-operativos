#include <bits/stdc++.h>
using namespace std;

const int MAX_PROCESSES = 5;

struct Process {
  string programmerName = "";
  string equation = "";
  int maxExpectedTime_s = 0;
  int id = 0;
};

struct Batch : public vector<Process> {
  int id = 0;
};

void calc(const string& equation) {}

void handle(const Batch& batch) {
  auto start_time = clock();

  for (const Process& process : batch) {
    calc(process.equation);
  }

  auto end_time = clock();
  cerr << "Execution time: " << (end_time - start_time) * (int)1e3 / CLOCKS_PER_SEC << " ms\n";
}

int main() {
  queue<Batch> qu;
  int numProcesses;
  cout << "Number of processes: ";
  cin >> numProcesses;
  set<int> ids;

  for (int i = 0; i < numProcesses; i++) {
    Process process;
    cout << "Programmer name: ";
    cin >> process.programmerName;
    cout << "Equation: ";
    cin >> process.equation;
    cout << "Maximum expected time: ";
    cin >> process.maxExpectedTime_s;

    do {
      cout << "ID: ";
      cin >> process.id;
    } while (ids.count(process.id));

    if (!qu.empty() && qu.front().size() < MAX_PROCESSES)
      qu.front().push_back(process);
    else {
      qu.front() = Batch{{process}};
      qu.front().id = qu.size() + 1;
    }
  }

  int numActiveProcesses = 0;

  while (!qu.empty()) {
    Batch batch = qu.front();
    qu.pop();
    handle(batch);
  }

  return 0;
}
