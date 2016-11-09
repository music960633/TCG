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

inline bool getBit(int& s, int idx) { return (s >> idx) & 1; }
inline void set1(int& s, int idx) { s |= (1 << idx); }
inline void set0(int& s, int idx) { s &= (~(1 << idx)); }

class Solver {
 public:
  explicit Solver(int n): n_(n) {
    map_.resize(n_);
    unk_.resize(n_);
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
    for (int i = 0; i < n_; ++i)
      unk_[i] = (1 << n_) - 1;
  }
  void solve() {
    init();
    preprocess();
    dfs(map_, unk_, 0, 0);
  }
  void preprocess() {
    bool flag = imply(map_, unk_, -1, -1);
    assert(flag);
  }
  bool imply(std::vector<int>& curMap, std::vector<int>& unknown, int x, int y) {
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
        if (!implyRow(curMap, unknown, idx, qCol))
          return false;
      }
      if (!qCol.empty()) {
        idx = qCol.front();
        qCol.pop();
        if (!implyCol(curMap, unknown, idx, qRow))
          return false;
      }
    }
    return true;
  }
  bool implyRow(std::vector<int>& curMap, std::vector<int>& unknown, int idx, std::queue<int>& qCol) {
    if (unknown[idx] == 0) return true;
    const std::vector<int>& v = mp[row_[idx]];
    int black = curMap[idx] & (~unknown[idx]);
    int white = (~curMap[idx]) & (~unknown[idx]);
    int imply_black = ~0, imply_white = ~0;
    for (int i = 0; i < v.size(); ++i) {
      if (((v[i] & black) == black) && (((~v[i]) & white) == white)) {
        imply_black &= v[i];
        imply_white &= (~v[i]);
      }
    }
    if (imply_black == ~0)
      return false;
    for (int i = 0; i < n_; ++i) {
      if (getBit(unknown[idx], i) == 1) {
        if (getBit(imply_black, i) == 1) {
          set0(unknown[idx], i);
          set1(curMap[idx], i);
          qCol.push(i);
        }
        else if (getBit(imply_white, i) == 1) {
          set0(unknown[idx], i);
          set0(curMap[idx], i);
          qCol.push(i);
        }
      }
    }
    return true;
  }
  bool implyCol(std::vector<int>& curMap, std::vector<int>& unknown, int idx, std::queue<int>& qRow) {
    bool done = true;
    for (int i = 0; i < n_; ++i)
      if (getBit(unknown[i], idx) == 1)
        done = false;
    if (done) return true;
    const std::vector<int>& v = mp[col_[idx]];
    int black = 0, white = 0;
    int imply_black = ~0, imply_white = ~0;
    for (int i = 0; i < n_; ++i) {
      if (getBit(unknown[i], idx) == 0 && getBit(curMap[i], idx) == 1)
        set1(black, i);
      else if (getBit(unknown[i], idx) == 0 && getBit(curMap[i], idx) == 0)
        set1(white, i);
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
      if (getBit(unknown[i], idx) == 1) {
        if (getBit(imply_black, i) == 1) {
          set0(unknown[i], idx);
          set1(curMap[i], idx);
          qRow.push(i);
        }
        else if (getBit(imply_white, i) == 1) {
          set0(unknown[i], idx);
          set0(curMap[i], idx);
          qRow.push(i);
        }
      }
    }
    return true;
  }
  bool dfs(std::vector<int>& prevMap, std::vector<int>& prevUnk, int x, int y) {
    if (x == n_){
      map_ = prevMap;
      unk_ = prevUnk;
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
    if (getBit(prevUnk[x], y) == 0) {
      return dfs(prevMap, prevUnk, nx, ny);
    }
    std::vector<int> curMap = prevMap;
    std::vector<int> curUnk = prevUnk;
    set1(curMap[x], y);
    set0(curUnk[x], y);
    if (imply(curMap, curUnk, x, y) && dfs(curMap, curUnk, nx, ny)) {
      return true;
    }
    curMap = prevMap;
    curUnk = prevUnk;
    set0(curMap[x], y);
    set0(curUnk[x], y);
    if (imply(curMap, curUnk, x, y) && dfs(curMap, curUnk, nx, ny)) {
      return true;
    }
    return false;
  }
  void display() {
    for (int i = 0; i < n_; ++i) {
      for (int j = 0; j < n_; ++j)
        std::cout << (char)(((unk_[i] >> j) & 1)? 'X' : ('0' + ((map_[i] >> j) & 1)));
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

 private:
  int                             n_;
  std::vector<int>                map_;
  std::vector<int>                unk_;
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
