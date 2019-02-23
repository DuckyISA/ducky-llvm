//===-- DuckySelectionDAGInfo.h - Ducky SelectionDAG Info -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Ducky subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef DuckySELECTIONDAGINFO_H
#define DuckySELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class DuckySelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  ~DuckySelectionDAGInfo();
};
}

#endif
