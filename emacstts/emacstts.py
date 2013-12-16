#!/usr/bin/env python

import pyttsx
from epc.server import *

##
# TTS class
##
class EmacsTTS:
    def __init__(self):
        self._eng = pyttsx.init()
        self._eng.connect("started-utterance", self._onStart)
        self._eng.connect("started-word", self._onWord)
        self._eng.connect("finished-utterance", self._onEnd)

    def _prepare(self):
        try:
            self._eng.endLoop()
        except:
            pass

    def say(self, text):
        print "say..."
        self._prepare()
        self._eng.say(text)
        self._eng.startLoop(False)
        for x in xrange(0,10):
            self._eng.iterate()

    def shutup(self):
        print "shutup..."
        self._eng.stop()

    def louder(self):
        self._prepare()
        vol = self._eng.getProperty("volume")
        newvol = vol + 0.25
        print "louder %f => %f" %(vol, newvol)
        self._eng.setProperty("volume", newvol)
        self._eng.runAndWait()
        self.say("louder")

    def quieter(self):
        self._prepare()
        vol = self._eng.getProperty("volume")
        newvol = vol - 0.25
        print "quieter %f => %f" %(vol, newvol)
        self._eng.setProperty("volume", newvol)
        self._eng.runAndWait()
        self.say("quieter")

    def faster(self):
        self._prepare()
        rate = self._eng.getProperty("rate")
        newrate = rate + 50
        print "faster %d => %d" %(rate, newrate)
        self._eng.setProperty("rate", newrate)
        self._eng.runAndWait()
        self.say("faster")

    def slower(self):
        self._prepare()
        rate = self._eng.getProperty("rate")
        newrate = rate - 50
        print "slower %d => %d" %(rate, newrate)
        self._eng.setProperty("rate", newrate)
        self._eng.runAndWait()
        self.say("slower")

    def voices(self):
        print "voices..."
        v = []
        for voice in self._eng.getProperty("voices"):
            v.append(voice.name)
        return v

    def setvoice(self, name):
        print "setvoice..."
        for voice in self._eng.getProperty("voices"):
            if voice.name == name:
                self._prepare()
                self._eng.setProperty("voice", voice.id)
                self._eng.runAndWait()
                self.say("I am %s." % voice.name)
                break

    def _onStart(self, name):
        print "onStart..."

    def _onWord(self, name, location, length):
        print "onWord..."

    def _onEnd(self, name, completed):
        print "onEnd..."


##
# Public interfaces
##
ttsEng = EmacsTTS()

def say(*args):
    try:
        ttsEng.say(args[0])
    except e, Exception:
        print e
    return args

def shutup():
    try:
        ttsEng.shutup()
    except e, Exception:
        print e

def louder():
    try:
        ttsEng.louder()
    except e, Exception:
        print e

def quieter():
    try:
        ttsEng.quieter()
    except e, Exception:
        print e

def faster():
    try:
        ttsEng.faster()
    except e, Exception:
        print e

def slower():
    try:
        ttsEng.slower()
    except e, Exception:
        print e

def voices():
    try:
        return ttsEng.voices()
    except e, Exception:
        print e
    return ""

def setvoice(*args):
    try:
        ttsEng.setvoice(args[0])
    except e, Exception:
        print e


##
# Main Program
##
if __name__ == "__main__":
    server = EPCServer(("localhost", 5085))
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
