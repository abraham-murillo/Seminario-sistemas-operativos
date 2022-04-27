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
};

void waitOnScreen(chrono::duration<long double> duration, bool clearScreen = false) {
  sleep_for(duration);
  if (clearScreen)
    system("clear");
}

const int CAPACITY = 20;

struct Container {
  int producerPos = 0;
  int consumerPos = 0;
  vector<int> vis;

  Container() : vis(CAPACITY, 0) {}

  int total() {
    int cnt = 0;
    for (int x : vis)
      cnt += (x > 0);
    return cnt;
  }

  int maxToProduce() {
    return CAPACITY - total();
  }

  int maxToConsume() {
    return total();
  }

  bool full() {
    return total() == vis.size();
  }

  bool empty() {
    return total() == 0;
  }

  void print() {
    for (int x : vis)
      cout << setw(3) << (x ? "*" : "");
    cout << '\n';

    for (int x : vis)
      cout << setw(3) << "-";
    cout << '\n';

    for (int i = 0; i < vis.size(); i++)
      cout << setw(3) << i + 1;
    cout << '\n';
  }

  void add(int x) {
    while (x--) {
      vis[producerPos] = 1;
      ++producerPos %= CAPACITY;
    }
  }

  void remove(int x) {
    while (x--) {
      vis[consumerPos] = 0;
      ++consumerPos %= CAPACITY;
    }
  }
};

enum { PRODUCER, CONSUMER };

int main() {

  Container container;
  Random random;
  int it = 40;

  while (it--) {
    int who = random.get<int>(1, 10000) % 2;
    if (who == PRODUCER && container.full()) {
      who ^= 1;
    } else if (who == CONSUMER && container.empty()) {
      who ^= 1;
    }

    if (who == PRODUCER) {
      int x = random.get<int>(1, container.maxToProduce());
      cout << "Productor coloca: " << x << '\n';
      container.add(x);
    } else {
      int x = random.get<int>(1, container.maxToConsume());
      cout << "Consumidor quita: " << x << '\n';
      container.remove(x);
    }

    container.print();

    // if (kbhit()) {
    //   int ch = getch();

    //   if (ch == 27)
    //     break;
    // }

    waitOnScreen(3s);
  }

  return 0;
}
