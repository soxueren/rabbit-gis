#!/usr/bin/env python
# --*-- coding:utf-8 --*--


import numpy
from array import array

buf_xsize = 100
buf_ysize = 30
buf_tile_server = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ÿÿ  ÿÿ                                                                                  ÿÿÿÿÿÿÿÿÿÿ  ÿÿ  ÿÿ              ÿÿÿÿÿÿÿÿ                                                                ÿÿ          ÿÿ             ÿÿÿÿÿÿÿÿÿ                                                                ÿÿ          ÿÿ             ÿÿÿ   ÿÿÿ                                                                ÿÿ      ÿÿ  ÿÿ   ÿÿÿÿÿÿÿ   ÿÿÿ         ÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿ    ÿÿÿÿ ÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿ               ÿÿ      ÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ    ÿÿÿ ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ               ÿÿ      ÿÿ  ÿÿ  ÿÿÿ   ÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿ   ÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ ÿÿÿ   ÿÿÿ  ÿÿÿÿÿ                 ÿÿ      ÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿ      ÿÿÿ  ÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿ                    ÿÿ      ÿÿ  ÿÿ  ÿÿÿ           ÿÿÿÿÿÿ  ÿÿÿ        ÿÿ      ÿÿÿ  ÿÿÿ  ÿÿÿ        ÿÿ                    ÿÿ      ÿÿ  ÿÿ  ÿÿÿ              ÿÿÿ  ÿÿÿ        ÿÿ      ÿÿÿÿÿÿÿÿ  ÿÿÿ        ÿÿ                    ÿÿ      ÿÿ  ÿÿ  ÿÿÿ     ÿ  ÿÿÿ   ÿÿÿ  ÿÿÿ     ÿ  ÿÿ       ÿÿÿÿÿÿ   ÿÿÿ     ÿ  ÿÿ                    ÿÿ      ÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿ       ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ  ÿÿ                    ÿÿ      ÿÿ  ÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿ  ÿÿ        ÿÿÿÿ     ÿÿÿÿÿÿÿÿ  ÿÿ                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    "

def printWatermark(tile_data, xsize, ysize, buf):
    ty, tx = tile_data.shape
    data = numpy.fromstring(buf, numpy.uint8)
    data = data.reshape(ysize, xsize)
    replace = tile_data[ty-ysize:, tx-xsize:] 
    tile_data[ty-ysize:, tx-xsize:] = numpy.choose(data>0, (replace, data))
    del data

if __name__=="__main__":
    printWatermark()
	
