#!/usr/bin/env python

import emacstts
import cmd


class TestCmd(cmd.Cmd):

    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = ">>> "
        self._voices = [ v for v in emacstts.voice_list() ]

    def do_say(self, args):
        emacstts.async_say(args)

    def do_voices(self, args):
        for v in self._voices:
            print v,
        print

    def do_setvoice(self, args):
        if args in self._voices:
            emacstts.set_voice(args)

    def do_EOF(self, line):
        return True

    def complete_setvoice(self, text, line, begidx, endidx):
        if not text:
            rval = self._voices[:]
        else:
            rval = [ f for f in self._voices if f.startswith(text) ]
        return rval

def f(x):
    print x

if __name__ == "__main__":
    TestCmd().cmdloop()
