#!/usr/bin/env python

import pyttsx
from multiprocessing import Pool
from epc.server import *

def say(*text):
    for msg in text:
        print "say %s" %(msg)
        ttsEng.say(msg)
    ttsEng.runAndWait()

def async_say(*text):
    ttsPool.apply_async(say, text)

def stop():
    ttsEng.stop()

def louder():
    volume = ttsEng.getProperty("volume")
    ttsEng.setProperty("volume", volume+0.25)

def quieter():
    volume = ttsEng.getProperty("volume")
    ttsEng.setProperty("volume", volume-0.25)

def faster():
    rate = ttsEng.getProperty("rate")
    ttsEng.setProperty("rate", rate+50)

def slower():
    rate = ttsEng.getProperty("rate")
    ttsEng.setProperty("rate", rate-50)

def voice_list():
    v = []
    for voice in ttsEng.getProperty("voices"):
        v.append(voice.id)
    return v

def set_voice(voice):
    ttsEng.setProperty("voice", voice)

def _onStart(name):
    print "onStart..."

def _onWord(name, location, length):
    pass

def _onEnd(name, completed):
    print "onEnd..."

ttsEng = pyttsx.init()
ttsEng.connect("started-utterance", _onStart)
ttsEng.connect("started-word", _onWord)
ttsEng.connect("finished-utterance", _onEnd)
ttsPool = Pool()

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
