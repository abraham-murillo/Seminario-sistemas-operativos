#include "VariadicTable.h"
#include <bits/stdc++.h>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using namespace std;

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

#ifdef LOCAL
bool kbhit() {
  static Random random;
  return random.get<int>(0, 5) % 2;
}

char getch() {
  static Random random;
  return random.get("ie?");
}
#else
#include <conio.h>
#endif

const int kMaxProcessesInMemory = 5;
const int kMaxBlockedTime = 8;

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

string getTimeWithFormat(const time_t& time) {
  auto timeObject = localtime(&time);
  return to_string(timeObject->tm_hour) + ":" + to_string(timeObject->tm_min) + ":" + to_string(timeObject->tm_sec);
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
  int id = -1;
  Operation operation;
  int maxExpectedTime = 0;
  FakeClock clock;
  FakeClock blockedClock;
  bool error = false;
  time_t arrivalTime;
  time_t finishedTime;
  int processingTime_s;
  int responseTime_s;

  void setError() {
    error = true;
  }

  string result() {
    return hasError() ? "Error" : to_string(operation.result());
  }

  int remainingTime() {
    return max(0, maxExpectedTime - clock.currentTime());
  }

  bool hasError() {
    return error;
  }

  bool hasFinished() {
    return clock.currentTime() >= maxExpectedTime;
  }

  bool hasBeenUnblocked() {
    return blockedClock.currentTime() >= kMaxBlockedTime;
  }

  void reset() {
    id = -1;
  }

  bool hasValue() {
    return id != -1;
  }
};

using Processes = deque<Process>;

struct Handler {
  Processes all, ready, blocked, finished;
  FakeClock globalClock;
  Process inExecution;
  bool paused = false;

  void add(Process& process) {
    all.push_back(process);
  }

  int numOfProcessesInMemory() {
    return ready.size() + blocked.size() + inExecution.hasValue();
  }

  void loadProcessesInMemory() {
    // Cargar kMaxProcessesInMemory en memoria
    // Ve si algún proceso bloqueado ya puede entrar a los ready otra vez
    while (blocked.size()) {
      if (blocked.front().hasBeenUnblocked()) {
        // Ya está desbloqueado
        ready.push_back(blocked.front());
        blocked.pop_front();
      } else {
        // Ninguno se ha desbloqueado
        break;
      }
    }

    // Agarrar más procesos si tengo menos de kMaxProcessesInMemory en memory
    while (numOfProcessesInMemory() < kMaxProcessesInMemory && all.size()) {
      ready.push_back(all.front());
      all.pop_front();
    }
  }

  void updateTime() {
    if (!paused) {
      globalClock.secondsAgo++;
      if (inExecution.hasValue())
        inExecution.clock.secondsAgo++;
      for (auto& process : blocked)
        process.blockedClock.secondsAgo++;
    }
  }

  void print() {
    println("Segundos transcurridos:", globalClock.currentTime());
    println("No. nuevos:", all.size());
    println();

    {
      println("Cola de listos:");
      if (ready.empty()) {
        println(" - ");
      } else {
        VariadicTable<int, int, string, int> readyTable({"Id", "Tiempo máximo estimado", "Operación", "Tiempo en ejecución"});
        for (auto& process : ready) {
          readyTable.addRow(process.id, process.maxExpectedTime, process.operation.toString(), process.clock.currentTime());
        }
        readyTable.print();
      }
      println();
    }

    {
      println("Proceso en ejecución:");
      if (inExecution.hasValue()) {
        println(" Id:", inExecution.id);
        println(" Tiempo máximo estimado:", inExecution.maxExpectedTime);
        println(" Operación:", inExecution.operation.toString());
        println(" Tiempo en ejecución:", inExecution.clock.currentTime());
        println(" Tiempo restante por ejecutar:", inExecution.remainingTime());
      } else {
        println(" - ");
      }
      println();
    }

    {
      println("Cola de bloqueados:");
      if (blocked.empty()) {
        println(" - ");
      } else {
        VariadicTable<int, int> blockedTable({"Id", "Tiempo transcurrido bloqueado"});
        for (auto& process : blocked) {
          blockedTable.addRow(process.id, process.blockedClock.currentTime());
        }
        blockedTable.print();
      }
      println();
    }

    {
      println("Procesos terminados:");
      if (finished.empty()) {
        println(" - ");
      } else {
        VariadicTable<int, string, string, int, int, string, string, int, int> finishedTable({"Id",
                                                                                              "Operación",
                                                                                              "Resultado",
                                                                                              "Tiempo máximo estimado",
                                                                                              "Reloj",
                                                                                              "Tiempo de llegada",
                                                                                              "Tiempo de finalización",
                                                                                              "Tiempo de retorno",
                                                                                              "Tiempo de respuesta"});
        for (auto& process : finished) {
          finishedTable.addRow(process.id,
                               process.operation.toString(),
                               process.result(),
                               process.maxExpectedTime,
                               process.clock.currentTime(),
                               getTimeWithFormat(process.arrivalTime),
                               getTimeWithFormat(process.finishedTime),
                               process.processingTime_s,
                               process.responseTime_s);
        }
        finishedTable.print();
      }
    }
  }

  void solve() {
    globalClock.reset();

    while (all.size() || numOfProcessesInMemory()) {
      if (kbhit()) {
        char ch = getch();

        if (ch == 'i') {
          if (inExecution.hasValue()) {
            inExecution.blockedClock.reset();
            blocked.push_back(inExecution);
            inExecution.reset();
          }
        } else if (ch == 'e') {
          inExecution.setError();
        } else if (ch == 'p') {
          paused = true;
        } else if (ch == 'c') {
          paused = false;
        }
      }

      if (inExecution.hasValue() && (inExecution.hasError() || inExecution.hasFinished())) {
        // Ya terminó o tuvo un error
        inExecution.finishedTime = time(0);
        inExecution.processingTime_s = inExecution.finishedTime - inExecution.arrivalTime;
        finished.push_back(inExecution);
        inExecution.reset();
      }

      // Cargar processos en memoria
      loadProcessesInMemory();

      // Seleccionar uno de esos procesos para que esté en ejecución
      if (!inExecution.hasValue()) {
        if (ready.size()) {
          inExecution = ready.front();
          ready.pop_front();
          time_t now = time(0);
          inExecution.responseTime_s = now - inExecution.arrivalTime;
        }
      }

      print();
      waitOnScreen(1s);
      updateTime();
    }

    print();
  }
};

int main() {
  int numProcesses;
  cout << "Número de procesos: ";
  cin >> numProcesses;

  Handler handler;
  Random random;
  for (int id = 0; id < numProcesses; id++) {
    Process process;
    process.id = id;
    process.maxExpectedTime = random.get<int>(6, 16);
    process.operation.a = random.get<int>(1, 100);
    process.operation.b = random.get<int>(1, 100);
    process.operation.op = random.get("+-*/%");
    process.arrivalTime = time(0); // current time
    handler.add(process);
  }

  system("clear");

  handler.solve();

  return 0;
}
