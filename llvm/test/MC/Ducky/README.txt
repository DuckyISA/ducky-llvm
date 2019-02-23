To refresh generated test cases, run gen-*.py scripts.



class DuckyOpcodes(enum.IntEnum):
  NOP    =  0

  # Memory load/store
  LW     =  1
  LS     =  2
  LB     =  3
  STW    =  4
  STS    =  5
  STB    =  6
  CAS    =  7
  LA     =  8

  LI     =  9
  LIU    = 10
  MOV    = 11
  SWP    = 12

  INT    = 13
  RETINT = 14

  CALL   = 15
  RET    = 16

  CLI    = 17
  STI    = 18
  RST    = 19
  HLT    = 20
  IDLE   = 21
  LPM    = 22
  IPI    = 23

  PUSH   = 24
  POP    = 25

  INC    = 26
  DEC    = 27
  ADD    = 28
  SUB    = 29
  MUL    = 30
  DIV    = 31
  UDIV   = 32
  MOD    = 33

  AND    = 34
  OR     = 35
  XOR    = 36
  NOT    = 37
  SHL    = 38
  SHR    = 39
  SHRS   = 40

  # Branch instructions
  J      = 46

  # Condition instructions
  CMP    = 47
  CMPU   = 48
  SET    = 49
  BRANCH = 50
  SELECT = 51

  # Control instructions
  CTR    = 60
  CTW    = 61
  FPTC   = 62
  SIS    = 63

