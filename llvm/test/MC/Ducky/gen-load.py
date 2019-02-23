#!/usr/bin/env python

from common import gen_pass_script, gen_fail_scripts

INSTR = {
    'lw': 1
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
        '{mnemonic} r0, r0',
        lambda M, O: [O, 0x00, 0x00, 0x00],
        None, None
    ),
    (
        '{mnemonic} sp, fp',
        lambda M, O: [0xC0 | O, 0xf7, 0x00, 0x00],
        None, None
    ),
    (
        '{mnemonic} r1, r2[0x79]',
        lambda M, O: [0x40 | O, 0x10, 0xf3, 0x00],
        None, None
    ),
    (
        '{mnemonic} r1, r2[-0x97]',
        lambda M, O: [0x40 | O, 0x10, 0xd3, 0xfe],
        None, None
    ),
#    (
#        '{mnemonic} r1, r2[-0x7ffe]',
#        lambda M, O: [O, 0x00, 0xff, 0xff],
#        None, None
#    ),
    (
        '{mnemonic} r1, r2[0x7fff]',
        lambda M, O: [0x40 | O, 0x10, 0xff, 0xff],
        None, None
    ),
    (
        '{mnemonic} r2, r3[0xffff]',
        None,
        lambda M, O: '<stdin>:5:{}: error: invalid operand'.format(len(mnemonic) + 1),
        'offset-out-of-range'
    ),
    (
        '{mnemonic} r2, r3[0xdeadbeef]',
        None,
        lambda M, O: '<stdin>:5:{}: error: invalid operand'.format(len(mnemonic) + 1),
        'offset-deadbeef'
    ),
    (
        '{mnemonic} r2, r3[dummy]',
        None,
        lambda M, O: '<stdin>:5:3: error: invalid operand',
        'offset-symbol'
    ),
    (
        'lw r2[0x00], r3',
        None,
        lambda M, O: '<stdin>:5:{}: error: unexpected token'.format(len(mnemonic) + 6),
        'offset-on-dest'
    ),
    (
        '{mnemonic} r1',
        None,
        '<stdin>:5:3: error: invalid operand',
        'just-dest'
    ),
    (
        '{mnemonic} 0x79',
        None,
        '<stdin>:5:3: error: invalid operand',
        'imm-first'
    ),
    (
        '{mnemonic} r2, 0x79',
        None,
        '<stdin>:5:3: error: invalid operand',
        'just-offset'
    )
]

for mnemonic, opcode in INSTR.iteritems():
    gen_pass_script(mnemonic, opcode, HEADER_PASS, TESTS)
    gen_fail_scripts(mnemonic, opcode, HEADER_FAIL, TESTS)
