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

#include <cstdint> // uintmax_t

#include <functional> // function<>
#include <initializer_list>

#include <vector>
#include <string>

namespace Scracc
{

using namespace std;

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

} // namespace Scracc

