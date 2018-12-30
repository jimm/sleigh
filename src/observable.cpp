#include "observable.h"
#include "utils.h"

void Observable::add_observer(Observer *o) {
  observers.push_back(o);
}

void Observable::remove_observer(Observer *o) {
  std::remove(observers.begin(), observers.end(), o);
}

void Observable::changed() {
  for (vector<Observer *>::iterator i = observers.begin(); i != observers.end(); ++i) {
    (*i)->update(this);
  }
}
