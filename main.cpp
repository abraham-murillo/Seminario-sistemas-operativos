#include <bits/stdc++.h>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using namespace std;

const int MAX_PROCESSES = 5;

template <class... Args>
void print(Args&&... args) {
  ((cout << args << " "), ...);
}

template <class... Args>
void println(Args&&... args) {
  print(args...);
  cout << '\n';
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

  friend istream& operator>>(istream& is, Operation& op) {
    do {
      // 5 + 6, 7 / 10, ...
      is >> op.a >> op.op >> op.b;
    } while (!op.valid());
    return is;
  }

  string toString() const {
    return to_string(a) + " " + op + " " + to_string(b);
  }

  double getResult() const {
    if (op == '+')
      return a + b;
    if (op == '-')
      return a - b;
    if (op == '*')
      return a * b;
    if (op == '/')
      return a / b;
    if (op == '%')
      return a % b;

    println("Operación inválida", op);
    exit(0);
    return 0;
  }
};

struct Process {
  string programmerName = "";
  Operation operation;
  int maxExpectedTime = 0;
  int id = 0;
};

struct Batch : public deque<Process> {
  int id = 0;
};

struct BatchHandler : public deque<Batch> {
  vector<Process> finished;
  FakeClock globalClock;
  int lastId = 0;

  void add(Process process) {
    if (!empty() && back().size() < MAX_PROCESSES)
      back().push_back(process);
    else {
      push_back(Batch{{process}});
      back().id = lastId++;
    }
  }

  void finishFirstProcess(Batch& batch) {
    if (size()) {
      finished.emplace_back(batch.front());
      batch.pop_front();
    }
  }

  void printFinishedProcesses() {
    if (finished.empty()) {
      println(" No hay procesos terminados");
    } else {
      println("Procesos Terminados: [");
      for (auto& process : finished) {
        println(" Número de programa:",
                process.id,
                ", Nombre del programador:",
                process.programmerName,
                ", Operación:",
                process.operation.toString(),
                ", Tiempo Máximo Estimado:",
                process.maxExpectedTime,
                ", Resultado:",
                process.operation.getResult(),
                ", Lote:",
                int(1 + cnt / 5));
      }
      println("]\n");
    }
  }
  
  void print() {

    if (size()) {
      auto nextBatch = front();

      while (!nextBatch.empty()) {
        Process execution = nextBatch.front();

        FakeClock tmp;
        while (tmp.currentTime() < execution.maxExpectedTime) {
          println("Segundos transcurridos:", globalClock.currentTime());
          println("Número de lotes pendientes:", size() - 1);
          println();

          println("Lote en Ejecución: [");
          for (auto& process : nextBatch) {
            println(" Número de programa:", process.id, ", Tiempo máximo estimado:", process.maxExpectedTime);
          }
          println("]\n");

          println("Proceso en ejecución:");
          println("- Nombre del programador:", execution.programmerName);
          println("- Operación:", execution.operation.toString());
          println("- Tiempo Máximo Estimado:", execution.maxExpectedTime);
          println("- Número del programa:", execution.id);
          println("- Tiempo transcurrido en ejecución:", tmp.currentTime());
          println("- Tiempo restante por ejecutar:", execution.maxExpectedTime - tmp.currentTime());
          println();

          printFinishedProcesses();

          // Wait for 1s
          sleep_for(1s);
          system("clear");

          globalClock.secondsAgo++;
          tmp.secondsAgo++;
        }

        finishFirstProcess(nextBatch);
      }

      pop_front();
    } else {
      println("Segundos transcurridos:", globalClock.currentTime());
      println("No hay lotes pendientes");
      println();
      printFinishedProcesses();
    }
  }
};

int main() {
  int numProcesses;
  cout << "Número de procesos: ";
  cin >> numProcesses;

  set<int> ids;
  BatchHandler handler;
  for (int i = 0; i < numProcesses; i++) {
    Process process;
    cout << "Nombre del programador: ";
    cin >> process.programmerName;
    cout << "Ecuación: ";
    cin >> process.operation;
    cout << "Tiempo Máximo Esperado: ";
    cin >> process.maxExpectedTime;

    do {
      cout << "Id: ";
      cin >> process.id;
    } while (ids.count(process.id));

    ids.insert(process.id);

    handler.add(process);
  }

  system("clear");

  handler.globalClock.reset();
  while (!handler.empty()) {
    handler.print();
  }
  handler.print();

  return 0;
}
