/*
 *  Init.cpp
 *  
 *
 *
 */

extern "C" void _init();
extern "C" void _fini();

/** A global object of this class is used to perform initialisation
    and shutdown services for the entire library. The constructor is
    run when the library is loaded and the destructor when it is
    unloaded. */
class StartupShutdownHandler {
public:

  StartupShutdownHandler() {
   _init();
  }

  ~StartupShutdownHandler() {
   _fini();
  }

} g_oStartupShutdownHandler;
