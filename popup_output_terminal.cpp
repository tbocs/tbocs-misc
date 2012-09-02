/*============================================================================*\

  popup_output_terminal.cpp
    Open a new terminal for output using FIFO. Linux & Gnome required.

  Copyright (C) 2012 Sam Zhai - The Beauty of Computer Science (sam@tbocs.org)

  This piece of software is under MIT lisense.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
  DEALINGS IN THE SOFTWARE.
\*============================================================================*/

// Comment this line to use C libraries instead.
#define USE_CXX

#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef USE_CXX
  #include <fstream>
#else
  #include <unistd.h>
  #include <fcntl.h>
#endif

using namespace std; 

class Main {
 public:
  string str_location;
  stringstream ss;
  int counter;

#ifdef USE_CXX
  fstream os_fifo;
#else
  int fd_fifo;
#endif

  // Create FIFO and initialize file handle.
  Main () : str_location("/tmp/testing"), counter(0) {
    mkfifo(str_location.c_str(), S_IFIFO | 0666);
    open_output_terminal();

#ifdef USE_CXX
    os_fifo.open(str_location.c_str(), ios_base::in | ios_base::out);
#else
    while ((fd_fifo = open(str_location.c_str(), O_RDWR|O_NONBLOCK)) == -1);
#endif
    cout << " == Press CTRL+C to quit == " << endl;
  }

  // Feed FIFO input periodically.
  void print_loop () {
    while (true) {
      sleep(1);
      ss << "Output No. " << counter++ << "\n" ;

#ifdef USE_CXX
      os_fifo << ss.str();
      os_fifo.flush();
#else
      write(fd_fifo, ss.str().c_str(), strlen(ss.str().c_str()));
#endif
      cout << "Sent: " << ss.str();
      ss.str("");
    }
  }

  // Clean up before quitting.
  ~Main () {
#ifdef USE_CXX
    os_fifo.close();
#else
    close(fd_fifo);
#endif
    system(str_location.insert(0, "rm -f ").c_str());
  }

  // Open new terminal for displaying FIFO output.
  void open_output_terminal() {
    ss << "gnome-terminal -e 'bash -c \"while true; do \
             if read -t 0.5 line <> " << str_location << "; then \
               echo Received: $line; \
             else \
                 kill -0 " << getpid() << "; \
                 if [ $? -ne 0 ]; then exit; fi; \
             fi; done;\" ' &";
    system(ss.str().c_str());
    ss.str("");
  }
} *m = NULL;

// Clean-up enforced.
void signal_callback (int sig) {
  if (m != NULL) {
    delete m;
  }
  exit(0);
}

int main (int argc, char** argv) {
  signal(SIGTERM, signal_callback);
  signal(SIGINT, &signal_callback);
  m = new Main();
  m->print_loop();
  delete m;
  return 0;
}
