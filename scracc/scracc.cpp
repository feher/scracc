//   This file is part of Scracc.
// 
//     Scracc is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     Scracc is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
// 
//     You should have received a copy of the GNU General Public License
//     along with Scracc.  If not, see <http://www.gnu.org/licenses/>.
// 
// Author: Gabor Fekete
// Created: 2013-02-14 09:33:10+02:00
// Modified: 2013-02-14 09:33:10+02:00
// Description:
//

// TODO:
// o AbsolutePath() returns "/some/path/../hello" -> this is crap!

#include "libscracc.h"

#include <boost/filesystem/operations.hpp> // absolute()
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


// check out for Execute*() : nice SMALL template lib
// http://pstreams.sourceforge.net/
//
// for graphing:
// http://mathgl.sourceforge.net/
//
// must be able to link/use arbitrary C++ lib!
// Special comments!
// //scracc: -lmathgl -lvtk
// //scracc: -I...
//

using namespace std;
using namespace boost::filesystem;

#define DO_DEBUG 0
#if DO_DEBUG
#define DEBUG(msg) \
    do { \
        std::cout << (msg) << std::endl; \
    } while (0)
#else
#define DEBUG(msg)
#endif

class Builder
{
public:
    Builder(const vector<string> & args);
    int BuildAndRun();
private:
    string mInputFilePath;
    string mCacheDir;
    string mCacheBin;
    string mCacheMd5;
    string mCacheSrc;
    string mBuildDir;
    string mBuildSrc;
    string mInputFileName;
    string mInputFileContentMd5;
    string mInputFilePathMd5;
    vector<string> mArgs;

    void GenerateSourceCode(const string & inputFile = "", ofstream * ostr = nullptr);
    bool Compile();
    int  Run();
    void RefreshCache();
    void CleanupCache();
    bool Changed();
};

/// Builds and executes the scracc source file.
///
///     scracc test.scc args-for-test.cc
///
/// @param args args[0] is the source file name
///             args[N] are the command line arguments for the compiled source
Builder::Builder(const vector<string> & args)
{
    if (args.size() < 1) {
        throw runtime_error("Not enough arguments!");
    }
    mArgs.assign(++(begin(args)), end(args));

    mInputFilePath = Scracc::AbsolutePath(args[0]);
    mInputFileName = Scracc::BaseName(mInputFilePath);

    auto homeDir = Scracc::GetEnv("HOME");
    mCacheDir = Scracc::AbsolutePath(string(homeDir) + "/.cache/scracc");
    Scracc::MkDirPath(mCacheDir);

    mInputFileContentMd5 = Scracc::Md5Sum(Scracc::ReadFile(mInputFilePath));
    mInputFilePathMd5 = Scracc::Md5Sum(mInputFilePath);
    DEBUG(string("content md5 = ") + mInputFileContentMd5);
    DEBUG(string("path md5 = ") + mInputFilePathMd5);

    mCacheBin = Scracc::BuildPath( { mCacheDir, mInputFilePathMd5 + ".bin" } );
    mCacheMd5 = Scracc::BuildPath( { mCacheDir, mInputFilePathMd5 + ".md5" } );
    mCacheSrc = Scracc::BuildPath( { mCacheDir, mInputFilePathMd5 + ".src" } );

    // TODO: use only part of the MD5 here!
    mBuildDir = Scracc::BuildPath( { mCacheDir, mInputFilePathMd5 } );
    mBuildSrc = Scracc::BuildPath( { mBuildDir, mInputFileName + ".cc" } );
}

void Builder::GenerateSourceCode(const string & inputFile, ofstream * ostr)
{
    ifstream ifs;
    if (inputFile.empty()) {
       ifs.open(mInputFilePath);
    }
    else {
       ifs.open(inputFile);
    }
    if (!ifs.is_open()) {
        throw runtime_error("Cannot open: " + inputFile);
    }
    ofstream * ofs = ostr;
    if (!ostr) {
        ofs = new ofstream(mBuildSrc);
        *ofs << "// Generated by scracc" << endl
             << "#include <scracc/libscracc.h>" << endl
             << "#include <iostream>" << endl
             << "using namespace Scracc;" << endl
             << "using namespace std;" << endl;
    }
    while (ifs.good()) {
        string line;
        getline(ifs, line);
        if (line.find("#include") != string::npos
            && line.find(".scc\"") != string::npos) {
            string includeFile = "segges";
            GenerateSourceCode(includeFile, ofs);
        }
        if (line.find("#!/") != string::npos
            && line.find("scracc") != string::npos) {
            // skip
        }
        else {
            *ofs << line << endl;
        }
    }
    if (!ostr) {
        delete ofs;
    }
}

bool Builder::Compile()
{
    Scracc::MkDirPath(mBuildDir);
    string curDir = Scracc::GetCwd();
    Scracc::ChDir(mBuildDir);

    GenerateSourceCode();

    string cmd = "g++ -std=c++11 -o " + mCacheBin + " " + mBuildSrc + " -lscracc";
    DEBUG(string("comp cmd: ") + cmd);
    bool ok = (Scracc::Execute(cmd) == 0);

    Scracc::RemoveAll(mBuildDir);
    Scracc::ChDir(curDir);

    return ok;
}

int Builder::Run()
{
    string command = mCacheBin;
    for (auto arg : mArgs) {
        command.append(" \"").append(arg).append("\"");
    }
    DEBUG(string("cmd = ") + command);
    return Scracc::Execute(command);
}

void Builder::RefreshCache()
{
    if (!Compile()) {
        throw runtime_error("Compilation FAILED!");
    }
    Scracc::WriteFile(mCacheMd5, mInputFileContentMd5);
    Scracc::WriteFile(mCacheSrc, mInputFilePath);
}

void Builder::CleanupCache()
{
}

bool Builder::Changed()
{
    bool changed = true;
    try {
        string cachedMd5 = Scracc::ReadFile(mCacheMd5);
        DEBUG(string("cached md5 =") + cachedMd5);
        return mInputFileContentMd5 != cachedMd5;
    }
    catch (runtime_error & e) {
    }
    return changed;
}

int Builder::BuildAndRun()
{
    if (Changed()) {
        RefreshCache();
    }
    return Run();
}

int main(int argc, char ** argv)
{
    using namespace std;

    Scracc::SetThrowExceptions(true);

    vector<string> args;
    for (size_t i = 1; i < argc; ++i) {
        args.push_back(string(argv[i]));
    }

    int ret = 255;
    try {
        Builder builder(args);
        ret = builder.BuildAndRun();
    }
    catch (runtime_error & e) {
        cout << "FAILED: " << e.what() << endl;
    }
    return ret;
}
