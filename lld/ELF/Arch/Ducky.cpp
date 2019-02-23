//===- Ducky.cpp ----------------------------------------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// AVR is a Harvard-architecture 8-bit micrcontroller designed for small
// baremetal programs. All AVR-family processors have 32 8-bit registers.
// The tiniest AVR has 32 byte RAM and 1 KiB program memory, and the largest
// one supports up to 2^24 data address space and 2^22 code address space.
//
// Since it is a baremetal programming, there's usually no loader to load
// ELF files on AVRs. You are expected to link your program against address
// 0 and pull out a .text section from the result using objcopy, so that you
// can write the linked code to on-chip flush memory. You can do that with
// the following commands:
//
//   ld.lld -Ttext=0 -o foo foo.o
//   objcopy -O binary --only-section=.text foo output.bin
//
// Note that the current AVR support is very preliminary so you can't
// link any useful program yet, though.
//
//===----------------------------------------------------------------------===//

#include "InputFiles.h"
#include "Symbols.h"
#include "Target.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"

#define DEBUG_TYPE "ducky-lld"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class Ducky final : public TargetInfo {
public:
  RelExpr getRelExpr(RelType Type, const Symbol &S, const uint8_t *Loc) const override;
  void relocateOne(uint8_t *Loc, RelType Type, uint64_t Val) const override;
};
} // namespace

RelExpr Ducky::getRelExpr(RelType Type, const Symbol &S, const uint8_t *Loc) const
{
  switch(Type) {
    case R_DUCKY_PI_20:
    case R_DUCKY_PA_20:
    case R_DUCKY_PA_16:
      return R_PC;
    default:
      return R_ABS;
  }
}

void Ducky::relocateOne(uint8_t *Loc, RelType Type, uint64_t Val) const {
  unsigned Content = read32le(Loc);
  uint32_t Patched = 0;

  LLVM_DEBUG(llvm::dbgs() << getErrorLocation(Loc) << '\n');
  LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Type=" << Type << '\n');
  LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Val=" << formatv("{0:X}", Val) << '\n');
  LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Content=" << formatv("{0:X}", Content) << '\n');

  switch (Type) {
    case R_DUCKY_Sys_Data_4:
      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     R_DUCKY_Sys_Data_4\n");

      checkIntUInt(Loc, Val, 32, Type);
      Patched = (uint32_t)Val;

      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Patched=" << formatv("{0:X}", Patched) << '\n');
      write32le(Loc, Patched);
      break;

    case R_DUCKY_PI_20:
      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     R_DUCKY_PI_20\n");

      Val -= 4;

      checkIntUInt(Loc, Val, 20, Type);

      Patched = (Content & 0x00000FFF) | (Val << 12) | (1 << 11);

      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Patched=" << formatv("{0:X}", Patched) << '\n');
      write32le(Loc, Patched);
      break;

    case R_DUCKY_PA_20:
      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     R_DUCKY_PA_20\n");

      Val = ((int64_t)Val >> 2) - 1;

      checkIntUInt(Loc, Val, 20, Type);
      Patched = (Content & 0x00000FFF) | (Val << 12) | (1 << 11);

      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Patched=" << formatv("{0:X}", Patched) << '\n');
      write32le(Loc, Patched);
      break;

    case R_DUCKY_PA_16:
      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     R_DUCKY_PA_16\n");

      Val = ((int64_t)Val >> 2) - 1;

      checkIntUInt(Loc, Val, 16, Type);
      Patched = (Content & 0x0000FFFF) | (Val << 16) | (1 << 15);

      LLVM_DEBUG(llvm::dbgs() << "Ducky::relocateOne:     Patched=" << formatv("{0:X}", Patched) << '\n');
      write32le(Loc, Patched);
      break;

    default:
      error(getErrorLocation(Loc) + "unrecognized reloc " + toString(Type));
  }
  LLVM_DEBUG(dbgs() << '\n');
}

TargetInfo *elf::getDuckyTargetInfo() {
  static Ducky Target;
  return &Target;
}
