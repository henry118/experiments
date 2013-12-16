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

    def say(self, text):
        print "say..."
        try:
            self._eng.endLoop()
        except:
            pass
        self._eng.say(text)
        self._eng.startLoop(False)
        self._eng.iterate()

    def shutup(self):
        print "shutup..."
        self._eng.stop()

    def louder(self):
        print "louder..."
        #self.say("louder")
        volume = self._eng.getProperty("volume")
        self._eng.setProperty("volume", volume+0.25)

    def quieter(self):
        print "quieter..."
        volume = self._eng.getProperty("volume")
        self._eng.setProperty("volume", volume-0.25)

    def faster(self):
        rate = self._eng.getProperty("rate")
        print "faster %d => %d" %(rate, rate+10)
        self._eng.setProperty("rate", rate+10)

    def slower(self):
        rate = self._eng.getProperty("rate")
        print "slower %d => %d" %(rate, rate-10)
        self._eng.setProperty("rate", rate-10)

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
                self._eng.setProperty("voice", voice.id)
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

def say(*text):
    ttsEng.say(text[0])

def shutup():
    ttsEng.shutup()

def louder():
    ttsEng.louder()

def quieter():
    ttsEng.quieter()

def faster():
    ttsEng.faster()

def slower():
    ttsEng.slower()

def voices():
    return ttsEng.voices()

def setvoice(name):
    ttsEng.setvoice(name)


##
# Main Program
##
if __name__ == "__main__":
    server = EPCServer(("localhost", 5085))
    server.register_function(say)
    server.register_function(stop)
    server.register_function(louder)
    server.register_function(quieter)
    server.register_function(faster)
    server.register_function(slower)
    server.register_function(voice_list)
    server.register_function(set_voice)
    server.print_port()
    server.serve_forever()
