"""
calADCSmooth
Tool to smooth CAL ADC/DAC data.
"""


__facility__  = "Offline"
__abstract__  = "Tool to smooth CAL ADC/DAC data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/03/10 18:44:06 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import os, copy
import Numeric
import logging

from calConstant import CROW, CPM




class removePoint:
    """
    ADC/DAC point for removal.

    This class provides a means of storing information about a point from the ADC/DAC
    table that should be removed as part of the filter process.
    """
    
    def __init__(self, tem, layer, end, fe, dac):
        """
        removePoint constructor.

        Param: tem - The TEM number of the data point (0-16).
        Param: layer - The layer number of the data point (0-7).
        Param: end - The end number of the data point (0-1).
        Param: fe - The channel number of the data point (0-11).
        Param: dac - The DAC value of the data point (0-127).
        """

        self.tem = tem        
        self.layer = layer
        self.end = end
        self.fe = fe
        self.dac = dac


    
class calADCSmooth:
    """
    CAL ADC/DAC data table smoothing object.
    """
    
    def __init__(self, verbose = False):
        """
        calADCSmooth constructor.
        """
        
        self.__log = logging.getLogger('calADCSmooth')
        if verbose:
            self.__log.setLevel(logging.DEBUG)


    def run(self, inData, maxSlope = 200, tems = (0,)):   
        """
        Perform the data smoothing.

        Param: inData - A Numeric array of shape (16,8,2,12,128) containing CAL ADC data.
        Param: maxSlope - The maximum segment slope allowed for outlying point filtering.
        Param: tems - A list of TEM ID's to run the smoothing on.

        Returns: A Numeric array of shape (16,8,2,12,128) containing smoothed CAL ADC data.        
        """
        
        self.__inData = inData
        self.__tmpData = copy.copy(inData)
        self.__outData = copy.copy(inData)

        self.__filter(maxSlope, tems)
        self.__smooth(tems)

        return self.__outData        
        

    def __filter(self, maxSlope, tems):

        # look for segments with slope greater than maxSlope
        
        removePoints = []

        for tem in tems:
            for layer in range(8):
                for end in range(2):

                    self.__log.debug("filter: Tower=%d Layer=%s%s", tem, CROW[layer], CPM[end])
                    
                    for fe in range(12):

                        # fine DAC settings

                        for dac in range(1, 64):

                             d0 = (dac - 1)
                             d1 = dac
                             a0 = self.__inData[tem, layer, end, fe, d0]
                             a1 = self.__inData[tem, layer, end, fe, d1]
                             da = (a1 - a0)
                             dd = (d1 - d0)
                             if dd != 0:
                                 m = (da / dd)
                             else:
                                 m = None
                             if m > maxSlope or m is None:
                                 p = removePoint(tem, layer, end, fe, dac)
                                 removePoints.append(p)
                                 self.__log.debug('filter: twr=%d,layer=%s%s,fe=%d, removing seg %s (%d,%d)-(%d,%d)', \
                                     tem, CROW[layer], CPM[end], fe, str(m), d0, a0, d1, a1)

                        # coarse DAC settings

                        for dac in range(65, 128):

                             d0 = (dac - 1)
                             d1 = dac
                             a0 = self.__inData[tem, layer, end, fe, d0]
                             a1 = self.__inData[tem, layer, end, fe, d1]
                             da = (a1 - a0)
                             dd = (d1 - d0)
                             if dd != 0:
                                 m = (da / dd)
                             else:
                                 m = None
                             if m > maxSlope or m is None:
                                 p = removePoint(tem, layer, end, fe, dac)
                                 removePoints.append(p)
                                 self.__log.debug('filter: twr=%d,layer=%s%s,fe=%d, removing seg %s (%d,%d)-(%d,%d)', \
                                    tem, CROW[layer], CPM[end], fe, str(m), d0, a0, d1, a1)                             

        # replace outlying points with linear interpolation

        for p in removePoints:

            a0 = self.__inData[p.tem, p.layer, p.end, p.fe, (p.dac - 1)]
            a1 = self.__inData[p.tem, p.layer, p.end, p.fe, (p.dac + 1)]
            a = (a0 + a1) / 2
            self.__tmpData[p.tem, p.layer, p.end, p.fe, p.dac] = a
            self.__log.debug('filter: twr=%d,layer=%s%s,fe=%d, new point (%d,%d)', p.tem, CROW[p.layer], CPM[p.end], \
                p.fe, p.dac, a)
            

    def __mean(self, points):

        N = len(points)
        M = 0
        for p in points:
            M = (M + p)
        return (M / N)    
            
                

    def __smooth(self, tems):

        # set ADC values less than zero to zero

        self.__tmpData = Numeric.clip(self.__tmpData, 0, 100000)        

        # run smoothing filter on data        

        for tem in tems:
            for layer in range(8):
                for end in range(2):

                    self.__log.debug("smooth: Tower=%d Layer=%s%s", tem, CROW[layer], CPM[end])

                    for fe in range(12):

                        # find pedestal for fine DAC settings

                        pedDac = 0            
                        for dac in range(0, 64):
                            adc = self.__tmpData[tem, layer, end, fe, dac]
                            if adc > 0:
                                pedDac = (dac + 4)
                                break

                        # filter fine DAC settings
                        
                        for dac in range(tem, 64):

                            a = []
                            adc = self.__tmpData[tem, layer, end, fe, dac]
                            a.append(adc)
                            
                            if dac > 0:
                                adc = self.__tmpData[tem, layer, end, fe, (dac - 1)]
                                a.append(adc)
                            if dac < 63:
                                adc = self.__tmpData[tem, layer, end, fe, (dac + 1)]
                                a.append(adc)   
                            if dac > 1 and dac > pedDac:
                                adc = self.__tmpData[tem, layer, end, fe, (dac - 2)]
                                a.append(adc)
                            if dac < 62 and dac > pedDac:
                                adc = self.__tmpData[tem, layer, end, fe, (dac + 2)]
                                a.append(adc)
                                                
                            adc = self.__mean(a)
                            self.__outData[tem,layer,end,fe,dac] = adc


                        # find pedestal for fine DAC settings

                        pedDac = 0            
                        for dac in range(64, 128):
                            adc = self.__tmpData[tem, layer, end, fe, dac]
                            if adc > 0:
                                pedDac = (dac + 4)
                                break

                        # coarse DAC settings
                        
                        for dac in range(64, 128):

                            a = []
                            adc = self.__tmpData[tem, layer, end, fe, dac]
                            a.append(adc)
                            
                            if dac > 64:
                                adc = self.__tmpData[tem, layer, end, fe, (dac - 1)]
                                a.append(adc)
                            if dac < 127:
                                adc = self.__tmpData[tem, layer, end, fe, (dac + 1)]
                                a.append(adc)   
                            if dac > 65 and dac > pedDac:
                                adc = self.__tmpData[tem, layer, end, fe, (dac - 2)]
                                a.append(adc)
                            if dac < 126 and dac > pedDac:
                                adc = self.__tmpData[tem, layer, end, fe, (dac + 2)]
                                a.append(adc)

                            adc = self.__mean(a)
                            self.__outData[tem,layer,end,fe,dac] = adc                    

                        

    