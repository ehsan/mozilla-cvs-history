// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License
// at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
// the License for the specific language governing rights and
// limitations under the License.
//
// The Initial Developer of the Original Code is Kipp E.B. Hickman.

#ifndef __leaky_h_
#define __leaky_h_

#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include "dict.h"
#include "strset.h"

typedef unsigned int u_int;

struct Symbol;

struct SymbolNode {
  SymbolNode(Symbol* aSymbol) {
    symbol = aSymbol;
    next = NULL;
  }

  void* operator new(size_t size);
  void operator delete(void* ptr);

  Symbol* symbol;
  SymbolNode* next;

  static SymbolNode* freeList;
};

struct Symbol {
  char* name;
  u_long address;
  bool dumped;
  bool leaker;
  u_long calls;
  SymbolNode* parents;
  SymbolNode* children;
  u_long bytesDirectlyLeaked;
  u_long childBytesLeaked;

  bool NotDumped() const {
    return 0 == dumped;
  }

  void SetDumped() {
    dumped = 1;
  }

  void Init(const char* aName, u_long aAddress);
  void AddParent(Symbol* aParent);
  void AddChild(Symbol* aChild);
};

struct LoadMapEntry {
  char* name;			// name of .so
  u_long address;		// base address where it was mapped in
  LoadMapEntry* next;
};

struct leaky {
  leaky();
  ~leaky();

  void initialize(int argc, char** argv);
  void open();

  char*  applicationName;
  char*  logFile;
  char*  progFile;

  int   sortByFrequency;
  int   dumpAll;
  int   dumpGraph;
  int   dumpXML;
  int   quiet;
  int   showAll;
  int   showAddress;
  u_int  stackDepth;

  int   fd;
  malloc_log_entry* base;
  malloc_log_entry* last;
  u_int  buckets;
  MallocDict*  dict;

  u_long  mallocs;
  u_long  reallocs;
  u_long  frees;
  u_long  totalMalloced;
  u_long  errors;
  u_long  totalLeaked;

  int sfd;
  Symbol* externalSymbols;
  u_int usefulSymbols;
  u_int numExternalSymbols;
  StrSet exclusions;
  u_long lowestSymbolAddr;
  u_long highestSymbolAddr;

  LoadMapEntry* loadMap;

  void usageError();

  void LoadMap();

  void analyze();

  void dumpLog();
  void dumpEntryToLog(malloc_log_entry* lep);

#if 0
  void dumpToTree();
  void dumpEntryToTree(FILE* fp, malloc_log_entry* lep);
#endif

  void insertAddress(u_long address, malloc_log_entry* lep);
  void removeAddress(u_long address, malloc_log_entry* lep);

  void displayStackTrace(malloc_log_entry* lep);

  void ReadSymbols(const char* fileName, u_long aBaseAddress);
  void ReadSharedLibrarySymbols();
  void setupSymbols(const char* fileName);
  Symbol* findSymbol(u_long address);
  int excluded(malloc_log_entry* lep);

  void buildLeakGraph();
  Symbol* findLeakGraphRoot(Symbol* aStart, Symbol* aEnd);
  void dumpLeakGraph();
  void dumpLeakTree(Symbol* aRoot, int aIndent, bool aEven);

  static void indentBy(int aCount) {
    while (--aCount >= 0) fputs("  ", stdout);
  }
};

#endif /* __leaky_h_ */
