#!/usr/bin/env python

import emacstts
import cmd
import threading
import Queue
import pyttsx

class TestCmd(cmd.Cmd):

    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = ">>> "
        self._voices = [ v for v in emacstts.voices() ]

    def do_say(self, args):
        emacstts.say(args)

    def do_shutup(self, args):
        print "shutup"
        emacstts.shutup()

    def do_voices(self, args):
        for v in self._voices:
            print v,
        print

    def do_setvoice(self, args):
        if args in self._voices:
            emacstts.setvoice(args)

    def do_louder(self, args):
        emacstts.louder()

    def do_quieter(self, args):
        emacstts.quieter()

    def do_faster(self, args):
        emacstts.faster()

    def do_slower(self, args):
        emacstts.slower()

    def do_EOF(self, line):
        return True

    def complete_setvoice(self, text, line, begidx, endidx):
        if not text:
            rval = self._voices[:]
        else:
            rval = [ f for f in self._voices if f.startswith(text) ]
        return rval


if __name__ == "__main__":
    TestCmd().cmdloop()
