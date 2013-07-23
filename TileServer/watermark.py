#!/usr/bin/env python
# --*-- coding:utf-8 --*--


import numpy
from array import array

buf_xsize = 100
buf_ysize = 30
buf_tile_server = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ��  ��                                                                                  ����������  ��  ��              ��������                                                                ��          ��             ���������                                                                ��          ��             ���   ���                                                                ��      ��  ��   �������   ���         �������   �� �������    ���� �������   �� ����               ��      ��  ��  ���������  ������     ���������  ����������    ��� ���������  �������               ��      ��  ��  ���   ���  ���������  ���   ���  �����  ����  ���� ���   ���  �����                 ��      ��  ��  ���������   ��������  ���������  ��      ���  ���  ���������  ��                    ��      ��  ��  ���           ������  ���        ��      ���  ���  ���        ��                    ��      ��  ��  ���              ���  ���        ��      ��������  ���        ��                    ��      ��  ��  ���     �  ���   ���  ���     �  ��       ������   ���     �  ��                    ��      ��  ��  ���������  ���������  ���������  ��       ������   ���������  ��                    ��      ��  ��   ��������  ��������    ��������  ��        ����     ��������  ��                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    "

def printWatermark(tile_data, xsize, ysize, buf):
    ty, tx = tile_data.shape
    data = numpy.fromstring(buf, numpy.uint8)
    data = data.reshape(ysize, xsize)
    replace = tile_data[ty-ysize:, tx-xsize:] 
    tile_data[ty-ysize:, tx-xsize:] = numpy.choose(data>0, (replace, data))
    del data

if __name__=="__main__":
    printWatermark()
	
