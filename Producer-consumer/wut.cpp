#include <bits/stdc++.h>
//#include "debug.h" //Elimina esta linea
#define fore(i, l, r) for (long long i = (l); i < (r); i++)
#define forex(i, l, r) for (long long i = (l); i >= (r); i--)
#define ll long long
#define ull unsigned long long
#define nl cout << "\n"
#define cnl "\n"
#define gfc "\033[32;1m"
#define rfc "\033[0m"
#define pb push_back
using namespace std;

const int N = 1001;
ll a[N][N];

int main() {

  ios_base::sync_with_stdio(0);
  cin.tie(0);
  // freopen("input.txt", "r", stdin);  //Elimina esta linea

  int n1, n2, n, r1, c1, r2, c2, aux, c = 0;
  cin >> n1 >> n2;

  fore (i, 0, n1 + 1) {
    c = 0;
    fore (j, 0, n2 + 1) {
      if (j == 0 || i == 0)
        a[i][j] = 0;
      else {
        cin >> aux;
        c += aux;
        a[i][j] = aux + a[i - 1][j] + a[i][j - 1] - a[i - 1][j - 1];
      }
    }
  }
  cin >> n;
  fore (i, 0, n) {
    cin >> r1 >> c1 >> r2 >> c2;

    assert(r1 <= r2);
    assert(c1 <= c2);

    ll r = a[r2][c2] - a[r1 - 1][c2] - a[r2][c1 - 1] + a[r1 - 1][c1 - 1];
    cout << r << cnl;
  }
}