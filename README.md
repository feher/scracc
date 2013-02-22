Scracc
========

Scracc is a C++ Prototyping Tool.

Here is the idea:

1. Create your scracc file. For example, here is a hello.scc:

        #!/usr/bin/scracc
        int main()
        {
            cout << "Hello World" << endl;
            return 0;
        }

2. Make it executable

        chmod +x hello.scc

3. Run it

        ./hello.scc

How does it work?
-------------------

When you say ```./hello.scc```, Scracc automatically compiles the executable and runs it.

Compilation happens only for the first time. Subsequent executions run a cached version of the executable.

Recompilation happens only if you change the scc file, otherwise a cached version of the executable is run.

The scracc file is compiled with gcc using C++11 features and it is linked against the libscracc library. The code in your scracc file is prepended by the following header for your convenience:

    // Generated by scracc
    #include <scracc/libscracc.h>
    #include <iostream>
    using namespace Scracc;
    using namespace std;


Libscracc
-----------
Your scracc executable is automatically linked with the libscracc library. It provides some commonly used functions with very simple and intuitive API.
Here is a list of the currently provided funcions in the ```Scracc``` namespace:

    void SetThrowExceptions(bool throwExceptions);

    void SetEnv(const string & name, const string & value, bool * ok = nullptr);
    string GetEnv(const string & name, bool * ok = nullptr);
    string Md5Sum(const string & message, bool * ok = nullptr);
    string ReadFile(const string & filePath, bool * ok = nullptr);
    void WriteFile(const string & filePath, const string & contents, bool * ok = nullptr);
    int Execute(const string & command, bool * ok = nullptr);
    string GetCwd(bool * ok = nullptr);
    void ChDir(const string & dirPath, bool * ok = nullptr);
    void MkDir(const string & dirPath, bool * ok = nullptr);
    void MkDirPath(const string & dirPath, bool * ok = nullptr);
    void Remove(const string & filePath, bool * ok = nullptr);
    void RemoveAll(const string & filePath, bool * ok = nullptr);
    string BaseName(const string & filePath, bool * ok = nullptr);
    string DirName(const string & filePath, bool * ok = nullptr);
    string AbsolutePath(const string & relPath, bool * ok = nullptr);
    string BuildPath(initializer_list<string> parts);
    bool Exists(const string & filePath, bool * ok = nullptr);
    bool IsDir(const string & dirPath, bool * ok = nullptr);
    bool IsRegularFile(const string & filePath, bool * ok = nullptr);
    bool IsFile(const string & filePath, bool * ok = nullptr);
    bool IsSymlink(const string & filePath, bool * ok = nullptr);
    void Symlink(const string & to, const string & from, bool * ok = nullptr);
    string ReadLink(const string & filePath, bool * ok = nullptr);
    uintmax_t FileSize(const string & filePath, bool * ok = nullptr);
    string Owner(const string & path, bool * ok = nullptr);
    string Group(const string & path, bool * ok = nullptr);
    string Permission(const string & path, bool * ok = nullptr);
    void ChMod(const string & path, const string & mode, bool recursive = false, bool * ok = nullptr);
    void ChOwn(const string & path, const string & ownership, bool recursive = false, bool * ok = nullptr);
    void ChGrp(const string & path, const string & group, bool recursive = false, bool * ok = nullptr);
    vector<string> Find(const string & startPath,
                        function<bool(const string & path)> predicate,
                        size_t depth = 0,
                        bool followSymlink = false);
    void FindAndDo(const string & startPath,
                   function<bool(const string & path)> predicate,
                   function<bool(const string & path)> action,
                   int depth = 0,
                   bool followSymlink = false);


Building Scracc
-----------------

Requirements

* CMake
* Boost Filesystem library
* Crypto++ library

On Ubuntu you can get these by:

    sudo apt-get install cmake libcrypto++-dev libboost-filesystem-dev

Simply clone and build.

    $ git clone https://github.com/feher/scracc.git
    $ cd scracc
    $ mkdir build
    $ cd build
    $ cmake ../
    $ make

On Debian systems, after the above steps you can also build an installable deb package:

    $ make package
    $ sudo apt-get install ./*.deb


Plans
------------

* Add more convenience functions to to libscracc.
* Allow linking with arbitrary libraries.
* Allow multiple scracc files for a single build.
  This would allow you to include several files into your scracc file. So you can split up your code to reasonable chunks.

Enjoy!
------------
