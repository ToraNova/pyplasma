PyPLASMA is a python wrapper for some functions
of PLASMA. This is specially made for some linear
regression work.

### Requirements

	swig
	python 3.6.7
	intel's math kernel library (MKL)
	plasma-17.1 (ICL)
	prodtools (custom library)

### Math Kernel Library

Please install intel's math kernel library (https://software.seek.intel.com/performance-libraries) 
run install.sh (if you run it with root permission, it installs onto /opt, else it will install on 
the user's home directory). The installation path is essential as we need to link the library later

### Installation

Install the pre-requisites

	apt-get install swig

Compile the custom library prodtools 

	git clone https://github.com/ToraNova/library
	cd library/prodtools && ./cmake_build.sh

Clone the repo (in a separate directory)

	git clone https://github.com/ToraNova/pydistlearn
	cd pydistlearn && git submodule init
	cd pyplasma/plasma-17.1

	# Edit the preset_env file to point to the root directory of MKL
	vim preset_env.sh



Version 1.0 - Git initialized and minimal version

### Disclaimer !
I take no credit for ICL's work. This is just a wrapper for their awesome library

#### PLASMA's super fast linear algebra software library
[https://bitbucket.org/icl/plasma/src/default/](https://bitbucket.org/icl/plasma/src/default/)
