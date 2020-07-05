# EventEmitter
Easy to use C ++ thread-based event system

# Depends on
[xxHash](https://github.com/Cyan4973/xxHash), but you can change the hash function for strings in `Event/common.cpp`, `Event::HashId::hashString`
I use `xxhash.hpp` from https://github.com/RedSpah/xxhash_cpp

# How to start
Add the `Event` folder to your includes.

OR use precompiled part from `includes`.

```c++
#include <iostream>

//include event management class with...
#include "Event/Emitter.h"
//or #include "include/EventEmitter.h"

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

# Data passing
if you want to pass some data in the function - just use `DataPack`/`DataPackCast`/`DataPackAutoClean`
#### Be careful!
- you don't need to delete the data packs manually
- you can pass **ONLY** dynamically created packs (by `new` etc.)
#### Examples:
```c++
//listener have to take only one argument with type of DataPack*:
void lis(DataPack*){
  //now you can cast the data pack
  DataPackCast<string> *pack = (DataPackCast<string>)DataPack;
  //and use the data
  cout << pack->data << endl;
}

//.....

//to pass the data:
myEvent.emit("someEvent", new DataPackCast<string>("my message"));
                                |
                                ^--- this pack will be deleted after 
                                     all listeners of this event are executed
                                     
//that's why if you want to share one resource, you have to use pointers in your packs
```

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


`Event/DataPack.h`
```
//// class DataPack
void* DataPack data //// data field

DataPack::DataPack(void* d) ////constructor

DataPack* copy() //// returns a copy of this pack with the same 'data' field

//// template<typename T = void*> class DataPackCast : public DataPack
////
//// it seems to DataPack but with template for data type:
T DataPackCast::data
DataPackCast::DataPackCast(T d)
DataPackCast* copy()

//// template<typename T> class DataPackAutoClean : public DataPack
////
//// Uses pointer to T (T*), delete it when all copies of this object.
//// Automatically executes 'delete' on its data ONLY in the case when all copies of this object have also been deleted
//// (all copies have a field that contains their total number)
T DataPackAutoClean::data
DataPackAutoClean::DataPackCast(T d)
DataPackAutoClean* copy()



```

