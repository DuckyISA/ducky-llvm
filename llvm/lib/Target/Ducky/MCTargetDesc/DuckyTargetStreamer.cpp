//===-- DuckyTargetStreamer.cpp - Ducky Target Streamer Methods -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Ducky specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "DuckyTargetStreamer.h"

using namespace llvm;

DuckyTargetStreamer::DuckyTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
