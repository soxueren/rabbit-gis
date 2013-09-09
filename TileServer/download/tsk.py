#!/usr/bin/env python
# --*-- coding:gbk --*--

import os
import sys
import logging

logger = logging.getLogger('tsk')

def from_lines(lines):
    _tsk = {}
    for line in lines:
	line = line.strip()
	if line=="" or line[0]=="#":
	    continue

	lr = line.split("=")
	if len(lr) !=2:
	    logger.warning("May be has error, %s" % line)
	    continue

	l,r = lr[0].strip().lower(), lr[1].strip()
	if l=="bbox":
	    bbox = r.split(",")
	    if len(bbox)==4:
		l,t,r,b = 0.0,0.0,0.0,0.0

		for i in range(4):
		    bbox[i] = bbox[i].strip()

		if bbox[0]: 
		    l = float(bbox[0]) 
		if bbox[1]: 
		    t = float(bbox[1]) 
		if bbox[2]:
		    r = float(bbox[2]) 
		if bbox[3]: 
		    b = float(bbox[3]) 
		_tsk['bbox'] = (l,t,r,b)
	    else:
		logging.error("Not correct, &s" % line)
		break
		
	elif l=="level":
	    levels = r.split(",")
	    lvls = []
	    for level in levels:
		level = level.strip()
		if not level.isdigit():
		    continue
		level = int(level)
		if level in lvls:
		    continue
		lvls.append(level)

	    if lvls:
		lvls.sort()
		_tsk['level'] = lvls
	    else:
		logging.error("Not correct, &s" % line)
		break

	elif l=="out":
	    _tsk['out'] = r
	elif l=="name":
	    _tsk['name'] = r
	elif l=="format":
	    _tsk['format'] = r
	elif l=="version":
	    _tsk['version'] = r
	
    return _tsk

def to_lines(_tsk):
    lines = []
    if 'bbox' in _tsk:
	lines.append("bbox=%s\n" % _tsk['bbox'])
    if 'level' in _tsk:
	lines.append("level=%s\n" % _tsk['level'])
    if 'out' in _tsk:
	lines.append("out=%s\n" % _tsk['out'])
    if 'name' in _tsk:
	lines.append("name=%s\n" % _tsk['name'])
    if 'format' in _tsk:
	lines.append("format=%s\n" % _tsk['format'])
    if 'version' in _tsk:
	lines.append("version=%s\n" % _tsk['version'])
    return lines
	


