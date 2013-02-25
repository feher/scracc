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

#include "libscracc.h"

#include <boost/filesystem/operations.hpp> // absolute()
#include <boost/filesystem.hpp>

#include <iterator>  // istream_iterator
#include <algorithm> // copy()
#include <sstream>   // istringstream
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

//####################################################################

#define DO_DEBUG 0
#if DO_DEBUG
#define DEBUG(msg) \
    do { \
        std::cout << (msg) << std::endl; \
    } while (0)
#else
#define DEBUG(msg)
#endif

//####################################################################

namespace {

void ShowHelp()
{
    cout << "This is Scracc v0.1.2." << endl;
    cout << "Scracc is a C++ Prototyping tool." << endl;
    cout << endl;
    cout << "Usage: scracc options input-file input-file-args" << endl;
    cout << endl;
    cout << "options              Scracc specific options." << endl;
    cout << "    -c | --clean-slate   Do not inlude any headers by default." << endl;
    cout << "                         Do not define any default using directives." << endl;
    cout << "    -r | --recompile     Force recompilation of input-file." << endl;
    cout << "    -d | --debug         Compile with debug information." << endl;
    cout << "                         This will use the cache directory for building," << endl;
    cout << "                         ignoring the SCRACC_BUILD_DIR variable." << endl;
    cout << "    -n | --nocache       Do not cache the compiled executable." << endl;
    cout << "                         It also deletes any existing cached binaries." << endl;
    cout << "    -h | --help          Print this help screen." << endl;
    cout << "                     NOTE: -n and -d are mutually exclusive." << endl;
    cout << endl;
    cout << "input-file           The scc file you want to compile and run." << endl;
    cout << "                     This argument cannot start with \"-\"!" << endl;
    cout << "input-file-args      These will be passed to the input-file as command line arguments." << endl;
    cout << endl;
}

} // namespace anonymous

//####################################################################

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
    string mCacheLoc;
    string mCacheSrc;
    string mBuildDir;
    string mBuildSrc;
    string mInputFileName;
    string mInputFileContentMd5;
    string mInputFilePathMd5;
    bool mCleanSlate;
    bool mRecompile;
    bool mDebug;
    bool mNoCache;
    vector<string> mArgs;

    size_t ProcessCommandlineArguments(const vector<string> & args);
    void GenerateSourceCode(const string & inputFile = "", ofstream * ostr = nullptr);
    bool Compile();
    int  Run();
    void RefreshCache();
    void GarbageCollectCache();
    bool Changed();
};

/// Builds and executes the scracc source file.
///
///     scracc test.scc args-for-test.cc
///
/// @param args args[0] is the source file name
///             args[N] are the command line arguments for the compiled source
Builder::Builder(const vector<string> & args)
    :  mCleanSlate (false)
      ,mRecompile (false)
      ,mDebug (false)
      ,mNoCache (false)
{
    auto processedArgCount = ProcessCommandlineArguments(args);
    
    if (args.size() - processedArgCount < 1) {
        throw runtime_error("Not enough arguments!");
    }

    mInputFilePath = Scracc::AbsolutePath(args[processedArgCount]);
    mInputFileName = Scracc::BaseName(mInputFilePath);

    mArgs.assign(begin(args) + processedArgCount + 1, end(args));

    mInputFileContentMd5 = Scracc::Md5Sum(Scracc::ReadFile(mInputFilePath));
    mInputFilePathMd5 = Scracc::Md5Sum(mInputFilePath);
    DEBUG(string("content md5 = ") + mInputFileContentMd5);
    DEBUG(string("path md5 = ") + mInputFilePathMd5);

    bool scraccOk = true;
    auto cacheDir = Scracc::GetEnv("SCRACC_CACHE_DIR", &scraccOk);
    if (cacheDir.empty()) {
        const auto homeDir = Scracc::GetEnv("HOME");
        mCacheDir = Scracc::AbsolutePath(
                                Scracc::BuildPath( { string(homeDir) ,
                                                     ".cache/scracc",
                                                     mInputFilePathMd5 } ) );
    }
    else {
        mCacheDir = Scracc::AbsolutePath(
                                Scracc::BuildPath( { cacheDir,
                                                     mInputFilePathMd5 } ) );
    }

    mCacheBin = Scracc::BuildPath( { mCacheDir, mInputFileName + ".bin" } );
    mCacheMd5 = Scracc::BuildPath( { mCacheDir, mInputFileName + ".md5" } );
    mCacheLoc = Scracc::BuildPath( { mCacheDir, mInputFileName + ".loc" } );
    mCacheSrc = Scracc::BuildPath( { mCacheDir, mInputFileName + ".cc" } );

    if (mDebug) {
        mBuildDir = mCacheDir;
    } else {
        mBuildDir = Scracc::GetEnv("SCRACC_BUILD_DIR", &scraccOk);
        if (mBuildDir.empty()) {
            mBuildDir = mCacheDir;
        } else {
            mBuildDir = Scracc::BuildPath( { mBuildDir, mInputFilePathMd5 } );
        }
    }
    mBuildSrc = Scracc::BuildPath( { mBuildDir, mInputFileName + ".cc" } );
}

size_t Builder::ProcessCommandlineArguments(const vector<string> & args)
{
    size_t i = 0;
    for (i = 0; i < args.size(); ++i) {
        if (args[i].find("-") != 0) {
            break;
        }
        if (args[i] == "-c" || args[i] == "--clean-slate") {
            mCleanSlate = true;
        }
        else if (args[i] == "-r" || args[i] == "--recompile") {
            mRecompile = true;
        }
        else if (args[i] == "-d" || args[i] == "--debug") {
            if (mNoCache) {
                throw runtime_error("-d and -n are mutually exclusive!");
            }
            mDebug = true;
        }
        else if (args[i] == "-n" || args[i] == "--nocache") {
            if (mDebug) {
                throw runtime_error("-d and -n are mutually exclusive!");
            }
            mNoCache = true;
        }
        else if (args[i] == "-h" || args[i] == "--help") {
            ShowHelp();
            throw runtime_error("Help requested.");
        }
        else {
            throw runtime_error(string("Unrecognized option: ") + args[i]);
        }
    }
    return i;
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
        *ofs << "// Generated by scracc" << endl;
        if (!mCleanSlate) {
            *ofs << "#include <scracc/libscracc.h>" << endl
                 << "#include <iostream>" << endl
                 << "using namespace Scracc;" << endl
                 << "using namespace std;" << endl;
        }
    }
    while (ifs.good()) {
        string line;
        getline(ifs, line);
        if (line.find("#!/") != string::npos
            && line.find("scracc") != string::npos) {
            continue;
        }
        if (line.find("#include") != string::npos
            && line.find(".scc\"") != string::npos) {
            string includeFile = "segges";
            GenerateSourceCode(includeFile, ofs);
        }
        *ofs << line << endl;
    }
    if (!ostr) {
        delete ofs;
    }
}

bool Builder::Compile()
{
    Scracc::MkDirPath(mCacheDir);
    Scracc::MkDirPath(mBuildDir);
    const string curDir = Scracc::GetCwd();
    Scracc::ChDir(mBuildDir);

    GenerateSourceCode();

    const string defaultLibs = mCleanSlate ? "" : " -lscracc";
    const string debugBuild = mDebug ? "-g" : "";
    const string cmd = "g++ -std=c++11 " + debugBuild + " -o " + mCacheBin + " " + mBuildSrc + defaultLibs;
    DEBUG(string("comp cmd: ") + cmd);
    bool ok = (Scracc::Execute(cmd) == 0);

    if (mBuildDir != mCacheDir) {
        Scracc::RemoveAll(mBuildDir);
    }
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
    Scracc::WriteFile(mCacheLoc, mInputFilePath);
}

void Builder::GarbageCollectCache()
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
    if (mNoCache || mRecompile || Changed()) {
        RefreshCache();
    }
    if (mDebug) {
        assert(mCacheSrc == mBuildSrc);
        cout << "SCRACC Executable: " << mCacheBin << endl;
        cout << "SCRACC Source:     " << mCacheSrc << endl;
    }
    int ret = Run();
    if (mNoCache) {
        Scracc::RemoveAll(mCacheDir);
    }
    return ret;
}

int main(int argc, char ** argv)
{
    using namespace std;

    Scracc::SetThrowExceptions(true);

    size_t i = 1;
    vector<string> args;

    // The arguments passed through shebang (aka #!) are in a single string.
    // We have to spit it up.
    if (argc > 1 && argv[1][0] == '-') {
        auto arg = string(argv[1]);
        istringstream iss(arg);
        copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter<vector<string> >(args));
        ++i;
    }

    // The rest are OK.
    for (; i < argc; ++i) {
        args.push_back(string(argv[i]));
    }

    int ret = 255;
    try {
        Builder builder(args);
        ret = builder.BuildAndRun();
    }
    catch (runtime_error & e) {
        cout << "STOPPED: " << e.what() << endl;
    }
    return ret;
}
