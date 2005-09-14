"""
Tool to smooth CAL ADC/DAC data.
"""


__facility__  = "Offline"
__abstract__  = "Tool to smooth CAL ADC/DAC data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/09/14 13:48:56 $"
__version__   = "$Revision: 1.1 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import os, copy

import logging

import Numeric

import calConstant



FILE_TYPE_DAC = 0
FILE_TYPE_ULD = 1


    
class calADCFilter:
    """
    CAL ADC/DAC data table filter/smoothing object.
    """
    
    def __init__(self, type, smoothing = False, verbose = False):
        """
        calADCSmooth constructor.

        Param: type - file type:
            FILE_TYPE_DAC = FLE, FHE, or LAC file
            FILE_TYPE_ULD = ULD file
        Param: smoothing - run boxcar digital filter
        Param: verbose - print debug info
        """
        
        self.__log = logging.getLogger('calADCFilter')
        if verbose:
            self.__log.setLevel(logging.DEBUG)

        if type != FILE_TYPE_DAC and type != FILE_TYPE_ULD:
            self.__log.error("type %d not supported", type)
        self.__smoothing = smoothing
        self.__type = type


    def run(self, inData, tems = (0,)):   
        """
        Perform the data smoothing.

        Param: inData - A Numeric array of shape (16,8,2,12,128) or (3,16,8,2,12,128)
                        containing CAL ADC data.
        Param: tems - A list of TEM ID's to run the smoothing on.

        Returns: A Numeric array of shape (16,8,2,12,128) or (3,16,8,2,12,128)
                 containing smoothed CAL ADC data.        
        """
        
        outData = Numeric.clip(inData, 0, 4095)

        if self.__type == FILE_TYPE_DAC:
            self.__runDAC(outData, tems)
        else:
            self.__runULD(outData, tems)

        return outData            


    def __runDAC(self, outData, tems):

        for tem in tems:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):

                        self.__log.debug("T%d,%s%s,%d" %
                                         (tem, calConstant.CROW[row], calConstant.CPM[end], fe))

                        self.__floor(outData[tem,row,end,fe,0:64])
                        self.__floor(outData[tem,row,end,fe,64:128])                    

                        self.__filter(outData[tem,row,end,fe,0:64])
                        self.__filter(outData[tem,row,end,fe,64:128])

                        if self.__smoothing:
                            self.__smooth(outData[tem,row,end,fe,0:64])
                            self.__smooth(outData[tem,row,end,fe,64:128])


    def __runULD(self, outData, tems):

        for tem in tems:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                        for erng in range(3):
                            self.__log.debug("T%d,%s%s,%d,%s" %
                                             (tem, calConstant.CROW[row], calConstant.CPM[end],
                                              fe, calConstant.CRNG[erng]))

                            self.__floor(outData[erng,tem,row,end,fe,0:64])
                            self.__floor(outData[erng,tem,row,end,fe,64:128])                    

                            self.__filter(outData[erng,tem,row,end,fe,0:64])
                            self.__filter(outData[erng,tem,row,end,fe,64:128])

                            if self.__smoothing:
                                self.__smooth(outData[erng,tem,row,end,fe,0:64])
                                self.__smooth(outData[erng,tem,row,end,fe,64:128])                            
        

    def __filter(self, data):

        # look for segments with slope greater than maxSlope
            
        dList = []

        for dac in range(1, 64):

            d0 = (dac - 1)
            d1 = dac
            a0 = data[d0]
            a1 = data[d1]
            da = (a1 - a0)
            dd = (d1 - d0)
            if dd != 0:
                m = (da / dd)
            else:
                m = None
            if m > 200 or m is None:
                dList.append(dac)
                self.__log.debug('removing seg %s (%d,%d)-(%d,%d)' % (str(m), d0, a0, d1, a1))

        # replace outlying points with linear interpolation

        for dac in dList:
            a0 = data[dac - 1]
            if dac == 63:
                a = a0
            else:
                a1 = data[dac + 1]
                a = (a0 + a1) / 2
            data[dac] = a
            self.__log.debug('new point (%d,%d)' % (dac, a))


    def __floor(self, data):             

        # replace zero points with interpolation

        dList = []
        da = 0
        aa = data[0]
        for dx in range(0, 64):
            if data[dx] == 0:
                if len(dList) == 0:
                    if dx != 0:
                        aa = data[dx - 1]
                        da = (dx - 1)
                    else:
                        aa = data[dx]
                        da = dx
                dList.append(dx)
                continue
            else:
                if len(dList) > 0:
                    ax = data[dx]
                    m = (ax - aa) / (dx - da)
                    self.__log.debug("interp  (%d,%f):(%d,%f):%d" % (da, aa, dx, ax, m))
                    for d in dList:
                        ax = (aa + m)
                        data[d] = ax
                        aa = ax
                        self.__log.debug("new point (%d,%f)" % (d, ax))
                    dList = []
                    da = d + 1   
            

    def __mean(self, points):

        N = len(points)
        M = 0
        for p in points:
            M = (M + p)
        return (M / N)    
            
                

    def __smooth(self, data):        

        # run smoothing filter on data        
                        
        for dac in range(0, 64):

            a = []
            adc = data[dac]
            a.append(adc)
                            
            if dac > 0:
                adc = data[dac - 1]
                a.append(adc)
            if dac < 63:
                adc = data[dac + 1]
                a.append(adc)   
            if dac > 1:
                adc = data[dac - 2]
                a.append(adc)
            if dac < 62:
                adc = data[dac + 2]
                a.append(adc)
                                                
            adc = self.__mean(a)
            data[dac] = adc

                 

                        

    