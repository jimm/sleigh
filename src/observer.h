#ifndef OBSERVER_H
#define OBSERVER_H

class Observable;

class Observer {
public:
  virtual void update(Observable *o) {}
};

#endif /* OBSERVER_H */
