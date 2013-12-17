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

import pyttsx
import logging
import argparse
from epc.server import *


### TTS class ###
class EmacsTTS:
    """A pyttsx wrapper class that provides convenient interface to clients"""

    def __init__(self):
        """Construct the inner pyttsx engine and setup the callbacks"""
        self._eng = pyttsx.init()
        self._eng.connect("started-utterance", self._onStart)
        self._eng.connect("started-word", self._onWord)
        self._eng.connect("finished-utterance", self._onEnd)

    def _prepare(self):
        """Helper method that make sure the engine loop is terminated"""
        try:
            self._eng.endLoop()
        except:
            pass

    def _flush(self):
        """Helper method that flushes the engine's command queue"""
        for x in xrange(0,10):
            self._eng.iterate()

    def say(self, text):
        """Request the engine to speak supplied text string"""
        logging.debug("say...")
        self._prepare()
        self._eng.say(text)
        self._eng.startLoop(False)
        self._flush()

    def shutup(self):
        """Ask the engine to stop speaking"""
        logging.debug("shutup...")
        self._eng.stop()

    def louder(self):
        """Increase the voice's volume"""
        self._prepare()
        vol = self._eng.getProperty("volume")
        newvol = vol + 0.25
        logging.debug("louder %f => %f" %(vol, newvol))
        self._eng.setProperty("volume", newvol)
        self._eng.runAndWait()
        self.say("louder")

    def quieter(self):
        """Reduce the voice's volume"""
        self._prepare()
        vol = self._eng.getProperty("volume")
        newvol = vol - 0.25
        logging.debug("quieter %f => %f" %(vol, newvol))
        self._eng.setProperty("volume", newvol)
        self._eng.runAndWait()
        self.say("quieter")

    def faster(self):
        """Increase the speaking speed (number of words per minute)"""
        self._prepare()
        rate = self._eng.getProperty("rate")
        newrate = rate + 50
        logging.debug("faster %d => %d" %(rate, newrate))
        self._eng.setProperty("rate", newrate)
        self._eng.runAndWait()
        self.say("faster")

    def slower(self):
        """Reduce the speaking speed (number of words per minute)"""
        self._prepare()
        rate = self._eng.getProperty("rate")
        newrate = rate - 50
        logging.debug("slower %d => %d" %(rate, newrate))
        self._eng.setProperty("rate", newrate)
        self._eng.runAndWait()
        self.say("slower")

    def voices(self):
        """Return a list of voices available on the system"""
        logging.debug("voices...")
        v = []
        for voice in self._eng.getProperty("voices"):
            v.append(voice.name)
        return v

    def setvoice(self, name):
        """Set the current voice to the supplied person's name"""
        logging.debug("setvoice...")
        for voice in self._eng.getProperty("voices"):
            if voice.name == name:
                self._prepare()
                self._eng.setProperty("voice", voice.id)
                self._eng.runAndWait()
                self.say("I am %s." % voice.name)
                break

    def _onStart(self, name):
        """Callback method when a utterance is spoken"""
        logging.debug("onStart...")

    def _onWord(self, name, location, length):
        """Callback method when a word is spoken"""
        logging.debug("onWord...")

    def _onEnd(self, name, completed):
        """Callback method when a utterance is finished spoken"""
        logging.debug("onEnd...")


### Public interfaces ###
ttsEng = EmacsTTS()

def say(*args):
    """Request the engine to speak given text"""
    try:
        ttsEng.say(args[0])
    except Exception, e:
        logging.error(e)

def shutup():
    """Ask the engine to stop speaking"""
    try:
        ttsEng.shutup()
    except Exception, e:
        logging.error(e)

def louder():
    """Increase the voice's volume"""
    try:
        ttsEng.louder()
    except Exception, e:
        logging.error(e)

def quieter():
    """Reduce the voice's volume"""
    try:
        ttsEng.quieter()
    except Exception, e:
        logging.error(e)

def faster():
    """Increase the speaking rate (words/min)"""
    try:
        ttsEng.faster()
    except Exception, e:
        logging.error(e)

def slower():
    """Reduce the speaking rate (words/min)"""
    try:
        ttsEng.slower()
    except Exception, e:
        logging.error(e)

def voices():
    """Return a list of voices avaiable on the system"""
    try:
        return ttsEng.voices()
    except Exception, e:
        logging.error(e)
    return ""

def setvoice(*args):
    """Set the current voice to the give person"""
    try:
        ttsEng.setvoice(args[0])
    except Exception, e:
        logging.error(e)

def main(port):
    """Entry point of the EPC server"""
    try:
        server = EPCServer(("localhost", port))
        server.register_function(say)
        server.register_function(shutup)
        server.register_function(louder)
        server.register_function(quieter)
        server.register_function(faster)
        server.register_function(slower)
        server.register_function(voices)
        server.register_function(setvoice)
        server.print_port()
        server.serve_forever()
    except Exception, e:
        logging.error(e)

### Main Program ###
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("port", type=int, nargs="?", default=0, help="The EPC server binding port")
    parser.add_argument("--log", default="error", help="Log level")
    args = parser.parse_args()
    logging.basicConfig(level=getattr(logging, args.log.upper()))
    main(args.port)
