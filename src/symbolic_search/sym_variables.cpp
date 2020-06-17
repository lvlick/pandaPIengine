#include "sym_variables.h"

#include "../Model.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

using namespace std;

namespace symbolic {

void exceptionError(string /*message*/) {
  // cout << message << endl;
  throw BDDError();
}

SymVariables::SymVariables(progression::Model *model)
    : model(model), cudd_init_nodes(16000000L), cudd_init_cache_size(16000000L),
      cudd_init_available_memory(0L), var_ordering(false) {}

void SymVariables::init() {
  vector<int> var_order;
  if (var_ordering) {
    // InfluenceGraph::compute_gamer_ordering(var_order);
  } else {
    for (int i = 0; i < model->numVars; ++i) {
      var_order.push_back(i);
    }
  }
  cout << "Sym variable order: ";
  for (int v : var_order)
    cout << v << " ";
  cout << endl;

  init(var_order);
}

// Constructor that makes use of global variables to initialize the
// symbolic_search structures

void SymVariables::init(const vector<int> &v_order) {
  cout << "Initializing Symbolic Variables" << endl;
  var_order = vector<int>(v_order);
  int num_fd_vars = var_order.size();

  // Initialize binary representation of variables.
  numBDDVars = 0;
  bdd_index_pre = vector<vector<int>>(v_order.size());
  bdd_index_eff = vector<vector<int>>(v_order.size());
  int _numBDDVars = 0; // numBDDVars;
  for (int var : var_order) {
    int var_len = ceil(log2(getDomainSize(var)));
    numBDDVars += var_len;
    for (int j = 0; j < var_len; j++) {
      bdd_index_pre[var].push_back(_numBDDVars);
      bdd_index_eff[var].push_back(_numBDDVars + 1);
      _numBDDVars += 2;
    }
  }
  cout << "Num variables: " << var_order.size() << " => " << numBDDVars << endl;

  // Initialize manager
  cout << "Initialize Symbolic Manager(" << _numBDDVars << ", "
       << cudd_init_nodes / _numBDDVars << ", " << cudd_init_cache_size << ", "
       << cudd_init_available_memory << ")" << endl;
  manager = unique_ptr<Cudd>(
      new Cudd(_numBDDVars, 0, cudd_init_nodes / _numBDDVars,
               cudd_init_cache_size, cudd_init_available_memory));

  manager->setHandler(exceptionError);
  manager->setTimeoutHandler(exceptionError);
  manager->setNodesExceededHandler(exceptionError);

  cout << "Generating binary variables" << endl;
  // Generate binary_variables
  for (int i = 0; i < _numBDDVars; i++) {
    variables.push_back(manager->bddVar(i));
  }

  preconditionBDDs.resize(num_fd_vars);
  effectBDDs.resize(num_fd_vars);
  biimpBDDs.resize(num_fd_vars);
  validValues.resize(num_fd_vars);
  validBDD = oneBDD();
  // Generate predicate (precondition (s) and effect (s')) BDDs
  for (int var : var_order) {
    for (int j = 0; j < getDomainSize(var); j++) {
      preconditionBDDs[var].push_back(createPreconditionBDD(var, j));
      effectBDDs[var].push_back(createEffectBDD(var, j));
    }
    validValues[var] = zeroBDD();
    for (int j = 0; j < getDomainSize(var); j++) {
      validValues[var] += preconditionBDDs[var][j];
    }
    validBDD *= validValues[var];
    biimpBDDs[var] =
        createBiimplicationBDD(bdd_index_pre[var], bdd_index_eff[var]);
  }

  cout << "Symbolic Variables... Done." << endl;
}

BDD SymVariables::getStateBDD(const std::vector<int> &state) const {
  BDD res = oneBDD();
  for (int i = var_order.size() - 1; i >= 0; i--) {
    res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
  }
  return res;
}

BDD SymVariables::getStateBDD(const int *state_bits,
                              int state_bits_size) const {
  BDD res = oneBDD();
  return res;
}

/*BDD SymVariables::getStateBDD(const GlobalState &state) const {
  BDD res = oneBDD();
  for (int i = var_order.size() - 1; i >= 0; i--) {
    res = res * preconditionBDDs[var_order[i]][state[var_order[i]]];
  }
  return res;
}*/

BDD SymVariables::getPartialStateBDD(
    const vector<pair<int, int>> &state) const {
  BDD res = validBDD;
  for (int i = state.size() - 1; i >= 0; i--) {
    // if(find(var_order.begin(), var_order.end(),
    //               state[i].first) != var_order.end()) {
    res = res * preconditionBDDs[state[i].first][state[i].second];
    //}
  }
  return res;
}

BDD SymVariables::generateBDDVar(const std::vector<int> &_bddVars,
                                 int value) const {
  BDD res = oneBDD();
  for (int v : _bddVars) {
    if (value % 2) { // Check if the binary variable is asserted or negated
      res = res * variables[v];
    } else {
      res = res * (!variables[v]);
    }
    value /= 2;
  }
  return res;
}

BDD SymVariables::createBiimplicationBDD(const std::vector<int> &vars,
                                         const std::vector<int> &vars2) const {
  BDD res = oneBDD();
  for (size_t i = 0; i < vars.size(); i++) {
    res *= variables[vars[i]].Xnor(variables[vars2[i]]);
  }
  return res;
}

vector<BDD> SymVariables::getBDDVars(const vector<int> &vars,
                                     const vector<vector<int>> &v_index) const {
  vector<BDD> res;
  for (int v : vars) {
    for (int bddv : v_index[v]) {
      res.push_back(variables[bddv]);
    }
  }
  return res;
}

int SymVariables::getDomainSize(int var) const {
  int var_domain_size = 0;

  // Check for domain size (boolean have same index)
  if (model->firstIndex[var] == model->lastIndex[var]) {
    var_domain_size = 2;
  } else {
    var_domain_size = model->firstIndex[var] - model->lastIndex[var] + 1;
  }
  return var_domain_size;
}

BDD SymVariables::getCube(int var, const vector<vector<int>> &v_index) const {
  BDD res = oneBDD();
  for (int bddv : v_index[var]) {
    res *= variables[bddv];
  }
  return res;
}

BDD SymVariables::getCube(const set<int> &vars,
                          const vector<vector<int>> &v_index) const {
  BDD res = oneBDD();
  for (int v : vars) {
    for (int bddv : v_index[v]) {
      res *= variables[bddv];
    }
  }
  return res;
}

std::vector<std::string> SymVariables::get_fd_variable_names() const {
  std::vector<string> var_names(numBDDVars * 2);
  for (int v : var_order) {
    int exp = 0;
    for (int j : bdd_index_pre[v]) {
      var_names[j] = "var" + to_string(v) + "_2^" + std::to_string(exp);
      var_names[j + 1] =
          "var" + to_string(v) + "_2^" + std::to_string(exp++) + "_primed";
    }
  }

  return var_names;
}

void SymVariables::print_options() const {
  cout << "CUDD Init: nodes=" << cudd_init_nodes
       << " cache=" << cudd_init_cache_size
       << " max_memory=" << cudd_init_available_memory
       << " ordering: " << (var_ordering ? "special" : "standard") << endl;
}

} // namespace symbolic