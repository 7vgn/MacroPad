The slider on MacroPad acts as a standard USB HID-class slider device and thus needs no driver. However, most operating systems don't know what to do with slider input and therefore just ignore it. For the slider to have any effect, you will need some application running on the host. 

This directory contains two small libraries - one for C++ and one for Python - that can be used to write such applications. Both libraries need the [https://github.com/libusb/hidapi](HIDAPI library) to be installed. 
- The C++ library consists of the MacroPad.h and MacroPad.cpp files. An example for how to use it is in example.cpp. 
- The Python library consists of the MacroPad.py file. An example for how to use it is in example.py. The [cffi](https://cffi.readthedocs.io/) module must be installed. 

In the subdirectories you can find more practical applications for these libraries. 
