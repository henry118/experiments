#!/usr/bin/env python2

import urllib
import urllib2
import json

args = {
    'input': '',
    'inputtype': 'py',
    'bg': 0,
    'ed': 20,
    'result': 'hanzi',
    'resultcoding': 'unicode',
    'ch_en': 0,
    'clientinfo': 'web'
}

def query(pinyin):
    global args
    args['input'] = pinyin
    encoded_args = urllib.urlencode(args)
    url = 'http://olime.baidu.com/py?' + encoded_args
    response = urllib2.urlopen(url).read()
    jr = json.loads(response)
    words = []
    for x in jr[0]:
        words.append(x[0])
    return words

if __name__ == '__main__':
    while True:
        py = raw_input('>> ')
        words = query(py)
        for x in words:
            print x, ' ',
        print
