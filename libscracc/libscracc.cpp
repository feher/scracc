//   This file is part of libscracc.
// 
//     Libscracc is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     Libscracc is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
// 
//     You should have received a copy of the GNU General Public License
//     along with libscracc.  If not, see <http://www.gnu.org/licenses/>.
//
// Author: Gabor Fekete
// Created: 2013-02-14 09:33:10+02:00
// Modified: 2013-02-14 09:33:10+02:00
// Description:
//

#include "libscracc.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/cryptlib.h> // md5
#include <cryptopp/hex.h> // md5
#include <cryptopp/md5.h> // md5

#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp> // absolute()
#include <boost/filesystem.hpp>

#include <cstdlib> // getenv(), system()

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cassert>


//####################################################################

#define SPP_EC_FINISH() \
    if (ok) { \
        *ok = !sErrorCode; \
    } \
    else if (sErrorCode && sThrowExceptions) { \
        throw runtime_error(sErrorCode.message()); \
    }

#define SPP_EC_FINISH_WITH_RET() \
    if (ok) { \
        *ok = !sErrorCode; \
    } \
    else if (sErrorCode && sThrowExceptions) { \
        throw runtime_error(sErrorCode.message()); \
    } \
    return ret

#define SPP_FINISH(msg) \
    if (ok) { \
        *ok = success; \
    } \
    else if (!success && sThrowExceptions) { \
        throw runtime_error((msg)); \
    }

#define SPP_FINISH_WITH_RET(msg) \
    if (ok) { \
        *ok = success; \
    } \
    else if (!success && sThrowExceptions) { \
        throw runtime_error((msg)); \
    } \
    return ret

//####################################################################

namespace
{

using namespace std;

/// Strip extra slashes from path.
/// Removes redundant slashes from the given path.
/// For example:
///
///     path:            returns:
///     ---------------------------
///     /usr/lib//       /usr/lib/
///     //usr///lib/     /usr/lib/
///     /usr/lib/        /usr/lib/
///     /usr/lib         /usr/lib
///
/// @return The stripped path.
string StripExtraSlashes(string path)
{
   size_t pos;
   while ( (pos = path.find("//")) != string::npos )
   {
      path = path.replace(pos, 2, "/");
   }
   return path;
}

} // namespace anonymous

//####################################################################

namespace Scracc
{

using namespace std;
using namespace boost::filesystem;

bool sThrowExceptions = true;
boost::system::error_code sErrorCode;

//####################################################################

void SetThrowExceptions(bool throwExceptions)
{
    sThrowExceptions = throwExceptions;
}

void SetEnv(const string & name, const string & value, bool * ok)
{
    assert(false);
}

string GetEnv(const string & name, bool * ok)
{
    char * v = getenv(name.c_str());
    bool success = (v != nullptr);
    string ret = (v ? string(v) : string(""));
    SPP_FINISH_WITH_RET(string("Environment variable not defined: ") + name);
}

string Md5Sum(const string & message, bool * ok)
{
    CryptoPP::Weak::MD5 hash;
    byte digest[ CryptoPP::Weak::MD5::DIGESTSIZE ];
    hash.CalculateDigest( digest, (byte*) message.c_str(), message.length() );
    CryptoPP::HexEncoder encoder;
    string ret;
    encoder.Attach( new CryptoPP::StringSink( ret ) );
    encoder.Put( digest, sizeof(digest) );
    encoder.MessageEnd();
    bool success = true;
    SPP_FINISH_WITH_RET("");
}

string ReadFile(const string & filePath, bool * ok)
{
    string ret;
    bool success = false;
    ifstream ifs(filePath);
    if (ifs.is_open()) {
        ret = string( (istreambuf_iterator<char>(ifs)) , istreambuf_iterator<char>() );
        success = true;
    }
    SPP_FINISH_WITH_RET(string("Cannot open file: ") + filePath);
}

void WriteFile(const string & filePath, const string & contents, bool * ok)
{
    bool success = false;
    ofstream ofs(filePath, ofstream::out | ofstream::trunc);
    if (ofs.is_open()) {
        ofs << contents;
        success = true;
    }
    SPP_FINISH(string("Cannot open file: ") + filePath);
}

int Execute(const string & command, bool * ok)
{
    int ret = system(command.c_str());
    bool success = (ret > -1);
    SPP_FINISH_WITH_RET(string("Cannot execute command: ") + command);
}

string GetCwd(bool * ok)
{
    string ret = current_path(sErrorCode).native();
    SPP_EC_FINISH_WITH_RET();
}

void ChDir(const string & dirPath, bool * ok)
{
    current_path(dirPath, sErrorCode);
    SPP_EC_FINISH();
}

void MkDir(const string & dirPath, bool * ok)
{
    create_directory(dirPath, sErrorCode);
    SPP_EC_FINISH();
}

void MkDirPath(const string & dirPath, bool * ok)
{
    create_directories(dirPath, sErrorCode);
    SPP_EC_FINISH();
}

void Remove(const string & filePath, bool * ok)
{
    remove(filePath, sErrorCode);
    SPP_EC_FINISH();
}

void RemoveAll(const string & filePath, bool * ok)
{
    remove_all(filePath, sErrorCode);
    SPP_EC_FINISH();
}

string BaseName(const string & filePath, bool * ok)
{
    string ret = path(filePath).filename().native();
    SPP_EC_FINISH_WITH_RET();
}

string DirName(const string & filePath, bool * ok)
{
    string ret = path(filePath).parent_path().native();
    SPP_EC_FINISH_WITH_RET();
}

string AbsolutePath(const string & relPath, bool * ok)
{
    bool success = true;
    string ret = absolute(path(relPath)).native();

    // absolute() does not eliminate ".." directories.
    // So, we have to do it ourselves.
    // /a/b/../c --> /a/c
    while (1) {
        const auto dotdotPos = ret.find("/../");
        if (dotdotPos == string::npos) {
            break;
        }
        const auto prevDirPos = ret.find_last_of("/", dotdotPos - 1);
        if (prevDirPos == string::npos) {
            success = false;
            SPP_FINISH_WITH_RET(string("Badly formatted path: ") + ret);
        }
        ret.erase(prevDirPos, dotdotPos + 3 - prevDirPos);
    }

    // absolute() does not eliminate "." directories.
    // So, we have to do it ourselves.
    // /a/b/./c --> /a/b/c
    while (1) {
        const auto dotPos = ret.find("/./");
        if (dotPos == string::npos) {
            break;
        }
        ret.erase(dotPos, 2);
    }

    SPP_EC_FINISH_WITH_RET();
}

/// Makes a path by concatenating the given parts.
/// Removes any extra slashes from the result.
/// For example:
///
///     { "/usr/", "/lib" }      "/usr/lib"
///     { "/usr", "lib" }        "/usr/lib"
///     { "//usr/", "//lib/" }   "/usr/lib/"
///     { "usr", "lib/" }        "usr/lib/"
///     { "/usr", "..", "lib" }  "/usr/../lib"
///
/// @return The path.
string BuildPath(initializer_list<string> parts)
{
   string path;
   auto len = parts.size();
   auto it = begin(parts);
   for (size_t i = 0; i < len; ++i, ++it)
   {
      path.append(*it);
      if (i != len - 1)
      {
         path.append("/");
      }
   }
   return StripExtraSlashes(path);
}

bool Exists(const string & filePath, bool * ok)
{
    bool ret = exists(filePath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

bool IsDir(const string & dirPath, bool * ok)
{
    bool ret = is_directory(dirPath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

bool IsRegularFile(const string & filePath, bool * ok)
{
    bool ret = is_regular_file(filePath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

bool IsFile(const string & filePath, bool * ok)
{
    bool ret = is_regular_file(filePath, sErrorCode) || is_other(filePath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

bool IsSymlink(const string & filePath, bool * ok)
{
    bool ret = is_symlink(filePath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

void Symlink(const string & to, const string & from, bool * ok)
{
    create_symlink(to, from, sErrorCode);
    SPP_EC_FINISH();
}

string ReadLink(const string & filePath, bool * ok)
{
    string ret = read_symlink(filePath, sErrorCode).native();
    SPP_EC_FINISH_WITH_RET();
}

uintmax_t FileSize(const string & filePath, bool * ok)
{
    auto ret = file_size(filePath, sErrorCode);
    SPP_EC_FINISH_WITH_RET();
}

string Owner(const string & path, bool * ok)
{
}

string Group(const string & path, bool * ok)
{
}

string Permission(const string & path, bool * ok)
{
}

void ChMod(const string & path, const string & mode, bool recursive, bool * ok)
{
    // do the recursive version with Find()!
}

void ChOwn(const string & path, const string & ownership, bool recursive, bool * ok)
{
}

void ChGrp(const string & path, const string & group, bool recursive, bool * ok)
{
}

vector<string> Find(const string & startPath,
                    function<bool(const string & path)> predicate,
                    size_t depth,
                    bool followSymlink)
{
    vector<string> files;
    recursive_directory_iterator dirend;
    recursive_directory_iterator dit(startPath,
                                     followSymlink
                                     ? symlink_option::recurse
                                     : symlink_option::no_recurse);
    while (++dit != dirend) {
        if (depth == 0 || (depth > 0 && dit.level() <= depth)) {
            path filePath = absolute((*dit).path(), startPath);
            if (predicate(filePath.native())) {
                files.push_back(filePath.native());
            }
        }
    }
    return files;
}

void FindAndDo(const string & startPath,
               function<bool(const string & path)> predicate,
               function<bool(const string & path)> action,
               int depth,
               bool followSymlink)
{
    recursive_directory_iterator dirend;
    recursive_directory_iterator dit(startPath,
                                     followSymlink
                                     ? symlink_option::recurse
                                     : symlink_option::no_recurse);
    while (++dit != dirend) {
        if (depth == 0 || (depth > 0 && dit.level() <= depth)) {
            path filePath = absolute((*dit).path(), startPath);
            if (predicate(filePath.native())) {
                if (!action(filePath.native())) {
                    break;
                }
            }
        }
    }
}

//####################################################################

} // namespace Scracc

//####################################################################
