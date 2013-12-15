#!/usr/bin/env python
# --*-- coding:gbk --*--

import os
import sys
import ConfigParser

def from_file(_path, _section):
    if not os.path.isfile(_path):
        return None
    _items = {}
    cfg = ConfigParser.ConfigParser()
    cfg.read(_path)
    if cfg.has_section(_section):
        for k,v in cfg.items(_section):
            _items[k] = v
    return _items

def to_file(_path, _section, _items):
    config = ConfigParser.ConfigParser()
    config.add_section(_section.lower())
    for k,v in _items.items():
        config.set(_section.lower(), k, v)

    with open(_path, 'wb') as configfile:
        config.write(configfile)

if __name__=="__main__":
    print 'unsupport main function.'


