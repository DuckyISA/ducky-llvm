#!/usr/bin/env python

from common import gen_pass_script, gen_fail_scripts

INSTR = {
    'add': 28,
    'sub': 29,
    'mul': 30,
    'div': 31,
    'udiv': 32,
    'mod': 33,
    'and': 34,
    'or': 35,
    'xor': 36,
    'shiftl': 38,
    'shiftr': 39,
    'shiftrs': 40
}

HEADER_PASS = """
; RUN: llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: not test -s %t

test:
"""

HEADER_FAIL = """
; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
"""

TESTS = [
    (
        '{mnemonic} r0, r1',
        lambda M, O: [O, 0x08, 0x00, 0x00],
        None, None
    ),
    (
        '{mnemonic} r1, sp',
        lambda M, O: [0x40 | O, 0xf8, 0x00, 0x00],
        None, None
    ),
    (
        '{mnemonic} fp, 0x79',
        lambda M, O: [0x80 | O, 0x07, 0xf3, 0x00],
        None, None
    ),
    (
        '{mnemonic} r0, 0x7fff',
        lambda M, O: [O, 0x00, 0xff, 0xff],
        None, None
    ),
    (
        '{mnemonic} r0, 0xffff',
        None,
        lambda M, O: '<stdin>:5:{}: error: immediate must fit into 15 bits'.format(len(mnemonic) + 8),
        'imm-out-of-range'
    ),
    (
        '{mnemonic} r0, 0xdeadbeef',
        None,
        lambda M, O: '<stdin>:5:{}: error: immediate must fit into 15 bits'.format(len(mnemonic) + 8),
        'deadbeef'
    ),
    (
        '{mnemonic} r1',
        None,
        '<stdin>:5:3: error: invalid operand',
        'just-reg'
    ),
    (
        '{mnemonic} 0x79',
        None,
        '<stdin>:5:3: error: invalid operand',
        'imm-first'
    )
]

for mnemonic, opcode in INSTR.iteritems():
    gen_pass_script(mnemonic, opcode, HEADER_PASS, TESTS)
    gen_fail_scripts(mnemonic, opcode, HEADER_FAIL, TESTS)
