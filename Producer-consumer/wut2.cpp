//#include "debug.h"
#include <bits/stdc++.h>
using namespace std;
#define fore(i, l, r) for (int i = l; i <= r; i++)
#define ll long long

int n, m;
vector<vector<int>> a(1001, vector<int>(1001));
vector<vector<int>> Prefix(1001, vector<int>(1001));

int que(int r1, int c1, int r2, int c2) {

  return Prefix[r2][c2] - Prefix[r1 - 1][c2] - Prefix[r2][c1 - 1] + Prefix[r1 - 1][c1 - 1];
}

int main() {
  cin.tie(0)->sync_with_stdio(0), cout.tie(0);
  // ios_base::sync_with_stdio(false);
  // cin.tie(nullptr);

  cin >> n >> m;

  fore (i, 1, n) {
    fore (j, 1, m) {
      cin >> a[i][j];
      Prefix[i][j] = a[i][j] + Prefix[i - 1][j] + Prefix[i][j - 1] - Prefix[i - 1][j - 1];
    }
  }

  int q;
  cin >> q;
  while (q--) {
    int r1, c1, r2, c2;
    cin >> r1 >> c1 >> r2 >> c2;
    cout << que(r1, c1, r2, c2) << "\n";
  }
  return 0;
}