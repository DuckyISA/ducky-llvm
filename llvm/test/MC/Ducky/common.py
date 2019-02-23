def gen_pass_script(mnemonic, opcode, header, tests):
    with open('inst-{}.s'.format(mnemonic), 'w') as f:
        print >> f, header.strip()

        for asmstr, encoding_getter, _, _ in tests:
            if not encoding_getter:
                continue

            print >> f, "  {}".format(asmstr.format(**locals()))

        print >> f

        for asmstr, encoding_getter, _, _ in tests:
            if not encoding_getter:
                continue

            print >> f, '; CHECK: {} ; encoding: [{}]'.format(asmstr.format(**locals()), ','.join(['0x%02x' % b for b in encoding_getter(mnemonic, opcode)]))

def gen_fail_scripts(mnemonic, opcode, header, tests):
    for asmstr, _, error_message, filename in tests:
        if not error_message:
            continue

        with open('inst-{mnemonic}-{filename}.s'.format(**locals()), 'w') as f:
            print >> f, header.strip()
            print >> f, "  {}".format(asmstr.format(**locals()))
            print >> f

            if callable(error_message):
                error_message = error_message(mnemonic, opcode)

            print >> f, '; CHECK-ERROR: {}'.format(error_message)
            print >> f, '; CHECK-ERROR:   {}'.format(asmstr.format(**locals()))
