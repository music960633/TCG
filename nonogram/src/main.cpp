#include <stdio.h>
#include <string.h>
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <cassert>

std::map<std::vector<int>, std::vector<int> > mp;

void buildMap(int n) {
  std::vector<int> key;
  int cnt;
  for (int s = 0; s < (1 << n); ++s) {
    key.clear();
    cnt = 0;
    for (int i = 0; i < n; ++i) {
      if ((s >> i) & 1)
        cnt += 1;
      else if (cnt != 0) {
        key.push_back(cnt);
        cnt = 0;
      }
    }
    if (cnt != 0) key.push_back(cnt);
    if (mp.find(key) == mp.end())
      mp[key] = std::vector<int>();
    mp[key].push_back(s);
  }
}

int str2int(const std::string& s) {
  int n = s.length();
  int ret = 0;
  for (int i = 0; i < n; ++i) {
    if(s[i] < '0' || s[i] > '9') return 0;
    ret = 10 * ret + (s[i] - '0');
  }
  return ret;
}

void splitStr(std::string s, std::vector<std::string>& tokens) {
  tokens.clear();
  s += "a";
  int n = s.length();
  std::string token = "";
  for (int i = 0; i < n; ++i) {
    if (s[i] >= '0' && s[i] <= '9') {
      token.append(1, s[i]);
    }
    else if (token.size() > 0) {
      tokens.push_back(token);
      token = "";
    }
  }
}

class Solver {
 public:
  explicit Solver(int n): n_(n) {
    map_.resize(n_ + 1);
    for (int i = 0; i <= n_; ++i)
      map_[i].resize(n_ + 1);
    row_.resize(n_);
    for (int i = 0; i < n_; ++i)
      row_[i].clear();
    col_.resize(n_);
    for (int i = 0; i < n_; ++i)
      col_[i].clear();
  }
  ~Solver() {}

  bool read() {
    int m;
    std::string str;
    std::vector<std::string> tokens;
    std::getline(std::cin, str);
    if (std::cin.eof()) return false;
    // read col
    for (int i = 0; i < n_; ++i) {
      std::getline(std::cin, str);
      splitStr(str, tokens);
      m = tokens.size();
      col_[i].resize(m);
      for (int j = 0; j < m; ++j) {
        col_[i][j] = str2int(tokens[j]);
      }
    }
    // read row
    for (int i = 0; i < n_; ++i) {
      std::getline(std::cin, str);
      splitStr(str, tokens);
      m = tokens.size();
      row_[i].resize(m);
      for (int j = 0; j < m; ++j) {
        row_[i][j] = str2int(tokens[j]);
      }
    }
    return true;
  }
  void init() {
    for (int i = 0; i <= n_; ++i)
      for (int j = 0; j <= n_; ++j)
        map_[i][j] = ((i == n_ || j == n_) ? 0 : -1);
  }
  void solve() {
    init();
    preprocess();
    dfs(map_, 0, 0);
  }
  void preprocess() {
    bool flag = imply(map_, -1, -1);
    assert(flag);
  }
  bool imply(std::vector<std::vector<int> >& curMap, int x, int y) {
    std::queue<int> qRow, qCol;
    int idx;
    if (x == -1 && y == -1) {
      for (int i = 0; i < n_; ++i) {
        qRow.push(i);
        qCol.push(i);
      }
    }
    else {
      qRow.push(x);
      qCol.push(y);
    }
    while (!qRow.empty() || !qCol.empty()) {
      if (!qRow.empty()) {
        idx = qRow.front();
        qRow.pop();
        if (!implyRow(curMap, idx, qCol))
          return false;
      }
      if (!qCol.empty()) {
        idx = qCol.front();
        qCol.pop();
        if (!implyCol(curMap, idx, qRow))
          return false;
      }
    }
    return true;
  }
  bool implyRow(std::vector<std::vector<int> >& curMap, int idx, std::queue<int>& qCol) {
    bool done = true;
    for (int i = 0; i < n_; ++i)
      if (curMap[idx][i] == -1)
        done = false;
    if (done) return true;
    const std::vector<int>& v = mp[row_[idx]];
    int black = 0, white = 0;
    int imply_black = ~0, imply_white = ~0;
    for (int i = 0; i < n_; ++i) {
      if (curMap[idx][i] == 1)
        black |= (1 << i);
      else if (curMap[idx][i] == 0)
        white |= (1 << i);
    }
    for (int i = 0; i < v.size(); ++i) {
      if (((v[i] & black) == black) && (((~v[i]) & white) == white)) {
        imply_black &= v[i];
        imply_white &= (~v[i]);
      }
    }
    if (imply_black == ~0)
      return false;
    for (int i = 0; i < n_; ++i) {
      if (curMap[idx][i] == -1) {
        if ((imply_black >> i) & 1) {
          curMap[idx][i] = 1;
          qCol.push(i);
        }
        else if ((imply_white >> i) & 1) {
          curMap[idx][i] = 0;
          qCol.push(i);
        }
      }
    }
    return true;
  }
  bool implyCol(std::vector<std::vector<int> >& curMap, int idx, std::queue<int>& qRow) {
    bool done = true;
    for (int i = 0; i < n_; ++i)
      if (curMap[i][idx] == -1)
        done = false;
    if (done) return true;
    const std::vector<int>& v = mp[col_[idx]];
    int black = 0, white = 0;
    int imply_black = ~0, imply_white = ~0;
    for (int i = 0; i < n_; ++i) {
      if (curMap[i][idx] == 1)
        black |= (1 << i);
      else if (curMap[i][idx] == 0)
        white |= (1 << i);
    }
    for (int i = 0; i < v.size(); ++i) {
      if (((v[i] & black) == black) && (((~v[i]) & white) == white)) {
        imply_black &= v[i];
        imply_white &= (~v[i]);
      }
    }
    if (imply_black == ~0)
      return false;
    for (int i = 0; i < n_; ++i) {
      if (curMap[i][idx] == -1) {
        if ((imply_black >> i) & 1) {
          curMap[i][idx] = 1;
          qRow.push(i);
        }
        else if ((imply_white >> i) & 1) {
          curMap[i][idx] = 0;
          qRow.push(i);
        }
      }
    }
    return true;
  }
  bool dfs(std::vector<std::vector<int> >& prevMap, int x, int y) {
    if (x == n_){
      map_ = prevMap;
      return true;
    }
    int nx, ny;
    if (y == n_ - 1) {
      nx = x + 1;
      ny = 0;
    }
    else {
      nx = x;
      ny = y + 1;
    }
    if (prevMap[x][y] != -1) {
      return dfs(prevMap, nx, ny);
    }
    std::vector<std::vector<int> > curMap = prevMap;
    curMap[x][y] = 1;
    if (imply(curMap, x, y) && dfs(curMap, nx, ny)) {
      return true;
    }
    curMap = prevMap;
    curMap[x][y] = 0;
    if (imply(curMap, x, y) && dfs(curMap, nx, ny)) {
      return true;
    }
    return false;
  }
  void display() {
    for (int i = 0; i < n_; ++i) {
      for (int j = 0; j < n_; ++j)
        std::cout << (char)(map_[i][j] == -1? 'X' : ('0' + map_[i][j]));
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

 private:
  int                             n_;
  std::vector<std::vector<int> >  map_;
  std::vector<std::vector<int> >  row_, col_;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <n>\n", argv[0]);
    return 0;
  }
  int n = str2int(argv[1]);
  if (n == 0) {
    fprintf(stderr, "n should be a positive integer\n");
    return 0;
  }

  Solver *solver = new Solver(n);
  buildMap(n);
  while (solver->read()) {
    solver->solve();
    solver->display();
  }
  delete solver;

  return 0;
}
