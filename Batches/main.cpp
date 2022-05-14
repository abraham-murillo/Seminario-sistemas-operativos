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
  return true;
}

int now = 0;

char getch() {
  const string wut = "iisisracracrac";
  char ch = wut[now++];
  now %= int(wut.size());
  cout << "Presionado (" << now - 1 << ") " << ch << '\n';
  return ch;
}

#else
#include <conio.h>
#endif

const int kMaxBlockedTime = 8;
const int kNumberOfFrames = 50;
const int kFrameSize = 4;
const int kFramesUsedByOS = 4;
const int kAmountOfAvailableMemory = 200;

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

int myCeil(int x, int y) {
  return (x + (y - 1)) / y;
}

struct Frame {
  int memoryUsed = 0;
  int processId = 0;
};

Frame frames[kNumberOfFrames];

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
  int memorySize = 0;
  int paginatedMemory = 0;

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

map<int, Process> lazy;

istream& operator>>(istream& is, Process& p) {
  is >> p.id;
  p = lazy[p.id];
  lazy.erase(p.id);
  return is;
}

ostream& operator<<(ostream& os, const Process& p) {
  os << setw(5) << p.id;
  return os;
}

Process randomProcess() {
  static int id = 0;
  static Random random;
  Process process;
  process.id = id++;
  process.maxExpectedTime = random.get<int>(6, 16);
  process.operation.a = random.get<int>(1, 100);
  process.operation.b = random.get<int>(1, 100);
  process.operation.op = random.get("+-*/%");
  process.memorySize = random.get<int>(5, 28);
  return process;
}

using Processes = deque<Process>;

struct Handler {
  Processes all, ready, blocked, finished;
  FakeClock globalClock;
  Process inExecution, returnProcessFromDisk;
  bool pause = false;
  bool showProcessesTable = false;
  bool showMemoryTable = false;
  bool suspendIfAny = false;
  bool returnIfPossible = false;
  int suspended = 0;
  int quantum = 0;
  int memoryUsed = 0;

  void setQuantum(int quantum) {
    this->quantum = quantum;
  }

  void add(Process process) {
    all.push_back(process);
  }

  int numOfProcessesInMemory() {
    return ready.size() + blocked.size() + inExecution.hasValue();
  }

  bool canFitProcessIntoMemory(const Process& process) {
    return kAmountOfAvailableMemory - memoryUsed >= process.memorySize;
  }

  int memoryNeeded(const Process& process) {
    return myCeil(process.memorySize, kFrameSize) * kFrameSize;
  }

  void clearMemory(Process& process) {
    // Liberas la memoria del proceso en ejecución
    memoryUsed -= memoryNeeded(process);
    process.paginatedMemory = 0;
    for (int i = 0; i < kNumberOfFrames; i++) {
      if (frames[i].processId == process.id) {
        frames[i].processId = 0;
        frames[i].memoryUsed = 0;
      }
    }
  }

  void assignMemory(Process& process) {
    // Ensartar proceso en memoria disponible
    memoryUsed += memoryNeeded(process);
    process.paginatedMemory = 0;
    for (int i = 0; i < kNumberOfFrames; i++) {
      if (frames[i].memoryUsed == 0 && process.memorySize - process.paginatedMemory > 0) {
        int memoryToAllocate = min(kFrameSize, process.memorySize - process.paginatedMemory);
        frames[i].memoryUsed += memoryToAllocate;
        frames[i].processId = process.id;
        process.paginatedMemory += memoryToAllocate;
      }
    }
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

    // Agarrar más procesos a listos siempre y cuando el siguiente proceso pueda caber en memoria
    while (!all.empty() && canFitProcessIntoMemory(all.front())) {
      ready.push_back(all.front());
      // memoryUsed += myCeil(all.front().memorySize, kFrameSize) * kFrameSize;
      // // Ensartar proceso en memoria disponible
      // for (int i = 0; i < kNumberOfFrames; i++) {
      //   if (frames[i].memoryUsed == 0 && ready.back().memorySize - ready.back().paginatedMemory > 0) {
      //     int memoryToAllocate = min(kFrameSize, ready.back().memorySize - ready.back().paginatedMemory);
      //     frames[i].memoryUsed += memoryToAllocate;
      //     frames[i].processId = ready.back().id;
      //     ready.back().paginatedMemory += memoryToAllocate;
      //   }
      // }

      assignMemory(ready.back());

      if (ready.back().paginatedMemory != ready.back().memorySize) {
        cerr << ready.back().id << '\n';
        cerr << ready.back().paginatedMemory << '\n';
        cerr << ready.back().memorySize << '\n';
        cerr << '\n';
      }

      assert(ready.back().paginatedMemory == ready.back().memorySize);

      if (!ready.back().arrivalTime.has_value())
        ready.back().arrivalTime = globalClock.currentTime();
      all.pop_front();
    }
  }

  bool exists(int id) {
    for (int i = 0; i < kNumberOfFrames; i++)
      if (frames[i].processId == id)
        return true;
    return false;
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

    } else if (showMemoryTable) {
      VariadicTable<int, string, string, int, string, string> memoryTable(
          {"Marco", "Memoria consumida", "ID de proceso", "Marco", "Memoria consumida", "ID de proceso"});

      for (int id = 0; id < kNumberOfFrames; id += 2)
        memoryTable.addRow(id,
                           to_string(frames[id].memoryUsed) + "/" + to_string(kFrameSize),
                           (id < kFramesUsedByOS         ? "OS"
                            : frames[id].memoryUsed == 0 ? "Libre"
                                                         : to_string(frames[id].processId)),
                           id + 1,
                           to_string(frames[id + 1].memoryUsed) + "/" + to_string(kFrameSize),
                           (id + 1 < kFramesUsedByOS         ? "OS"
                            : frames[id + 1].memoryUsed == 0 ? "Libre"
                                                             : to_string(frames[id + 1].processId)));

      memoryTable.print();
    } else {
      println("Segundos transcurridos:", globalClock.currentTime());
      println("No. nuevos:", all.size());
      println("Quantum:", quantum);
      println("Memoria utilizada:", memoryUsed, "MB");
      println("Memoria disponible:", kAmountOfAvailableMemory - memoryUsed, "MB");
      println();
      println("Procesos suspendidos:", suspended);
      println();

      {
        if (returnProcessFromDisk.id >= -1) {
          if (returnProcessFromDisk.id == -1) {
            println("Se intentó regresar un proceso de disco pero no había");
          } else {
            println("Proceso regresado de disco:");
            println(" Id:", abs(returnProcessFromDisk.id));
            println(" Memoria:", returnProcessFromDisk.memorySize);
            println(" Status:", exists(returnProcessFromDisk.id) ? "agregado a listos" : "no cabe en memoria");
          }
        }
        println();
      }

      {
        println("Cola de listos:");
        if (ready.empty()) {
          println(" - ");
        } else {
          VariadicTable<int, int, string, int, int> readyTable({"Id", "Tiempo máximo estimado", "Operación", "Tiempo en ejecución", "Tamaño"});
          for (auto& process : ready) {
            readyTable.addRow(process.id, process.maxExpectedTime, process.operation.toString(), process.clock.currentTime(), process.memorySize);
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
          println(" Memoria:", inExecution.memorySize);
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

    // Seguir procesando hasta que se termine la memoria y los procesos
    while (!all.empty() || memoryUsed > 0) {
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
          showMemoryTable = false;
        } else if (ch == 'a') {
          pause = true;
          showProcessesTable = false;
          showMemoryTable = true;
        } else if (ch == 'n') {
          add(randomProcess());
        } else if (ch == 't') {
          pause = true;
          showProcessesTable = true;
          showMemoryTable = false;
        } else if (ch == 's') {
          suspendIfAny = true;
        } else if (ch == 'r') {
          returnIfPossible = true;
        } else if (ch == 'x') {
          exit(0);
        }
      }

      // Ya terminó o tuvo un error
      if (inExecution.hasValue() && (inExecution.hasError() || inExecution.hasFinished())) {
        if (!inExecution.hasError() && inExecution.hasFinished()) {
          inExecution.status = completedProcess;
        }

        // // Liberas la memoria del proceso en ejecución
        // memoryUsed -= myCeil(inExecution.memorySize, kFrameSize) * kFrameSize;

        // for (int i = 0; i < kNumberOfFrames; i++) {
        //   if (frames[i].processId == inExecution.id) {
        //     frames[i].processId = 0;
        //     frames[i].memoryUsed = 0;
        //   }
        // }

        clearMemory(inExecution);

        inExecution.finishedTime = globalClock.currentTime();
        inExecution.returnTime = inExecution.finishedTime - inExecution.arrivalTime.value();
        inExecution.serviceTime = inExecution.finishedTime - inExecution.firstTimeInExecution.value();
        inExecution.waitTime = inExecution.firstTimeInExecution.value();
        finished.push_back(inExecution);
        inExecution.clear();
      }

      // Si algún proceso está en bloqueados pasa a ser suspendido
      if (suspendIfAny) {
        if (blocked.size() > 0) {
          fstream disk("disk.txt", fstream::app);
          Process current = blocked.front();
          blocked.pop_front();

          disk << current;
          disk.close();

          lazy[current.id] = current;
          clearMemory(current);
          suspended++;
        }
        suspendIfAny = false;
      }

      // Le ponemos esto para decir que no se intentó regresar a nadie
      returnProcessFromDisk.id = -2;

      // Si hay procesos suspendidos regresarlos a la memoria
      if (returnIfPossible) {
        if (suspended > 0) {
          fstream disk("disk.txt", fstream::in | fstream::out);
          Process current;
          int lastPositionRead = 0;
          do {
            lastPositionRead = disk.tellg();
            disk >> current;
          } while (current.id == -1);

          returnProcessFromDisk = current;
          assert(current.id != -1);

          // Si hay alguno disponible
          if (canFitProcessIntoMemory(current)) {
            // Hay que marcarlo como leído, para eso sólo cambiamos el id a -1
            disk.seekp(lastPositionRead);
            cout << "Escribe en " << lastPositionRead << " -1\n";
            disk << setw(5) << -1;

            // Lo regreso a la cola de listos y asigno memoria para el proceso
            ready.push_back(current);
            assignMemory(current);
            suspended--;
          }

          disk.close();
        } else {
          // No hay procesos para regresar
          returnProcessFromDisk.id = -1;
        }

        returnIfPossible = false;
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
      waitOnScreen(3s, numOfProcessesInMemory() != 0 /* clearScreen */);
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
  // Limpiar el disco
  ofstream disk("disk.txt");
  disk.close();

  int numProcesses;
  int quantum;
  cout << "Número de procesos: ";
  cin >> numProcesses;
  cout << "Quantum (segundos): ";
  cin >> quantum;
  Handler handler;

  // Asignar memoria al sistema operativo
  for (int i = 0; i < kFramesUsedByOS; i++) {
    frames[i].memoryUsed = kFrameSize;
    frames[i].processId = -1;
    handler.memoryUsed += kFrameSize;
  }

  for (int id = 0; id < numProcesses; id++) {
    auto process = randomProcess();
    handler.add(process);
  }

  system("clear");

  handler.setQuantum(quantum);
  handler.solve();

  return 0;
}
