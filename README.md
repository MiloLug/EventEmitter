# EventEmitter
Easy to use C ++ thread-based event system

# Depends on
[xxHash](https://github.com/Cyan4973/xxHash) 
(but you can change hash function for strings in `Event/common.cpp`, `Event::HashId::hashString`)

# How to start

```c++
#include <iostream>

//include event management class
#include "Event/Emitter.h"
//since the events are multithreaded, I highly recommend using mutexes
#include <mutex>

std::mutex m;
void listener(Event::DataPack* data){
  m.lock();
  std::cout << "i'm alive!" << std::endl;
  m.unlock();
  
  //stop event processing
  Event::terminate();
}

int main(){
  //do something...

  //initialize event loops
  Event::init();
  
  //An Event namespace has a predefined Object of the class Event::Emitter, it called Event::root. 
  //You can call methods directly by pointing this object,
  //also you can call Event::[method name] to refer to it.
  //Ok, create the listener
  Event::on("someEvent", listener);
  
  //now we can emit that event
  Event::emit("someEvent");
  
  //wait for the termination function (Event::terminate) to be called
  Event::wait();
  
  //do something...
  
  
  return 0;
}
```
Also you can check `main.cpp` for example.

# Details
(only public methods, fields etc.)
## Typedefs:
```
Event/ThreadLoops.h:
    Event::defaultListenerFn_t = std::function<void(DataPack*)>
    
Event/Emitter.h:
    Event::mainListenersMap_t = std::map<elemIdNum_t, std::vector<defaultListenerFn_t*>>
    
Event/common.h
    Event::elemIdNum_t = unsigned long long
```

## Fields and methods:

`Event/ThreadLoops.h`
```
static bool ThreadLoops::stop //// becomes true after executing ThreadLoops::terminate();

static void ThreadLoops::addExecutable(DataPack* data, anyof fn) //// add new functions into execution stack
  data - some wrapped data
  fn - one of the following:
      const defaultListenerFn_t
      std::stack<defaultListenerFn_t>
      std::vector<defaultListenerFn_t>
      std::vector<defaultListenerFn_t*>
      
static void ThreadLoops::init([size_t threadsNumber]) //// threads initialization
  threadsNumber - number of threads to be created
      if the parameter is not passed, it will use the number of cores
      
static void ThreadLoops::wait() //// wait for the termination function (ThreadLoops::terminate) to be called

static void ThreadLoops::terminate(bool dontWaitFunctions = false) //// stop all threads.
  dontWaitFunctions - if false => wait until the last functions complete
      if true => functions in threads are still executing, but you can exit the program
```


`Event/Emitter.h`
```
Emitter* Emitter::on(HashId id, const defaultListenerFn_t& fn [, defaultListenerFn_t*& listenerPtr]) //// bind new listener function
  id - HashId/string/char[]/elemIdNum_t - name of event
  fn - function
  listenerPtr - if passed, takes a pointer to a new function in the storage. 
      It can be useful if a lambda was passed as an argument to "fn", 
      then you can remove such a listener with Emitter::off only by this pointer
      
Emitter* off(HashId id, const defaultListenerFn_t& fn) //// remove the listener
  id - HashId/string/char[]/elemIdNum_t - name of event
  fn - function
  
Emitter* emit(HashId id, DataPack* data = nullptr) //// emit some event
  id - HashId/string/char[]/elemIdNum_t - name of event
  data - DataPack wrapper with some data
  
Emitter Event::root //// 'extern' defined instance of the Emitter
  
void Event::init(...) //// ref to ThreadLoops::init

void Event::wait(...) //// ref to ThreadLoops::wait

void Event::terminate(...) //// ref to ThreadLoops::terminate

void Event::on(...) //// ref to Event::root.on

void Event::off(...) //// ref to Event::root.off

void Event::emit(...) //// ref to Event::root.emit
```


`Event/common.h`
```
static elemIdNum_t HashId::hashString(anyof data) //// get hash
  data - one of the following:
      std::string
      const char*
  
elemIdNum_t HashId::data = 0 //// the hash
string HashId::src = "" //// source string of the hash

HashId::HashId(anyof data) //// constructors
  data - one of the following:
      elemIdNum_t
      std::string
      const char*
      HashId
      const HashId

HashId operators: =, !=, ==
    types for all:
        elemIdNum_t
        std::string
        const char*
        HashId
        const HashId

template<typename T, typename... U> bool Event::fnCanBeCompared(const std::function<T(U...)>& f)
     ////check if a function can be compared by Event::comparableCompare

template<typename T, typename... U> bool Event::comparableCompare(const std::function<T(U...)>& f1, const std::function<T(U...)>& f2)
    ////only for comparable functions. if f1 == f2 => true
    
template<typename T, typename... U> bool compareFn(const std::function<T(U...)>& fnA, const std::function<T(U...)>& fnB)
    ////truth map:
        if fnA and fnB are comparable: return fnA == fnB
        if only one is comparable: return false
        if both are uncomparable: return &fnA == &fnB
```
