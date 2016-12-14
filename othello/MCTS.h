#ifndef MCTS_H_
#define MCTS_H_

#include "board.h"
#include <stdio.h>
#include <math.h>
#include <cassert>
#include <random>
#ifdef _WIN32
#include <chrono>
#endif

#define NUM_SIM 300

template<class RIT>
RIT random_choice(RIT st, RIT ed) {
#ifdef _WIN32
  //std::random_device is deterministic with MinGW gcc 4.9.2 on Windows
  static std::mt19937 local_rand(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
#else
  static std::mt19937 local_rand(std::random_device{}());
#endif
  return st + std::uniform_int_distribution<int>(0, ed - st - 1)(local_rand);
}

class Node {
 public:
  Node(bool type, int move, int tile, Node* par): type_(type), move_(move), tile_(tile), par_(par), nSim_(0), nWin_(0) {
    child_.clear();
  }
  ~Node() {
    for (int i = 0, n = child_.size(); i < n; ++i)
      delete child_[i];
  }

  // functions for getting information
  int get_move() const { return move_; }
  Node* get_parent() const { return par_; }
  float get_rate() const { return (float)nWin_ / nSim_; }
  float get_ucb(int N) const {
    static const float c = 0.2;
    return (float)nWin_ / nSim_ + c * sqrt(log(N) / nSim_);
  }
  bool is_leaf() const { return child_.size() == 0; }

  // functions for modifying private variables
  void inc_nSim(int nSim) { nSim_ += nSim; }
  void inc_nWin(int nWin) { nWin_ += nWin; }

  // functions for modifying other objects
  void update_board(board& b) {
    assert(move_ != -1);
    static int dummy[64];
    b.update(move_>>3, move_&7, dummy);
  }
  void update_parent(int nSim, int nWin) {
    Node* now = this;
    while (now != NULL) {
      now->inc_nSim(nSim);
      now->inc_nWin(nWin);
      now = now->get_parent();
    }
  }

  // functions for MCTS
  Node* get_best_child(int totSim) const {
    assert(!is_leaf());
    if (child_.size() == 1)
      return child_[0];
    Node* best_node = child_[0];
    float best_ucb = child_[0]->get_ucb(totSim);
    for (int i = 1, n = child_.size(); i < n; ++i) {
      float tmp_ucb = child_[i]->get_ucb(totSim);
      if (type_ ^ (tmp_ucb > best_ucb)) {
        best_node = child_[i];
        best_ucb = tmp_ucb;
      }
    }
    return best_node;
  }
  int get_best_move() const {
    assert(!is_leaf());
    assert(!type_);
    if (child_.size() == 1)
      return child_[0]->get_move();
    int best_move = child_[0]->get_move();
    float best_rate = child_[0]->get_rate();
    for (int i = 1, n = child_.size(); i < n; ++i) {
      float tmp_rate = child_[i]->get_rate();
      if (tmp_rate > best_rate) {
        best_move = child_[i]->get_move();
        best_rate = tmp_rate;
      }
    }
    printf("win rate: %f\n", best_rate);
    return best_move;
  }
  // B is board after updating move_
  void random_simulate(const board& B, int nSim) {
    static int dummy[64], ml[64], *mled;
    int choice;
    int nWin = 0;
    for (int i = 0; i < nSim; ++i) {
      board b(B);
      while (!b.is_game_over()) {
        mled = b.get_valid_move(ml);
        choice = (mled == ml ? 64 : *random_choice(ml, mled));
        b.update(choice>>3, choice&7, dummy);
      }
      if ((tile_ == 1 && b.get_score() > 0) || (tile_ == 2 && b.get_score() < 0))
        nWin += 1;
    }
    update_parent(nSim, nWin);
  }
  // B is board after updating move_
  void gen_child(const board& B) {
    assert(child_.size() == 0);
    assert(!B.is_game_over());
    static int ml[64], *mled;
    mled = B.get_valid_move(ml);
    if (mled == ml) 
      child_.push_back(new Node(!type_, 64, tile_, this));
    else
      for (int* p = ml; p != mled; ++p)
        child_.push_back(new Node(!type_, *p, tile_, this));
  }
  void sim_child(const board& B, int nSim) {
    assert(child_.size() > 0);
    for (int i = 0, n = child_.size(); i < n; ++i) {
      Node* ch = child_[i];
      board b(B);
      ch->update_board(b);
      ch->random_simulate(b, nSim / n);
    }
  }

  // helper functions
  void print(int tab, int totSim) const {
    for (int i = 0; i < tab; ++i) printf("  ");
    printf("(%s) ", type_? "MIN" : "MAX");
    if (move_ != -1)
      printf("move: (%d %d), ", move_>>3, move_&7);
    else
      printf("Root, ");
    printf("nSim: %d, mWin: %d, ", nSim_, nWin_);
    printf("UCB: %.3f\n", get_ucb(totSim));
    for (int i = 0, n = child_.size(); i < n; ++i)
      child_[i]->print(tab + 1, totSim);
  }

 private:
  bool type_;   // 0: max, 1: min
  int move_;
  int tile_;
  Node* par_;
  int nSim_, nWin_;
  std::vector<Node*> child_;
};

class MCTS {
 public:
  MCTS(const board& b): b_(b), totSim_(0) {
    root_ = new Node(false, -1, b.get_my_tile(), NULL);
  }
  ~MCTS() {
    delete root_;
  }

  void run() {
    board b(b_);
    // 1. Selection
    Node* target_node = get_best_node(b);
    if (!b.is_game_over()) {
      // 2. Expansion
      target_node->gen_child(b);
      // 3. Simulation + 4. Back propagation
      target_node->sim_child(b, NUM_SIM);
    }
    else {
      target_node->random_simulate(b, NUM_SIM);
    }
    totSim_ += NUM_SIM;
  }
  void print() const {
    root_->print(0, totSim_);
  }
  int get_best_move() {
    assert(root_ != NULL);
    assert(!root_->is_leaf());
    int ret = root_->get_best_move();
    printf("decision = (%d %d)\n", ret>>3, ret&7);
    return ret;
  }

 private:
  Node* get_best_node(board& b) {
    Node* now = root_;
    while (!now->is_leaf()) {
      now = now->get_best_child(totSim_);
      now->update_board(b);
    }
    return now;
  }

  board b_;
  Node* root_;
  int totSim_;
};

#endif  // MCTS_H__
