# SpaceX Challenge

This document describes the code package that will complete the SpaceX coding challenge for the software engineering internship.

## Setup and Installation

Download and unzip files from folder shared via email.

This package requires g++ to run.


### macOS
macOS has a built-in shell. Open the “Terminal” application.

Install a compiler.

```cmd
$ xcode-select --install
```
This compiler is really Apple LLVM pretending to be `g++`.
```cmd
$ g++ --version
Configured with: --prefix=/Library/Developer/CommandLineTools/usr 
--with-gxx-include-dir=/usr/include/c++/4.2.1
Apple LLVM version 9.0.0 (clang-900.0.38)
Target: x86_64-apple-darwin16.7.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
```

Install the [Homebrew package manager](https://brew.sh/).

Use Homebrew to install a few command line programs.
```
$ brew install wget git tree
```

### Windows
Use Windows 10 Subsystem for Linux (WSL). WSL runs native Linux command-line tools directly on Windows.

Start PowerShell and run it as administrator. Search for PowerShell in the start menu, then right-click and select “Run as administrator”.

Next, [follow the instructions from Microsoft](https://msdn.microsoft.com/en-us/commandline/wsl/install-win10). Be sure to select “Ubuntu Linux”.

Start an Ubuntu Bash shell (not a Windows PowerShell).

Run the following commands in your Bash shell:
```
$ sudo apt-get update
$ sudo apt-get install g++ make rsync wget git ssh gdb
```

### Check Tools
After you’ve installed a command line interface, you should have all these command line programs installed. Your versions might be different.
```
$ g++ --version
g++ (GCC) 6.4.0
$ make --version
GNU Make 4.2.1
$ gdb --version   # Windows and Linux only
GNU gdb (Ubuntu 8.1-0ubuntu3) 8.1.0.20180409-git
$ lldb --version  # macOS only
lldb-900.0.64
$ rsync --version
rsync  version 3.1.2  protocol version 31
$ wget --version
GNU Wget 1.19.4 built on linux-gnu.
$ git --version
git version 2.15.1
$ ssh -V
OpenSSH_7.6p1, OpenSSL 1.0.2m  2 Nov 2017
```

You should now be able to open and run the files for this challenge. Windows users should ensure they are using the Remote-WSL in their IDE.

## Usage

```python
import foobar

# returns 'words'
foobar.pluralize('word')

# returns 'geese'
foobar.pluralize('goose')

# returns 'phenomenon'
foobar.singularize('phenomena')
```


## Authors and acknowledgment
[Gurish Sharma](https://www.linkedin.com/in/gurish-sharma-/)