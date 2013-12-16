#!/usr/bin/env python

# Copyright (C) 2013-  Henry Wang <wh_henry@hotmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import emacstts
import cmd
import pyttsx
import readline
import rlcompleter

if 'libedit' in readline.__doc__:
    readline.parse_and_bind("bind ^I rl_complete")
else:
    readline.parse_and_bind("tab: complete")


class TestCmd(cmd.Cmd):
    """A console for test command input"""

    def __init__(self):
        """Construct the console"""
        cmd.Cmd.__init__(self)
        self.prompt = ">>> "
        self._voices = [ v for v in emacstts.voices() ]

    def do_say(self, args):
        """Test the speaking function"""
        emacstts.say(args)

    def do_shutup(self, args):
        """Test the stopping function"""
        emacstts.shutup()

    def do_voices(self, args):
        """Test the voice listing function"""
        for v in self._voices:
            print v,
        print

    def do_setvoice(self, args):
        """Test the voice setting function"""
        if args in self._voices:
            emacstts.setvoice(args)

    def do_louder(self, args):
        """Test the volume setting"""
        emacstts.louder()

    def do_quieter(self, args):
        """Test the volume setting"""
        emacstts.quieter()

    def do_faster(self, args):
        """Test the rate setting"""
        emacstts.faster()

    def do_slower(self, args):
        """Test the rate setting"""
        emacstts.slower()

    def do_EOF(self, line):
        """Exit the console"""
        return True

    def complete_setvoice(self, text, line, begidx, endidx):
        """Voice candidate completions"""
        if not text:
            rval = self._voices[:]
        else:
            rval = [ f for f in self._voices if f.startswith(text) ]
        return rval


if __name__ == "__main__":
    TestCmd().cmdloop()
