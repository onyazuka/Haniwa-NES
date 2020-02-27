#pragma once
#include <vector>
#include <algorithm>

/*
    I can pass type and get pointer to it on updating - it's very useful.
    I don't intend to use this observer with multiple types of observables,
        but if I would like to do so, I can use multiple inheritance(but should I?)
*/
template<typename T>
class Observer {
public:
    virtual ~Observer() {}
    virtual void update(T* observable, int eventType) = 0;
};

template<typename T>
class Observable {
public:
    virtual ~Observable() {}
    void attach(Observer<T>* obs) {
        observers.push_back(obs);
    }
    void detach(Observer<T>* obs) {
        auto foundObs = std::find(observers.begin(), observers.end(), obs);
        if(foundObs == observers.end()) return;
        observers.erase(foundObs);
    }
    void notify(int eventType) {
        for(auto observer : observers) {
            observer->update(static_cast<T*>(this), eventType);
        }
    }
private:
    std::vector<Observer<T>*> observers;
};
