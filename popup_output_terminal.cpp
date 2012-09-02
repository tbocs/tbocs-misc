/*============================================================================*\

  popup_output_terminal.cpp
    Open a new terminal for output using FIFO.

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

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define AUTO_OUTPUT_WINDOW

using namespace std; 

int main (int argc, char** argv) {
  string str_location("/tmp/testing");
  mkfifo(str_location.c_str(), S_IFIFO | 0666); 
  stringstream ss;

#ifdef AUTO_OUTPUT_WINDOW
  ss << "gnome-terminal -e 'bash -c \"while true; do \
           if read -t 0.5 line <> " << str_location << "; then \
             echo Received: $line; \
           else \
               kill -0 " << getpid() << "; \
               if [ $? -ne 0 ]; then \
                 rm " << str_location << "; exit; fi; \
           fi; done;\" ' &";
  system(ss.str().c_str());
  ss.str("");
#endif

  int pipefd = -1;
  while ((pipefd = open(str_location.c_str(), O_RDWR|O_NONBLOCK)) == -1);
  cout << " == Press CTRL + C to quit == " << endl;
  int counter = 0;

  while (true) {
    sleep(1);
    ss << "Output No. " << counter++ << "\n" ;
    write(pipefd, ss.str().c_str(), strlen(ss.str().c_str()));
    cout << "Sent: " << ss.str();
    ss.str("");
  }
  return 0;
}
