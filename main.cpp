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
  int batchId = 0;
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

    back().back().batchId = size();
  }

  void print(Batch&& current, optional<Process> execution, Batch& finished, FakeClock& tmp) {
    println("Segundos transcurridos:", globalClock.currentTime());
    println("Número de lotes pendientes:", size());
    println();

    println("Lote en ejecución: ");
    for (auto& process : current) {
      println(" Número de programa:", process.id, ", Tiempo máximo estimado:", process.maxExpectedTime);
    }
    println();

    println("Proceso en ejecución:");
    if (execution.has_value()) {
      println(" Nombre del programador:", execution.value().programmerName);
      println(" Operación:", execution.value().operation.toString());
      println(" Tiempo máximo estimado:", execution.value().maxExpectedTime);
      println(" Número del programa:", execution.value().id);
      println(" Tiempo transcurrido en ejecución:", tmp.currentTime());
      println(" Tiempo restante por ejecutar:", execution.value().maxExpectedTime - tmp.currentTime());
    } else {
      println(" Cargando...");
    }
    println();

    println("Procesos terminados:");
    if (finished.empty()) {
      println(" No hay procesos terminados.");
    } else {
      for (auto& process : finished) {
        println(" Número de programa:",
                process.id,
                ", Nombre del programador:",
                process.programmerName,
                ", Operación:",
                process.operation.toString(),
                ", Tiempo máximo estimado:",
                process.maxExpectedTime,
                ", Resultado:",
                process.operation.getResult(),
                ", Lote:",
                process.batchId);
      }
    }
  }

  void solve() {
    globalClock.reset();

    FakeClock tmp;
    while (!empty()) {
      Batch current = front();
      pop_front();

      while (!current.empty()) {
        Process execution = current.front();

        tmp.reset();
        print(/* Batch */ move(current), /* execution */ nullopt, /* finished */ finished, /* clock */ tmp);
        waitOnScreen(0.3s);

        current.pop_front();
        while (tmp.currentTime() < execution.maxExpectedTime) {
          print(/* Batch */ move(current), /* execution */ execution, /* finished */ finished, /* clock */ tmp);
          waitOnScreen(1s);
          globalClock.secondsAgo++;
          tmp.secondsAgo++;
        }

        finished.emplace_back(execution);
      }
    }

    tmp.reset();
    print(/* Batch */ {}, /* execution */ nullopt, /* finished */ finished, /* clock */ tmp);
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

  handler.solve();

  return 0;
}
