#include "Data.h"
//
Data::Data(Globals *g, unordered_map<string, Data *> *map) {
  this->globals = g;
  this->symbol_table = map;
}

Data::Data(string var,
           string simulator,
           int biding,
           queue<string> *queue,
           unordered_map<string, Data *> *map,
           Globals *g) {
  this->sign = biding;
  this->varName = var;
  this->sim = simulator;
  this->update_simulator_q = queue;
  this->symbol_table = map;
  this->globals = g;
}
int Data::execute(vector<string> *string_vec, int j) {
  // extract the string of the expression, send it to interpret and calculate
  int i = getIndexBeforeOp((*string_vec)[j],0);
  string str = (*string_vec)[j].substr(i+1);
  // set the value of this data obj to it, and update the simulator if it  should
  setValue(fromStringToValue(str));
  return 2;
}
//return the index before the first operator
int Data::getIndexBeforeOp(string str, int i) {
  for (; i < (int) str.length(); i++) {
    if (str[i] == '=' || str[i] == '<' || str[i] == '>' || str[i] == '/' || str[i] == '*' || str[i] == '+'
        || str[i] == '-' || str[i] == '$' || str[i] == '%' || str[i] == '^' || str[i] == '&') {
      break;
    }
  }
  return i;
}
//return the index after the first operator
int Data::getIndexAfterOp(string str, int i) {
  for (; i < (int) str.length(); i++) {
    if (!(str[i] == '=' || str[i] == '<' || str[i] == '>' || str[i] == '/' || str[i] == '*' || str[i] == '+'
        || str[i] == '-' || str[i] == '$' || str[i] == '%' || str[i] == '^' || str[i] == '&')) {
      break;
    }
  }
  return i;
}

// set the value of this data obj to it, and update the simulator if it  should
void Data::setValue(double val) {
  this->value = val;
  if (sign == 1) {
    globals->locker.lock();
    this->update_simulator_q->push(this->varName);
    globals->locker.unlock();
  }
}
double Data::getValue() {
  return this->value;
}
string Data::getSim() {
  return this->sim;
}
string Data::getVarName() {
  return this->varName;
}
int Data::getSign() {
  return this->sign;
}

// get a string, convert it to an expression return the calculation
double Data::fromStringToValue(string str) {
  int i = 0, l = 0;
  // make all the two char operator represent as one
  string s1 = "<=";
  string s2 = "$";
  string s3 = ">=";
  string s4 = "%";
  string s5 = "==";
  string s6 = "^";
  string s7 = "!=";
  string s8 = "&";
  replace(str, s1, s2);
  replace(str, s3, s4);
  replace(str, s5, s6);
  replace(str, s7, s8);
  str.erase(remove(str.begin(), str.end(), '{'), str.end());
  Interpreter interpreter;
  // extract all the variable out of the string and insert it the the interpreter vars map
  while (i <= (int) str.length()) {
    i = getIndexAfterOp(str, l);
    l = getIndexBeforeOp(str, i);
    string variable_name = str.substr(i, l - i);
    variable_name.erase(remove(variable_name.begin(), variable_name.end(), ')'), variable_name.end());
    variable_name.erase(remove(variable_name.begin(), variable_name.end(), '('), variable_name.end());
    globals->locker.lock();
    if (symbol_table->find(variable_name) == symbol_table->end()) {
      globals->locker.unlock();
      i = l + 1;
      continue;
    }
    Data *data = (*symbol_table)[variable_name];
    globals->locker.unlock();
    string value_str = std::to_string(data->getValue());
    interpreter.setVariables(variable_name + "=" + value_str);
    i = l + 1;
  }
  // return the original operation representation
  replace(str, s2, s1);
  replace(str, s4, s3);
  replace(str, s6, s5);
  replace(str, s8, s7);
  try {
    // try to calculate the expression
    auto *exp = interpreter.interpret(str);
    double newValue = exp->calculate();
    delete (exp);
    return newValue;
  } catch (...) {
    cerr << "Something went wrong with the interpretation" << endl;
  }
  return 0;
}

//replace sub strings
bool Data::replace(std::string &str, const std::string &from, const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

