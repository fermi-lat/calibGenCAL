"""
Classes to represent CAL calibration XML documents.
"""


__facility__  = "Offline"
__abstract__  = "Classes to represent CAL calibration XML documents."
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/12 14:09:28 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, time
import logging
import xml.dom.minidom

import Numeric

import calXML
import calConstant
from calExcept import *


MODE_CREATE     = calXML.MODE_CREATE
MODE_READONLY   = calXML.MODE_READONLY


POSNEG = ('NEG', 'POS')

ERNG_MAP = {'LEX8' : 0, 'LEX1' : 1, 'HEX8' : 2, 'HEX1' : 3}
POSNEG_MAP = {'NEG' : 0, 'POS' : 1}



class calCalibXML(calXML.calXML):
    """
    CAL calibration XML file class.

    This class provides methods for accessing CAL calibration data stored
    in XML format.
    """
  
    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL calibration XML file

        \param fileName The XML file name.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        calXML.calXML.__init__(self, fileName, mode)  


    
class calTholdCICalibXML(calCalibXML):
    """
    CAL TholdCI calibration XML file class.

    This class provides methods for accessing CAL threshold charge
    injection calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL TholdCI calibration XML file.
        """
        
        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, dacData, adcData, pedData, lrefGain, hrefGain, tems = (0,)):
        """
        Write data to a CAL TholdCI XML file

        \param dacData A list of Numeric arrays of DAC settings data
            (16, 8, 2, 12):
            dacData[0] = ULD DAC settings data
            dacData[1] = LAC DAC settings data
            dacData[2] = FLE DAC settings data
            dacData[3] = FHE DAC settings data
        \param adcData A list of Numeric arrays of ADC/DAC characterization data.
            adcData[0] = A Numeric array of ULD ADC/DAC characterization data
                (3, 16, 8, 2, 12, 128).
            adcData[1] = A Numeric array of LAC ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[2] = A Numeric array of FLE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[3] = A Numeric array of FHE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
        \param pedData A Numeric array of pedestal value data
            (16, 9, 4, 8, 2, 12).
        \param lrefGain The LE gain value.
        \param hrefGain The HE gain value.
        \param tems A list of TEM ID's to write out.
        """

        uldDac = dacData[0]
        lacDac = dacData[1]
        fleDac = dacData[2]
        fheDac = dacData[3]

        uldAdc = adcData[0]
        lacAdc = adcData[1]
        fleAdc = adcData[2]
        fheAdc = adcData[3]

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = doc.createElement('generic')
        g.setAttribute('instrument', 'LAT')
        g.setAttribute('calibType', 'CAL_TholdCI')
        g.setAttribute('fmtVersion', 'v2r2')
        g.setAttribute('creator', '$Name:  $')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        g.setAttribute('timestamp', ts)
        r.appendChild(g)

        # insert <dimension> element  
            
        d = doc.createElement('dimension')
        d.setAttribute('nRow', '1')
        d.setAttribute('nCol', '1')
        d.setAttribute('nLayer', '8')
        d.setAttribute('nXtal', '12')
        d.setAttribute('nFace', '2')
        d.setAttribute('nRange', '4')
        d.setAttribute('exact', 'true')
        r.appendChild(d)

        for tem in tems:

            # insert <tower> element

            t = doc.createElement('tower')
            t.setAttribute('iRow', '0')
            t.setAttribute('iCol', '0')
            r.appendChild(t)
            
            for layer in range(8):

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(12):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(2):

                        # insert <face> elements

                        f = doc.createElement('face')
                        f.setAttribute('end', POSNEG[end])
                        x.appendChild(f)

                        # insert <tholdCI> element

                        tc = doc.createElement('tholdCI')
                        
                        dac = int(lacDac[tem, row, end, fe])
                        adc = lacAdc[tem, row, end, fe, dac]
                        c = doc.createComment('LAC DAC = %d' % dac)
                        tc.appendChild(c)
                        tc.setAttribute('LACVal', "%0.3f" % adc)
                        tc.setAttribute('LACSig', '1')
                        
                        dac = int(fleDac[tem, row, end, fe])
                        adc = fleAdc[tem, row, end, fe, dac]
                        c = doc.createComment('FLE DAC = %d' % dac)
                        tc.appendChild(c)
                        tc.setAttribute('FLEVal', "%0.3f" % adc)
                        tc.setAttribute('FLESig', '1')

                        dac = int(fheDac[tem, row, end, fe])
                        adc = fheAdc[tem, row, end, fe, dac]
                        c = doc.createComment('FHE DAC = %d' % dac)
                        tc.appendChild(c)
                        tc.setAttribute('FHEVal', "%0.3f" % adc)
                        tc.setAttribute('FHESig', '1')                     
                        
                        f.appendChild(tc)

                        for erng in range(3):

                            if erng < 2:
                                gain = lrefGain
                            else:
                                gain = (hrefGain - 8)

                            # insert <tholdCIRange> elements

                            tcr = doc.createElement('tholdCIRange')
                            
                            tcr.setAttribute('range', calConstant.CRNG[erng])
                            
                            dac = int(uldDac[tem, row, end, fe])
                            adc = uldAdc[erng, tem, row, end, fe, dac]
                            c = doc.createComment('ULD DAC = %d' % dac)
                            tcr.appendChild(c)
                            tcr.setAttribute('ULDVal', "%0.3f" % adc)
                            tcr.setAttribute('ULDSig', '30')

                            ped = pedData[tem, gain, erng, row, end, fe]                        
                            tcr.setAttribute('PEDVal', "%0.3f" % ped)
                            tcr.setAttribute('PEDSig', '1')
                            
                            tc.appendChild(tcr)

                        # insert fake <tholdCIRange> element for HEX1 range

                        tcr = doc.createElement('tholdCIRange')
                        
                        tcr.setAttribute('range', 'HEX1')
                        
                        tcr.setAttribute('ULDVal', '4095.000')
                        tcr.setAttribute('ULDSig', '30')
                        
                        ped = pedData[tem, (hrefGain - 8), 3, row, end, fe]
                        tcr.setAttribute('PEDVal', "%0.3f" % ped)
                        tcr.setAttribute('PEDSig', '1')
                        
                        tc.appendChild(tcr)
                    
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL TholdCI XML file

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data:
        """
        
        doc = self.getDoc()

        # find <tower> elements

        tList = doc.getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            print "tower: %d %d" % (tRow, tCol)
        
        # find <tholdCI> elements

        
        
class calIntNonlinCalibXML(calCalibXML):
    """
    CAL IntNonlin calibration XML file class.

    This class provides methods for accessing CAL int non-linearity
    calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL IntNonlin calibration XML file.
        """
        
        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, startTime, stopTime, triggers, mode, source, tems = (0,)):             
        """
        Write data to a CAL IntNonlin XML file
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = doc.createElement('generic')
        g.setAttribute('instrument', 'LAT')
        g.setAttribute('calibType', 'CAL_TholdCI')
        g.setAttribute('fmtVersion', 'v2r2')
        g.setAttribute('creator', '$Name:  $')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        g.setAttribute('timestamp', ts)
        r.appendChild(g)

        # insert <inputSample> element

        s = doc.createElement('inputSample')
        s.setAttribute('startTime', startTime)
        s.setAttribute('stopTime', stopTime)
        s.setAttribute('triggers', triggers)
        s.setAttribute('mode', mode)
        s.setAttribute('source', source)
        r.appendChild(s)

        # insert <dimension> element  
            
        d = doc.createElement('dimension')
        d.setAttribute('nRow', '1')
        d.setAttribute('nCol', '1')
        d.setAttribute('nLayer', '8')
        d.setAttribute('nXtal', '12')
        d.setAttribute('nFace', '2')
        d.setAttribute('nRange', '4')
        d.setAttribute('exact', 'true')
        r.appendChild(d)

        # insert <dac> elements

        for erng in range(4):

            dr = doc.createElement('dac')
            dr.setAttribute('range', calConstant.CRNG[erng])
            dr.setAttribute('values', '')
            r.appendChild(dr)

        for tem in tems:
            
            # insert <tower> elements

            t = doc.createElement('tower')
            t.setAttribute('iRow', '0')
            t.setAttribute('iCol', '0')
            r.appendChild(t)
            
            for layer in range(8):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(12):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(2):

                        # insert <face> elements

                        f = doc.createElement('face')
                        f.setAttribute('end', POSNEG[end])
                        x.appendChild(f)

                        for erng in range(4):

                            # insert <intNonlin> elements

                            n = doc.createElement('intNonlin')
                            n.setAttribute('range', calConstant.CRNG[erng])
                            n.setAttribute('values', '')
                            f.appendChild(n)
    
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL IntNonlin XML file

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data:
        """
        
        doc = self.getDoc()
        
        dacData = [None, None, None, None]
        adcData = [None, None, None, None]

        dataSize = [0, 0, 0, 0]        

        # find <dac> elements

        dList = doc.getElementsByTagName('dac')
        for d in dList:
            erngName = d.getAttribute('range')
            erng = ERNG_MAP[erngName]
            valueStr = d.getAttribute('values')
            valueList = valueStr.split()
            data = []
            for dac in valueList:
                data.append(int(dac))
            dacData[erng] = data
            dataSize[erng] = len(data)

        # create empty ADC data arrays

        for erng in range(4):
            size = dataSize[erng]
            data = Numeric.zeros((16, 8, 2, 12, size), Numeric.Float32)
            adcData[erng] = data

        # find <tower> elements

        tList = doc.getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = 0

            # find <layer> elements

            lList = t.getElementsByTagName('layer')
            for l in lList:

                layer = int(l.getAttribute('iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                xList = l.getElementsByTagName('xtal')
                for x in xList:

                    fe = int(x.getAttribute('iXtal'))

                    # find <face> elements

                    fList = x.getElementsByTagName('face')
                    for f in fList:

                        face = f.getAttribute('end')
                        end = POSNEG_MAP[face]

                        # find <intNonlin> elements

                        nList = f.getElementsByTagName('intNonlin')
                        for n in nList:

                            erngName = n.getAttribute('range')
                            erng = ERNG_MAP[erngName]
                            valueStr = n.getAttribute('values')
                            valueList = valueStr.split()
                            data = adcData[erng]
                            x = 0
                            for adc in valueList:
                                data[tem, row, end, fe, x] = float(adc)
                                x += 1
                            if x < dataSize[erng]:
                                for xi in range(x, dataSize[erng]):
                                    data[tem, row, end, fe, xi] = -1
                                

        return (dacData, adcData)
    

def layerToRow(layer):
    """
    Translate CAL layer number to CAL row number

    Returns: The row number (0 - 7)
    """

    row = (layer / 2)
    xy = (layer % 2)
    if xy == 1:
        row += 4
    return row


def rowToLayer(row):
    """
    Translate CAL row number to CAL layer number

    Returns: The layer number (0 - 7)
    """

    if row < 4:
        layer = (row * 2)
    else:
        layer = (((row - 4) * 2) + 1)
    return layer

    
