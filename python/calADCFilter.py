"""
Tool to smooth CAL ADC/DAC data.
"""


__facility__  = "Offline"
__abstract__  = "Tool to smooth CAL ADC/DAC data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/01/26 23:20:27 $"
__version__   = "$Revision: 1.14 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import logging

import Numeric

import calConstant



DAC_TYPE_FLE = 0
DAC_TYPE_FHE = 1
DAC_TYPE_LAC = 3
DAC_TYPE_ULD = 4


    
class calADCFilter:
    """
    CAL ADC/DAC data table filter/smoothing object.
    """
    
    def __init__(self, type, smoothing = False, verbose = False):
        """
        calADCSmooth constructor.

        Param: type - file type:
            DAC_TYPE_DAC = FLE, FHE, or LAC file
            DAC_TYPE_ULD = ULD file
        Param: smoothing - run boxcar digital filter
        Param: verbose - print debug info
        """
        
        self.__log = logging.getLogger('calADCFilter')
        if verbose:
            self.__log.setLevel(logging.DEBUG)

        if type != DAC_TYPE_FLE and type != DAC_TYPE_FHE and type != DAC_TYPE_LAC \
           and type != DAC_TYPE_ULD:
            raise TypeError, "type %d not supported" % type
        
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

        if self.__type == DAC_TYPE_FLE or self.__type == DAC_TYPE_FHE \
           or self.__type == DAC_TYPE_LAC:
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

                        fineData = outData[tem,row,end,fe,0:64]
                        coarseData = outData[tem,row,end,fe,64:128]
                        
                        self.__floor(fineData)
                        self.__floor(coarseData)

                        fillLAC = False
                        if self.__type == DAC_TYPE_LAC:
                            z = Numeric.nonzero(fineData)
                            if len(z) < 2:
                                fillLAC = True                        

                        if not fillLAC:
                            self.__extrapolateLow(fineData)
                        else:
                            self.__fillLAC(fineData)
                        self.__extrapolateLow(coarseData)

                        if not fillLAC:
                            self.__restore(fineData)
                        self.__restore(coarseData)                    

                        if not fillLAC:
                            self.__filter(fineData)
                        self.__filter(coarseData)

                        if self.__type == DAC_TYPE_FLE:
                            self.__extrapolateHigh(coarseData)
                    
                        if self.__smoothing:
                            self.__smooth(fineData)
                            self.__smooth(coarseData)                            


    def __runULD(self, outData, tems):

        for tem in tems:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                        for erng in range(3):
                            self.__log.debug("T%d,%s%s,%d,%s" %
                                             (tem, calConstant.CROW[row], calConstant.CPM[end],
                                              fe, calConstant.CRNG[erng]))

                            fineData = outData[erng,tem,row,end,fe,0:64]
                            coarseData = outData[erng,tem,row,end,fe,64:128]

                            self.__floor(fineData)
                            self.__floor(coarseData)

                            self.__restore(fineData)
                            self.__restore(coarseData)                    

                            self.__filter(fineData)
                            self.__filter(coarseData)

                            if self.__smoothing:
                                self.__smooth(fineData)
                                self.__smooth(coarseData)                            


        
    def __floor(self, data):
        """
        Look for outlying data in pedestal zero data.  The characteristic is point before = 0
        and the point after is less.  Only the first point meeting these criteria is removed.
        Also check for zero response in the last data point.
        Next the pedestal floor is scanned for and set to 0.
        """     

        # fix zero response in last data point

        if data[-1] == 0.0:
            dac = 63
            while data[dac] == 0.0:
                dac -= 1
            data[-1] = data[dac]
            self.__log.debug('floor: replacing (63,%f)', data[dac])

        # look for outlying noise point in pedestal range, set to 0

        for dac in range(0, 64):
            if dac == 0:
                a0 = 0.0
            else:
                a0 = data[dac - 1]
            a1 = data[dac]

            for d in range(dac + 1, 64):
                a2 = data[d]
                if a2 > 0.0:
                    break
                              
            if a1 != 0.0 and a0 == 0.0 and a1 > a2:
                data[dac] = 0.0
                self.__log.debug('floor: replacing (%d,%f)', dac, a1)
                break

        # zero out pedestal range by looking for segments with nil slope

        for dac in range(0, 64):
            if data[dac] == 0.0:
                continue
            d0 = dac
            for dac in range(d0 + 1, 64):
                if data[dac] == 0.0:
                    continue
                d1 = dac
                a0 = data[d0]
                a1 = data[d1]
                m = (a1 - a0) / (d1 - d0)
                if m < 20.0 and (a0 < 30.0 or a1 < 100.0):
                    data[d0] = 0.0
                    self.__log.debug('floor: replacing %d,%d,%f', d0, d1, m)
                    break
                    


    def __filter(self, data):
        """
        Look for segments with very large slopes.  Replace them with linear interpolation.
        """

        # find excessive segement slopes
            
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



    def __restore(self, data):
        """
        Replace zero points with interpolation when the points occur in groups.  The last
        good point before and the first good point after the group are used as the
        interpolation end points.
        """

        # find first non-zero point

        for d in range(0, 64):
            if data[d] > 0.0:
                da = d
                break

        # start search for strings of zero data

        dList = []
        aa = data[da]
        for dx in range(da, 64):
            
            if data[dx] == 0:
                
                if len(dList) == 0:

                    # start new list
                                        
                    if dx != 0:
                        aa = data[dx - 1]
                        da = (dx - 1)
                    else:
                        aa = data[dx]
                        da = dx

                # add point to segement list
                        
                dList.append(dx)
                continue
            
            else:
                
                if len(dList) > 0:

                    # end of list
                    # go back and interpolate points of gathered segements
                    
                    ax = data[dx]
                    m = (ax - aa) / (dx - da)
                    self.__log.debug("interp  (%d,%f):(%d,%f):%d" % (da, aa, dx, ax, m))
                    for d in dList:
                        ax = (aa + m)
                        data[d] = ax
                        aa = ax
                        self.__log.debug("new point (%d,%f)" % (d, ax))
                        
                    # reset segment list
                        
                    dList = []
                    da = d + 1



    def __extrapolateLow(self, data):
        """
        Extrapolate ADC values beyond noise range.

        Param: data - A Numeric array containing one range of ADC data.  The first good
                      segment after pedestal noise is used to extrapolate the remaining
                      values in the range. The data is changed in place.              
        """
        
        # find first two good data points

        for d in range(0, 64):
            a = data[d]
            if a > 0:
                d0 = d
                break

        d1 = None
        for d in range((d0 + 1), 64):
            a = data[d]
            if a > 0:
                d1 = d
                break
        if d1 is None:
            return
            
        # get slope of first good segment

        a0 = data[d0]
        a1 = data[d1]
        m = (a1 - a0) / (d1 - d0)

        self.__log.debug('extrapolate: %d %f', d0, m)        

        # extrapolate preceding values

        for d in range((d0 - 1), -1, -1):

            a = a0 - m
            if a < 0:
                a = 0
            data[d] = a
            a0 = a
                


    def __extrapolateHigh(self, data):
        """
        Extrapolate FLE ADC values beyond testing range.

        Param: data - A Numeric array containing one range of ADC data.  The last good
        segment before saturation is used to extrapolate the remaining values in the range.
        The data is changed in place.
        """
                    
        # find data before saturation

        dac = 63
        sat = adc = data[dac]
        while adc == sat:
            dac -= 1
            adc = data[dac]
        dac -= 10
        
        # get slope of last good segment
                    
        a0 = data[dac - 1]
        a1 = data[dac]
        m = (a1 - a0)

        self.__log.debug('extrapolate: %d %f', dac, m)

        # extrapolate remaining values

        for d in range((dac + 1), 64):
            data[d] = a1 + m
            a1 = data[d]



    def __fillLAC(self, data):
        """
        Extrapolate LAC values from very sparse data (single point at DAC 63).

        Param: data - A Numeric array containing one range of ADC data.  The last good
                      point is used to extrapolate the remaining values in the range.
                      The data is changed in place.              
        """
            
        m = 11.0
        a0 = data[63]
        self.__log.debug('LAC fill: %d %f', a0, m)
        
        # extrapolate preceding values

        for d in range(62, -1, -1):

            a = a0 - m
            if a < 0:
                a = 0
            data[d] = a
            a0 = a
            
                

    def __smooth(self, data):        
        """
        Run 5-point boxcar digital filter on data.
        """
                        
        for dac in range(2, 62):

            adc = data[dac]
            if adc == 0.0:
                continue

            a = data[dac - 2 : dac + 3]            
                                                
            adc = Numeric.average(a)
            data[dac] = adc

    
                        

    