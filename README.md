# PyPLASMA
PyPLASMA is a python wrapper for some functions
of PLASMA. This is specially made for some linear
regression work.

### Requirements
* swig
* python 3.6.7
* Intel's math kernel library (MKL)
* plasma-17.1 (ICL, already pre-included)
* prodtools (custom library)
* CMAKE 3.14.x
* numpy

### Math Kernel Library

Please install intel's math kernel library (https://software.seek.intel.com/performance-libraries) 
run install.sh (if you run it with root permission, it installs onto /opt, else it will install on 
the user's home directory). The installation path is essential as we need to link the library later

### Compilation

Install the pre-requisites

	apt-get install swig
	apt-get install python3-dev
	apt-get install libopenblas-dev liblapacke-dev liblapack-dev
	apt-get install python3-pip
	pip3 install numpy

Compile the custom library prodtools 

	git clone https://github.com/ToraNova/library
	cd library/prodtools && ./cmake_build.sh

Clone this repository

	git clone https://github.com/ToraNova/pyplasma
	cd pyplasma

Edit the preset_env file to point to the roots for MKL (use your favourite editor), afterwhich, 
source the preset_env.sh file to your environment.

	. preset_env.sh

Compile

	make

#### Run a test
To ensure compilation and setup is successful, please run a test.

	make runtest

Expect the weight array to have almost 1 on the first element. The rest of the test should return OK or 0

### Troubleshooting
[see here](troubleshoot.md)

### Disclaimer !
I take no credit for ICL's work. This is just a wrapper for their awesome library

#### PLASMA's super fast linear algebra software library
[https://bitbucket.org/icl/plasma/src/default/](https://bitbucket.org/icl/plasma/src/default/)

Version 1.0 - Git initialized and minimal version
