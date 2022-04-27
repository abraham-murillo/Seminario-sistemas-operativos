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
  return true;
}

char getch() {
  static Random random;
  char ch = random.get("ientc");
  cout << "Presionado " << ch << '\n';
  return ch;
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

void waitOnScreen(chrono::duration<long double> duration, bool clearScreen = false) {
  sleep_for(duration);
  if (clearScreen)
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

const string newProcess = "Nuevo";
const string completedProcess = "Terminado normalmente";
const string processWithError = "Terminado con error";
const string blockedProcess = "Bloqueado";
const string inQueue = "En cola";

struct Process {
  int id = -1;
  Operation operation;
  int maxExpectedTime = 0;
  FakeClock clock;
  FakeClock blockedClock;
  bool error = false;
  int finishedTime;
  optional<int> arrivalTime;
  optional<int> firstTimeInExecution;
  int returnTime;
  optional<int> responseTime;
  int waitTime;
  int serviceTime;
  string status = newProcess;
  int quantum = 0;

  void setError() {
    status = processWithError;
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

  void clear() {
    id = -1;
  }

  bool hasValue() {
    return id != -1;
  }

  string getStatus() {
    return status;
  }

  string getResult() {
    string res = operation.toString();
    if (status == completedProcess || status == processWithError) {
      res += " = ";
      res += result();
    } else {
      res += " = ?";
    }
    return res;
  }

  string getArrivalTime() {
    return arrivalTime.has_value() ? to_string(arrivalTime.value()) : "-";
  }

  string getFinishedTime() {
    return hasFinished() || hasError() ? to_string(finishedTime) : "-";
  }

  string getReturnTime() {
    return hasFinished() || hasError() ? to_string(returnTime) : "-";
  }

  string getWaitTime() {
    return hasFinished() || hasError() ? to_string(waitTime) : "-";
  }

  string getServiceTime() {
    return hasFinished() || hasError() ? to_string(serviceTime) : "-";
  }

  string getRemainingTime() {
    if (status == blockedProcess || status == inQueue)
      return to_string(remainingTime());
    return "-";
  }

  string getResponseTime() {
    return responseTime.has_value() ? to_string(responseTime.value()) : "-";
  }
};

Process randomProcess() {
  static int id = 0;
  static Random random;
  Process process;
  process.id = id++;
  process.maxExpectedTime = random.get<int>(6, 16);
  process.operation.a = random.get<int>(1, 100);
  process.operation.b = random.get<int>(1, 100);
  process.operation.op = random.get("+-*/%");
  return process;
}

using Processes = deque<Process>;

struct Handler {
  Processes all, ready, blocked, finished;
  FakeClock globalClock;
  Process inExecution;
  bool pause = false;
  bool showProcessesTable = false;
  int quantum = 0;

  void setQuantum(int quantum) {
    this->quantum = quantum;
  }

  void add(Process process) {
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
        blocked.front().status = inQueue;
        ready.push_back(blocked.front());
        if (!ready.back().arrivalTime.has_value())
          ready.back().arrivalTime = globalClock.currentTime();
        blocked.pop_front();
      } else {
        // Ninguno se ha desbloqueado
        break;
      }
    }

    // Agarrar más procesos si tengo menos de kMaxProcessesInMemory en memory
    while (numOfProcessesInMemory() < kMaxProcessesInMemory && all.size()) {
      ready.push_back(all.front());
      if (!ready.back().arrivalTime.has_value())
        ready.back().arrivalTime = globalClock.currentTime();
      all.pop_front();
    }
  }

  void updateTime() {
    if (!pause) {
      globalClock.secondsAgo++;
      if (inExecution.hasValue()) {
        inExecution.clock.secondsAgo++;
        inExecution.quantum++;
      }
      for (auto& process : blocked)
        process.blockedClock.secondsAgo++;
    }
  }

  void print() {
    if (showProcessesTable) {
      VariadicTable<int, string, string, string, string, string, string, string, string, string> processesTable({"Id",
                                                                                                                 "Estado del proceso",
                                                                                                                 "Operación y datos",
                                                                                                                 "Tiempo de llegada",
                                                                                                                 "Tiempo de finalización",
                                                                                                                 "Tiempo de retorno",
                                                                                                                 "Tiempo de espera",
                                                                                                                 "Tiempo de servicio",
                                                                                                                 "Tiempo restante en CPU",
                                                                                                                 "Tiempo de respuesta"});
      for (auto queue : {all, ready, blocked, finished})
        for (auto& process : queue) {
          processesTable.addRow(process.id,
                                process.getStatus(),
                                process.getResult(),
                                process.getArrivalTime(),
                                process.getFinishedTime(),
                                process.getReturnTime(),
                                process.getWaitTime(),
                                process.getServiceTime(),
                                process.getRemainingTime(),
                                process.getResponseTime());
        }

      if (inExecution.hasValue()) {
        processesTable.addRow(inExecution.id,
                              "En ejecución",
                              inExecution.getResult(),
                              inExecution.getArrivalTime(),
                              inExecution.getFinishedTime(),
                              inExecution.getReturnTime(),
                              inExecution.getWaitTime(),
                              inExecution.getServiceTime(),
                              inExecution.getRemainingTime(),
                              inExecution.getResponseTime());
      }
      processesTable.print();

    } else {
      println("Segundos transcurridos:", globalClock.currentTime());
      println("No. nuevos:", all.size());
      println("Quantum:", quantum);
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
          println(" Quantum restante:", quantum - inExecution.quantum);
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
          VariadicTable<int, string, string, int> finishedTable({
              "Id",
              "Operación",
              "Resultado",
              "Tiempo máximo estimado",
          });
          for (auto& process : finished) {
            finishedTable.addRow(process.id, process.operation.toString(), process.result(), process.maxExpectedTime);
          }
          finishedTable.print();
        }
      }
    }
  }

  void interruptProcess() {
    inExecution.blockedClock.reset();
    inExecution.status = blockedProcess;
    blocked.push_back(inExecution);
    inExecution.clear();
  }

  void solve() {
    globalClock.reset();

    while (all.size() || numOfProcessesInMemory()) {
      if (inExecution.quantum >= quantum) {
        inExecution.blockedClock.reset();
        inExecution.status = inQueue;
        ready.push_back(inExecution);
        inExecution.clear();
      }

      if (kbhit()) {
        char ch = getch();

        if (ch == 'i') {
          if (inExecution.hasValue()) {
            interruptProcess();
          }
        } else if (ch == 'e') {
          inExecution.setError();
        } else if (ch == 'p') {
          pause = true;
        } else if (ch == 'c') {
          pause = false;
          showProcessesTable = false;
        } else if (ch == 'n') {
          add(randomProcess());
        } else if (ch == 't') {
          showProcessesTable = true;
          pause = true;
        }
      }

      if (inExecution.hasValue() && (inExecution.hasError() || inExecution.hasFinished())) {
        // Ya terminó o tuvo un error
        if (!inExecution.hasError() && inExecution.hasFinished()) {
          inExecution.status = completedProcess;
        }

        inExecution.finishedTime = globalClock.currentTime();
        inExecution.returnTime = inExecution.finishedTime - inExecution.arrivalTime.value();
        inExecution.serviceTime = inExecution.finishedTime - inExecution.firstTimeInExecution.value();
        inExecution.waitTime = inExecution.firstTimeInExecution.value();
        finished.push_back(inExecution);
        inExecution.clear();
      }

      // Cargar processos en memoria
      loadProcessesInMemory();

      // Seleccionar uno de esos procesos para que esté en ejecución
      if (!inExecution.hasValue()) {
        if (ready.size()) {
          inExecution = ready.front();
          inExecution.quantum = 0;
          ready.pop_front();
          int now = globalClock.currentTime();
          if (!inExecution.responseTime.has_value())
            inExecution.responseTime = now - inExecution.arrivalTime.value();
          if (!inExecution.firstTimeInExecution.has_value())
            inExecution.firstTimeInExecution = now;
        }
      }

      print();
      waitOnScreen(1s, numOfProcessesInMemory() != 0 /* clearScreen */);
      updateTime();
    }

    VariadicTable<int, int, int, int, int, int, int> finishedTable(
        {"Id", "Tiempo de llegada", "Tiempo de finalización", "Tiempo de retorno", "Tiempo de respuesta", "Tiempo de espera", "Tiempo de servicio"});
    for (auto& process : finished) {
      finishedTable.addRow(process.id,
                           process.arrivalTime.value(),
                           process.finishedTime,
                           process.returnTime,
                           process.responseTime.value(),
                           process.waitTime,
                           process.serviceTime);
    }
    finishedTable.print();
  }
};

int main() {
  int numProcesses;
  int quantum;
  cout << "Número de procesos: ";
  cin >> numProcesses;
  cout << "Quantum (segundos): ";
  cin >> quantum;

  Handler handler;
  for (int id = 0; id < numProcesses; id++) {
    auto process = randomProcess();
    handler.add(process);
  }

  system("clear");

  handler.setQuantum(quantum);
  handler.solve();

  return 0;
}