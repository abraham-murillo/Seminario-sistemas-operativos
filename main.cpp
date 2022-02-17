#include "VariadicTable.h"
#include <bits/stdc++.h>
#include <conio.h>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using namespace std;

const int MAX_PROCESSES = 5;

struct Random {
  mt19937 rng;

  Random() : rng(chrono::steady_clock::now().time_since_epoch().count()) {}

  template <class T>
  T get(T l, T r) {
    return uniform_int_distribution<T>(l, r)(rng);
  }

  char get(string s) {
    shuffle(s.begin(), s.end(), rng);
    return s[0];
  }
};

template <class... Args>
void print(Args&&... args) {
  ((cout << args << " "), ...);
}

template <class... Args>
void println(Args&&... args) {
  print(args...);
  cout << '\n';
}

void waitOnScreen(chrono::duration<long double> duration) {
  sleep_for(duration);
  system("clear");
}

struct FakeClock {
  int secondsAgo = 0;

  int currentTime() {
    return secondsAgo;
  }

  void reset() {
    secondsAgo = 0;
  }
};

struct Operation {
  long long a, b;
  char op;

  bool valid() {
    if (op == '/' || op == '%') {
      if (b == 0) {
        cout << "Operación inválida, ingrese una nueva: ";
      }
      return b != 0;
    }
    return true;
  }

  string toString() const {
    return to_string(a) + " " + op + " " + to_string(b);
  }

  double result() const {
    if (op == '+')
      return a + b;
    if (op == '-')
      return a - b;
    if (op == '*')
      return a * b;
    if (op == '/')
      return 1.0 * a / b;
    if (op == '%')
      return a % b;
    exit(0);
    return 0;
  }
};

struct Process {
  Operation operation;
  int maxExpectedTime = 0;
  int id = 0;
  int batchId = -1;
  FakeClock clock;
  bool hasError = false;

  void error() {
    hasError = true;
  }

  string result() {
    return hasError ? "Error" : to_string(operation.result());
  }

  int remainingTime() {
    return max(0, maxExpectedTime - clock.currentTime());
  }
};

using Batch = deque<Process>;

struct BatchHandler : public deque<Batch> {
  Batch finished;
  FakeClock globalClock;

  void add(Process& process) {
    if (!empty() && back().size() < MAX_PROCESSES) {
      back().push_back(process);
    } else {
      push_back(Batch{{process}});
    }
    // Set the curretn batchId for the process
    back().back().batchId = size();
  }

  void print(Batch&& runningBatch, optional<Process> inExecution, Batch& finished) {
    println("Segundos transcurridos:", globalClock.currentTime());
    println("Lotes pendientes:", this->size());
    println();

    {
      VariadicTable<int, int, string, int> runningBatchTable({"Id", "Tiempo máximo estimado", "Operación", "Tiempo en ejecución"});
      println("Lote en ejecución:");
      for (auto& process : runningBatch) {
        runningBatchTable.addRow(process.id, process.maxExpectedTime, process.operation.toString(), process.clock.currentTime());
      }
      runningBatchTable.print();
      println();
    }

    {
      println("Proceso en ejecución:");
      if (inExecution.has_value()) {
        println(" Id:", inExecution.value().id);
        println(" Tiempo máximo estimado:", inExecution.value().maxExpectedTime);
        println(" Operación:", inExecution.value().operation.toString());
        println(" Tiempo en ejecución:", inExecution.value().clock.currentTime());
        println(" Tiempo restante por ejecutar:", inExecution.value().remainingTime());
      } else {
        println(" Cargando...");
      }
      println();
    }

    {
      println("Procesos terminados:");
      if (finished.empty()) {
        println(" No hay procesos terminados.");
      } else {
        VariadicTable<int, string, string, int> finishedProcessesTable({"Id", "Operación", "Resultado", "Lote"});
        for (auto& process : finished) {
          finishedProcessesTable.addRow(process.id, process.operation.toString(), process.result(), process.batchId);
        }
        finishedProcessesTable.print();
      }
    }
  }

  void solve() {
    globalClock.reset();

    while (!empty()) {
      Batch runningBatch = front();
      pop_front();

      while (!runningBatch.empty()) {
        Process inExecution = runningBatch.front();
        // Si tiene error algún proceso, mandar llamar process.error()

        print(move(runningBatch), /* inExecution */ nullopt, finished);
        waitOnScreen(0.3s);

        bool paused = false;
        runningBatch.pop_front();
        while (inExecution.clock.currentTime() < inExecution.maxExpectedTime) {

          if (kbhit()) {
            char ch = getch();

            if (ch == 'i') {
              runningBatch.push_back(inExecution);
              break;
            } else if (ch == 'e') {
              inExecution.error();
              break;
            } else if (ch == 'p') {
              paused = true;
            } else if (ch == 'c') {
              paused = false;
            }
          }

          print(move(runningBatch), inExecution, finished);
          waitOnScreen(1s);

          if (!paused) {
            globalClock.secondsAgo++;
            inExecution.clock.secondsAgo++;
          }
        }

        if (inExecution.hasError || inExecution.clock.currentTime() >= inExecution.maxExpectedTime) {
          finished.emplace_back(inExecution);
        }
      }
    }

    print(/* runningBatch */ {}, /* inExecution */ nullopt, finished);
  }
};

int main() {
  int numProcesses;
  cout << "Número de procesos: ";
  cin >> numProcesses;

  BatchHandler handler;
  Random random;
  for (int id = 0; id < numProcesses; id++) {
    Process process;
    process.id = id;
    process.maxExpectedTime = random.get<int>(6, 16);
    process.operation.a = random.get<int>(1, 100);
    process.operation.b = random.get<int>(1, 100);
    process.operation.op = random.get("+-*/%");
    handler.add(process);
  }

  system("clear");

  handler.solve();

  return 0;
}
