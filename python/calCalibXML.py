"""
Classes to represent CAL calibration XML documents.
"""


__facility__  = "Offline"
__abstract__  = "Classes to represent CAL calibration XML documents."
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/02/06 17:22:17 $"
__version__   = "$Revision: 1.37 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys
import time

import Numeric

import calXML
import calConstant
from calExcept import *


MODE_CREATE     = calXML.MODE_CREATE
MODE_READONLY   = calXML.MODE_READONLY


POSNEG = ('NEG', 'POS')

ERNG_MAP = {'LEX8' : 0, 'LEX1' : 1, 'HEX8' : 2, 'HEX1' : 3}
POSNEG_MAP = {'NEG' : 0, 'POS' : 1}


INTNONLIN_MAX_DATA = 256


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

        calXML.calXML.__init__(self, fileName, mode, validating = True)


    def info(self):
        """
        Return ancillary information from a CAL calibration XML file.

        Returns: A dictionary with the following keys and values:
            'instrument' - from <generic> element
            'timestamp' - from <generic> element
            'calibType' - from <generic> element
            'fmtVersion' - from <generic> element
            'DTDVersion' - from <generic> element
        """
        
        i = {}

        # get <generic> attribute values        
            
        self.genericInfo(i)

        return i             


    def genericWrite(self, type):
        """
        Creates the <generic> element for a calibration XML file

        Returns: A reference to the <generic> Element object.        
        """

        g = self.getDoc().createElement('generic')
        g.setAttribute('instrument', 'LAT')
        g.setAttribute('calibType', str(type))
        g.setAttribute('fmtVersion', 'v2r3')
        g.setAttribute('creator', 'myname')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        g.setAttribute('timestamp', ts)

        return g


    def dimensionWrite(self, nFace = 2, nRange = 4, nDacCol = 0, nXpos = 0):
        """
        Creates the <dimension> element for a calibration XML file

        Param: nFace The value for the 'nFace' attribute.
        Param: nRange The value for the 'nRange' attribute.
        Param: nDacCol The value for the 'nDacCol' attribute.
        Param: nXpos: The value for the 'nXpos' attribute.
        
        Returns: A reference to the <dimension> Element object.
        """

        d = self.getDoc().createElement('dimension')
        d.setAttribute('nRow', '4')
        d.setAttribute('nCol', '4')
        d.setAttribute('nLayer', '8')
        d.setAttribute('nXtal', '12')
        d.setAttribute('nFace', str(nFace))
        d.setAttribute('nRange', str(nRange))
        d.setAttribute('exact', 'false')
        d.setAttribute('nDacCol', str(nDacCol))
        d.setAttribute('nXpos', str(nXpos))

        return d


    def getType(self):
        """
        Get CAL calibration XML file type.

        Returns: The <generic> calibType attribute value.        
        """

        # find <generic> element

        gList = self.getDoc().getElementsByTagName('generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        value = g.getAttribute('calibType')
        if len(value) == 0:
            raise calFileReadExcept, "calibType attribute not found for <generic> element"
        
        return str(value)


    def genericInfo(self, infoDict):
        """
        Fill in the generic portion of an XML info dictionary.

        Param: infoDict - The dictionary to fill in

        Returns: The XML doc node for the <generic> element        
        """

        # find <generic> element

        gList = self.getDoc().getElementsByTagName('generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        # fill in dictionary        

        value = g.getAttribute('instrument')
        if len(value) == 0:
            infoDict['instrument'] = None
        else:
            infoDict['instrument'] = str(value)

        value = g.getAttribute('calibType')
        if len(value) == 0:
            infoDict['calibType'] = None
        else:
            infoDict['calibType'] = str(value)

        value = g.getAttribute('timestamp')
        if len(value) == 0:
            infoDict['timestamp'] = None
        else:
            infoDict['timestamp'] = str(value)

        value = g.getAttribute('fmtVersion')
        if len(value) == 0:
            infoDict['fmtVersion'] = None
        else:
            infoDict['fmtVersion'] = str(value)

        value = g.getAttribute('DTDVersion')
        if len(value) == 0:
            infoDict['DTDVersion'] = None
        else:
            infoDict['DTDVersion'] = str(value)

        return g


    def getTowers(self):
        """
        Get the ID's of towers contributing to the data file.

        Returns: A list of tower ID's.        
        """

        towers = []        

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)
            towers.append(tem)

        return towers
    

    
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
        

    def write(self, dacData, adcData, intNonlinData, intNonlinLength, pedData,
              lrefGain, hrefGain, biasData, tems = (0,)):
        """
        Write data to a CAL TholdCI XML file

        Param: dacData - A list of Numeric arrays of DAC settings data
            (16, 8, 2, 12):
            dacData[0] = ULD DAC settings data
            dacData[1] = LAC DAC settings data
            dacData[2] = FLE DAC settings data
            dacData[3] = FHE DAC settings data
        Param: adcData - A list of Numeric arrays of ADC/DAC characterization data.
            adcData[0] = A Numeric array of ULD ADC/DAC characterization data
                (3, 16, 8, 2, 12, 128).
            adcData[1] = A Numeric array of LAC ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[2] = A Numeric array of FLE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[3] = A Numeric array of FHE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
        Param: intNonlinData - A Numeric array of ADC non-linearity characterization
            data for HEX1 energy range (16, 8, 2, 12, <N>).
        Param: intNonlinLength = A Numeric array of ADC intnonlin data lengths for
            HEX1 energy range (16, 8, 2, 12, 1)
        Param: pedData - A Numeric array of pedestal value data
            (16, 9, 4, 8, 2, 12).
        Param: lrefGain A Numeric array of LE gain index settings data
            (16, 8, 2, 12)
        Param: hrefGain A Numeric array of HE gain index settings data
            (16, 8, 2, 12).
        Param: biasData A Numeric array of bias correction values
            (16, 8, 2, 12, 2)
        Param: tems A list of TEM ID's to write out.
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

        g = self.genericWrite('CAL_TholdCI')
        r.appendChild(g)

        # insert <dimension> element  

        d = self.dimensionWrite()            
        r.appendChild(d)

        for tem in tems:

            # insert <tower> element

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

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
                        adc += biasData[tem, row, end, fe, 0]
                        c = doc.createComment('FLE DAC = %d' % dac)
                        tc.appendChild(c)
                        tc.setAttribute('FLEVal', "%0.3f" % adc)
                        tc.setAttribute('FLESig', '1')

                        dac = int(fheDac[tem, row, end, fe])
                        adc = fheAdc[tem, row, end, fe, dac]
                        adc += biasData[tem, row, end, fe, 0]
                        c = doc.createComment('FHE DAC = %d' % dac)
                        tc.appendChild(c)
                        tc.setAttribute('FHEVal', "%0.3f" % adc)
                        tc.setAttribute('FHESig', '1')

                        dac = int(uldDac[tem, row, end, fe])    
                        c = doc.createComment('ULD DAC = %d' % dac)
                        tc.appendChild(c)

                        c = doc.createComment('LE gain = %d' % lrefGain[tem, row, end, fe])
                        tc.appendChild(c)
                        c = doc.createComment('HE gain = %d' % hrefGain[tem, row, end, fe])
                        tc.appendChild(c)

                        c = doc.createComment('LE bias = %0.3f' % biasData[tem, row, end, fe, 0])
                        tc.appendChild(c)
                        c = doc.createComment('HE bias = %0.3f' % biasData[tem, row, end, fe, 1])
                        tc.appendChild(c)
                        
                        f.appendChild(tc)
                        
                        for erng in range(3):

                            # get gain setting for channel

                            if erng < calConstant.CRNG_HEX8:
                                gain = lrefGain[tem, row, end, fe]
                            else:
                                gain = (hrefGain[tem, row, end, fe] - 8)
                                if gain < 0:
                                    gain = 8
                            gain = int(gain)
                       
                            # insert <tholdCIRange> elements

                            tcr = doc.createElement('tholdCIRange')
                            
                            tcr.setAttribute('range', calConstant.CRNG[erng])
                            
                            dac = int(uldDac[tem, row, end, fe])
                            adc = uldAdc[erng, tem, row, end, fe, dac]
                
                            tcr.setAttribute('ULDVal', "%0.3f" % adc)
                            tcr.setAttribute('ULDSig', '30')

                            ped = pedData[tem, gain, erng, row, end, fe]                        
                            tcr.setAttribute('pedVal', "%0.3f" % ped)
                            tcr.setAttribute('pedSig', '1')                            
                            
                            tc.appendChild(tcr)

                        # insert <tholdCIRange> element for HEX1 range
                        # use top element from intNonlin ADC value list

                        tcr = doc.createElement('tholdCIRange')
                        
                        tcr.setAttribute('range', 'HEX1')

                        size = int(intNonlinLength[tem, row, end, fe, 0])
                        adc = intNonlinData[tem, row, end, fe, (size - 1)]
                        tcr.setAttribute('ULDVal', '%0.3f' % adc)
                        tcr.setAttribute('ULDSig', '30')

                        gain = hrefGain[tem, row, end, fe] - 8
                        if gain < 0:
                            gain = 8
                        gain = int(gain)
                        ped = pedData[tem, gain, calConstant.CRNG_HEX1, row, end, fe]
                        tcr.setAttribute('pedVal', "%0.3f" % ped)
                        tcr.setAttribute('pedSig', '1')
                        
                        tc.appendChild(tcr)
                    
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL TholdCI XML file

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data (adcData, uldData, pedData):
            adcData - A Numeric array of shape (16, 8, 2, 12, 3).  The last
            dimension holds the LAC, FLE, and FHE values for the channel:
                [:, 0] = LAC ADC threshold values
                [:, 1] = FLE ADC threshold values
                [:, 2] = FHE ADC threshold values
            uldData - A Numeric array of shape (16, 8, 2, 12, 4).  The last
            dimension holds the ULD values for each energy range:
                [:, 0] = LEX8 ULD ADC threshold values
                [:, 1] = LEX1 ULD ADC threshold values
                [:, 2] = HEX8 ULD ADC threshold values
                [:, 3] = HEX1 ULD ADC threshold values
            pedData - A Numeric array of shape (16, 8, 2, 12, 4).  The last
            dimension holds the pedestal values for each energy range:
                [:, 0] = LEX8 ULD pedestal values
                [:, 1] = LEX1 ULD pedestal values
                [:, 2] = HEX8 ULD pedestal values
                [:, 3] = HEX1 ULD pedestal values      
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_TholdCI':
            raise calFileReadExcept, "XML file is type %s (expected CAL_TholdCI)" % type

        # create empty data arrays        
        
        adcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 3), Numeric.Float32)
        uldData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 4), Numeric.Float32)
        pedData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 4), Numeric.Float32)

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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
        
                        # find <tholdCI> elements

                        ciList = f.getElementsByTagName('tholdCI')
                        for ci in ciList:

                            adc = ci.getAttribute('LACVal')
                            adcData[tem, row, end, fe, 0] = float(adc)
                            adc = ci.getAttribute('FLEVal')
                            adcData[tem, row, end, fe, 1] = float(adc)
                            adc = ci.getAttribute('FHEVal')
                            adcData[tem, row, end, fe, 2] = float(adc)

                            # find <tholdCIRange> elements

                            cirList = ci.getElementsByTagName('tholdCIRange')
                            for cir in cirList:
                                erngName = cir.getAttribute('range')
                                erng = ERNG_MAP[erngName]
                                adc = cir.getAttribute('ULDVal')
                                uldData[tem, row, end, fe, erng] = float(adc)
                                adc = cir.getAttribute('pedVal')
                                pedData[tem, row, end, fe, erng] = float(adc)

        return (adcData, uldData, pedData)   

        
        
class calIntNonlinCalibXML(calCalibXML):
    """
    CAL IntNonlin calibration XML file class.

    This class provides methods for accessing CAL int non-linearity
    calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL IntNonlin calibration XML file.

        Param: fileName - The name of the XML file.
        Param: mode - The XML file access mode (MODE_CREATE or MODE_READONLY).
        """
        
        calCalibXML.__init__(self, fileName, mode)


    def write(self, lengthData, dacData, adcData, inputSample = None, tems = (0,)):
        """
        Write data to a CAL IntNonlin XML file

        Param: lengthData - A list of 4 elements, each a reference to a Numeric
                            array of DAC values. The shape of each array is
                            (16, 8, 2, 12, 1), where the last dimension holds
                            the length of the ADC and DAC value lists for that
                            channel.
        Param: dacData - A list of 4 elements, each a reference to a Numeric
                         array of DAC values. The shape of each array is
                         (16, 8, 2, 12, <size>), where <size> is the length of
                         the corresponding dacData array for that energy range.
        Param: adcData - A list of 4 elements, each a reference to a Numeric
                         array of ADC values. The shape of each array is
                         (16, 8, 2, 12, <size>), where <size> is the length of
                         the corresponding dacData array for that energy range.
        Param: inputSample - A dictionary of <inputSample> values:
            'startTime' - The <inputSample> 'startTime' attribute value.
            'stopTime' - The <inputSample> 'stopTime' attribute value.
            'trigger' - The <inputSample> 'trigger' attribute value.
            'mode' - The <inputSample> 'mode' attribute value.
            'source' - The <inputSample> 'source' attribute value.
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_IntNonlin')
        r.appendChild(g)

        # insert <inputSample> element

        if inputSample is not None:
            s = doc.createElement('inputSample')
            if startTime is not None:
                s.setAttribute('startTime', inputSample['startTime'])
            if stopTime is not None:
                s.setAttribute('stopTime', inputSample['stopTime'])
            if triggers is not None:
                s.setAttribute('triggers', inputSample['triggers'])
            if mode is not None:
                s.setAttribute('mode', inputSample['mode'])
            if source is not None:
                s.setAttribute('source', inputSample['source'])
            g.appendChild(s)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4)
        r.appendChild(d)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        # insert <face> elements

                        f = doc.createElement('face')
                        f.setAttribute('end', POSNEG[end])
                        x.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            # insert <intNonlin> elements

                            n = doc.createElement('intNonlin')
                            n.setAttribute('range', calConstant.CRNG[erng])

                            # get length of data lists for this channel

                            length = lengthData[erng]                            

                            # insert 'values' attribute holding ADC data
                            
                            s = ''
                            data = adcData[erng]
                            for xi in range(length[tem, row, end, fe, 0]): 
                                adc = data[tem, row, end, fe, xi]               
                                s += '%0.2f ' % float(adc)
                            n.setAttribute('values', s.rstrip(' '))

                            # insert 'sdacs' attribute holding DAC data
                            
                            s = ''
                            data = dacData[erng]
                            for xi in range(length[tem, row, end, fe, 0]): 
                                dac = data[tem, row, end, fe, xi]               
                                s += '%0.2f ' % float(dac)
                            n.setAttribute('sdacs', s.rstrip(' '))                            
                            
                            n.setAttribute('error', '0.10')
                            f.appendChild(n)
    
        # write output XML file

        self.writeFile()
        

    def __write_v2r2(self, dacData, adcData, inputSample = None, tems = (0,)):             
        """
        Write data to a CAL IntNonlin XML file, version v2r2

        Param: dacData - A list of DAC values. The length of this array is the
                         number of data points for that energy range.
        Param: adcData - A list of 4 elements, each a reference to a Numeric
                         array of ADC values. The shape of each array is
                         (16, 8, 2, 12, <size>), where <size> is the length of
                         the corresponding dacData array for that energy range.
                         If the ADC data in the array has less points than the
                         correspoding DAC values array, the extra ADC values at
                         the end should be set to '-1'.  This will prevent them
                         from being written to the XML file.
        Param: inputSample - A dictionary of <inputSample> values:
            'startTime' - The <inputSample> 'startTime' attribute value.
            'stopTime' - The <inputSample> 'stopTime' attribute value.
            'trigger' - The <inputSample> 'trigger' attribute value.
            'mode' - The <inputSample> 'mode' attribute value.
            'source' - The <inputSample> 'source' attribute value.
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_IntNonlin')
        r.appendChild(g)

        # insert <inputSample> element

        if inputSample is not None:
            s = doc.createElement('inputSample')
            if startTime is not None:
                s.setAttribute('startTime', inputSample['startTime'])
            if stopTime is not None:
                s.setAttribute('stopTime', inputSample['stopTime'])
            if triggers is not None:
                s.setAttribute('triggers', inputSample['triggers'])
            if mode is not None:
                s.setAttribute('mode', inputSample['mode'])
            if source is not None:
                s.setAttribute('source', inputSample['source'])
            g.appendChild(s)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4, nDacCol = 4)
        r.appendChild(d)

        # insert <dac> elements

        for erng in range(calConstant.NUM_RNG):

            dr = doc.createElement('dac')
            dr.setAttribute('range', calConstant.CRNG[erng])
            s = ''
            data = dacData[erng]
            for dac in data:
                s += '%s ' % str(dac)
            dr.setAttribute('values', s.rstrip(' '))
            dr.setAttribute('error', '0.0')
            r.appendChild(dr)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        # insert <face> elements

                        f = doc.createElement('face')
                        f.setAttribute('end', POSNEG[end])
                        x.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            # insert <intNonlin> elements

                            n = doc.createElement('intNonlin')
                            n.setAttribute('range', calConstant.CRNG[erng])
                            s = ''
                            data = adcData[erng]
                            for adc in data[tem, row, end, fe, :]:
                                if adc < 0:
                                    continue
                                s += '%0.2f ' % float(adc)
                            n.setAttribute('values', s.rstrip(' '))
                            n.setAttribute('error', '0.00')
                            f.appendChild(n)
    
        # write output XML file

        self.writeFile()
        

    def read(self):
        """
        Read data from a CAL IntNonlin XML file

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data (lengthData, dacData, adcData):
            lengthData - A list of 4 elements, each a reference to a Numeric
                         array of length values.  The shape of each array
                         is (16, 8, 2, 12, 1), where the last dimension
                         contains the length of the DAC list for that
                         channel.  This value may be used to determine the
                         number of valid data values in the following
                         dacData and adcData arrays.
            dacData -   A list of 4 elements, each a reference to a Numeric
                        array of DAC values. The shape of each array is
                        (16, 8, 2, 12, 256)  The last dimension contains the
                        DAC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
            adcData -   A list of 4 elements, each a reference to a Numeric
                        array of ADC values. The shape of each array is
                        (16, 8, 2, 12, 256).  The last dimension contains the
                        ADC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_IntNonlin':
            raise calFileReadExcept, "XML file is type %s (expected CAL_IntNonlin)" % type        

        # find <generic> element

        gList = self.getDoc().getElementsByTagName('generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        # get format version

        value = g.getAttribute('fmtVersion')
        if len(value) == 0:
            raise calFileReadExcept, "cannot find fmtVersion attribute of <generic> element"
        version = str(value)

        if version == 'v2r2':

            # copy old format data into new format arrays
            
            (dacDataOld, adcData) = self.__read_v2r2()

            lengthData = [None, None, None, None]
            dacData = [None, None, None, None]

            for erng in range(calConstant.NUM_RNG):
                
                size = len(dacDataOld[erng])
                dacData[erng] = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, size), Numeric.Float32)
                lengthData[erng] = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, 1), Numeric.Int16)

                length = lengthData[erng]
                length[...] = size
                
                for n in range(size):
                    
                    data = dacData[erng]
                    oldData = dacDataOld[erng]
                    data[...,n] = oldData[n]

                # fixup lengths for short ADC lists

                for tem in range(calConstant.NUM_TEM):
                    for row in range(calConstant.NUM_ROW):
                        for end in range(calConstant.NUM_END):
                            for fe in range(calConstant.NUM_FE):
                                n = size
                                data = adcData[erng]
                                while data[tem, row, end, fe, (n - 1)] < 0:
                                    n -= 1
                                length[tem, row, end, fe, 0] = n
                                

            return (lengthData, dacData, adcData)                    
                    
                                                
        elif version == 'v2r3':
            return self.__read_v2r3()
        
        else:
            raise calFileReadExcept, "fmtVersion %s not supported" % version


    def __read_v2r2(self):
        """
        Read data from a CAL IntNonlin XML file, format version v2r2

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data (dacData, adcData):
            dacData -   A list of DAC values. The length of this array
                        is the number of data points for that energy range.
            adcData -   A list of 4 elements, each a reference to a Numeric
                        array of ADC values. The shape of each array is
                        (16, 8, 2, 12, <size>), where <size> is the length of
                        the corresponding dacData array for that energy range.
                        If the ADC data in the XML file has less points than the
                        correspoding DAC values array, the extra ADC values at
                        the end are set to '-1'.
        """

        # create empty data arrays
        
        dacData = [None, None, None, None]
        adcData = [None, None, None, None]

        dataSize = [0, 0, 0, 0]        

        # find <dac> elements

        dList = self.getDoc().getElementsByTagName('dac')
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

        for erng in range(calConstant.NUM_RNG):
            size = dataSize[erng]
            data = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, size), Numeric.Float32)
            adcData[erng] = data

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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


    def __read_v2r3(self):
        """
        Read data from a CAL IntNonlin XML file, format version v2r3

        Returns: A tuple of references to Numeric arrays and containing the
        calibration data (dacData, adcData):
            lengthData - A list of 4 elements, each a reference to a Numeric
                         array of length values.  The shape of each array
                         is (16, 8, 2, 12, 1), where the last dimension
                         contains the length of the DAC list for that
                         channel.  This value may be used to determine the
                         number of valid data values in the following
                         dacData and adcData arrays.
            dacData -   A list of 4 elements, each a reference to a Numeric
                        array of DAC values. The shape of each array is
                        (16, 8, 2, 12, 256)  The last dimension contains the
                        DAC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
            adcData -   A list of 4 elements, each a reference to a Numeric
                        array of ADC values. The shape of each array is
                        (16, 8, 2, 12, 256).  The last dimension contains the
                        ADC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
        """
        
        # create empty data arrays

        lengthData = [None, None, None, None]
        dacData = [None, None, None, None]
        adcData = [None, None, None, None]

        for erng in range(calConstant.NUM_RNG):        

            lengthData[erng] = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, 1), Numeric.Int16)
            dacData[erng] = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, INTNONLIN_MAX_DATA), Numeric.Float32)
            adcData[erng] = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, INTNONLIN_MAX_DATA), Numeric.Float32)

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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
                            
                            adata = adcData[erng]
                            ddata = dacData[erng]
                            ldata = lengthData[erng]
                            
                            valueStr = n.getAttribute('values')
                            valueList = valueStr.split()

                            sdacStr = n.getAttribute('sdacs')
                            sdacList = sdacStr.split()

                            if len(valueList) != len(sdacList):
                                raise calFileReadExcept, "<intNonlin> sdac and value lists different lengths"

                            ldata[tem, row, end, fe, 0] = len(valueList)
                                                    
                            x = 0
                            for adc in valueList:
                                adata[tem, row, end, fe, x] = float(adc)
                                d = sdacList[x]
                                ddata[tem, row, end, fe, x] = float(d)
                                x += 1
                                
        return (lengthData, dacData, adcData)            


    def info(self):
        """
        Return ancillary information from CAL IntNonlin XML file.

        Returns: A dictionary with the following keys and values:
            'instrument' - from <generic> element
            'timestamp' - from <generic> element
            'calibType' - from <generic> element
            'fmtVersion' - from <generic> element
            'DTDVersion' - from <generic> element
            'startTime' - from <inputSample> element
            'stopTime' - from <inputSample> element
            'triggers' - from <inputSample> element
            'mode' - from <inputSample> element
            'source' - from <inputSample> element
        """

        i = {}

        # get <generic> attribute values        
            
        g = self.genericInfo(i)       

        # find <inputSample> element

        isList = g.getElementsByTagName('inputSample')
        isLen = len(isList)
        if isLen != 1:
            i['startTime'] = None
            i['stopTime'] = None
            i['triggers'] = None
            i['mode'] = None
            i['source'] = None
            return i
            
        # get <inputSample> attribute values

        isn = isList[0]
        
        value = isn.getAttribute('startTime')
        if len(value) == 0:
            i['startTime'] = None
        else:
            i['startTime'] = str(value)

        value = isn.getAttribute('stopTime')
        if len(value) == 0:
            i['stopTime'] = None
        else:
            i['stopTime'] = str(value)

        value = isn.getAttribute('triggers')
        if len(value) == 0:
            i['triggers'] = None
        else:
            i['triggers'] = str(value)

        value = isn.getAttribute('mode')
        if len(value) == 0:
            i['mode'] = None
        else:
            i['mode'] = str(value)
            
        value = isn.getAttribute('source')
        if len(value) == 0:
            i['source'] = None
        else:
            i['source'] = str(value)

        return i
    


class calAsymCalibXML(calCalibXML):
    """
    CAL Asym calibration XML file class.

    This class provides methods for accessing CAL light asymmetry
    calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL Asym calibration XML file.
        """
        
        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, xposData, asymData, tems = (0,)):
        """
        Write data to a CAL Asym XML file

        Param: xposData - A list of position values. The length of this array is the
                          number of data points for each crystal.
        Param: asymData -   A Numeric array of shape (16, 8, 12, 8, <size>), where
                         <size> is the length of xposData array. The next-to-last
                         dimension contains the following data:
                             0 = bigVals value
                             1 = smallVals value
                             2 = NsmallPbigVals value
                             3 = PsmallNbigVals value
                             4 = bigSigs value
                             5 = smallSigs value
                             6 = NsmallPbigSigs value
                             7 = PsmallNbigSigs values
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_Asym')
        r.appendChild(g)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 1, nFace = 1, nXpos = 1)
        r.appendChild(d)

        # insert <xpos> element

        xp = doc.createElement('xpos')
        s = ''
        for pos in xposData:
            s += '%s ' % str(pos)
        xp.setAttribute('values', s.rstrip(' '))
        r.appendChild(xp)        

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)

                    # insert <face> element

                    f = doc.createElement('face')
                    f.setAttribute('end', 'NA')
                    x.appendChild(f)

                    # insert <asym> element

                    a = doc.createElement('asym')
                    f.appendChild(a)
                    pLen = len(xposData)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 0, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('bigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 1, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('smallVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 2, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('NsmallPbigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 3, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('PsmallNbigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 4, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('bigSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 5, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('smallSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 6, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('NsmallPbigSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 7, d]
                        s += '%0.6f ' % ad
                    a.setAttribute('PsmallNbigSigs', s)                    
                      
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL Asym XML file
        
        Returns: A tuple of references to Numeric arrays and containing the
        calibration data (xposData, asymData):
            xposData -   A list of position values. The length of this array
                         is the number of data points for each crystal.
            asymData -   A Numeric array of shape (16, 8, 12, 8, <size>), where
                         <size> is the length of xposData array. The next-to-last
                         dimension contains the following data:
                             0 = bigVals value
                             1 = smallVals value
                             2 = NsmallPbigVals value
                             3 = PsmallNbigVals value
                             4 = bigSigs value
                             5 = smallSigs value
                             6 = NsmallPbigSigs value
                             7 = PsmallNbigSigs value 
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_Asym':
            raise calFileReadExcept, "XML file is type %s (expected CAL_Asym)" % type

        # find <xpos> element

        xpList = self.getDoc().getElementsByTagName('xpos')
        xpLen = len(xpList)
        if xpLen != 1:
            raise calFileReadExcept, "found %d <xpos> elements (expected 1)" % xpLen
        xp = xpList[0]

        valueStr = xp.getAttribute('values')
        values = valueStr.split(' ')
        xposData = []
        for pos in values:
            xposData.append(float(pos))

        # create empty asymmetry data array

        asymData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE,
                                  8, len(xposData)), Numeric.Float32)           
        
        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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
                    fLen = len(fList)
                    if fLen != 1:
                        raise calFileReadExcept, "found %d <face> elements (expected 1)" % fLen
                    f = fList[0]
                    face = f.getAttribute('end')
                    
                    # find <asym> elements

                    asList = f.getElementsByTagName('asym')
                    asLen = len(asList)
                    if asLen != 1:
                        raise calFileReadExcept, "found %d <asym> elements (expected 1)" % asLen
                    as = asList[0]

                    valueList = as.getAttribute('bigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 0, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('smallVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 1, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('NsmallPbigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 2, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('PsmallNbigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 3, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('bigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 4, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('smallSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 5, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('NsmallPbigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 6, v] = float(asym)
                        v += 1

                    valueList = as.getAttribute('PsmallNbigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        asymData[tem, row, fe, 7, v] = float(asym)
                        v += 1                        

        return (xposData, asymData)



class calMevPerDacCalibXML(calCalibXML):
    """
    CAL MevPerDac calibration XML file class.

    This class provides methods for accessing CAL energy conversion
    calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL MevPerDac calibration XML file.
        """
        
        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, energyData, tems = (0,)):
        """
        Write data to a CAL MevPerDac XML file

        Param: energyData -
            A Numeric array containing the energy conversion data
            of shape (16, 8, 12, 8) The last dimension contains
            the following data for each crystal:
                0 = bigVal value
                1 = smallVal value
                2 = bigSig value
                3 = smallSig value
                4 = NEG end bigSmallRatioVals value
                5 = NEG end bigSmallRatioSigs value
                6 = POS end bigSmallRatioVals value
                7 = POS end bigSmallRatioSigs value
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_MevPerDac')
        r.appendChild(g)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 1, nFace = 1)
        r.appendChild(d)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)

                    # insert <face> element

                    f = doc.createElement('face')
                    f.setAttribute('end', 'NA')
                    x.appendChild(f)

                    # insert <mevPerDac> element

                    me = doc.createElement('mevPerDac')
                    
                    val = energyData[tem, row, fe, 0]
                    me.setAttribute('bigVal', '%0.6f' % val)
                    val = energyData[tem, row, fe, 1]
                    me.setAttribute('smallVal', '%0.6f' % val)
                    val = energyData[tem, row, fe, 2]
                    me.setAttribute('bigSig', '%0.6f' % val)
                    val = energyData[tem, row, fe, 3]
                    me.setAttribute('smallSig', '%0.6f' % val)

                    f.appendChild(me)

                    # insert <bigSmall> elements

                    for end in range(calConstant.NUM_END):

                        bs = doc.createElement('bigSmall')

                        bs.setAttribute('end', POSNEG[end])
                        val = energyData[tem, row, fe, (4 + (end * 2))]
                        bs.setAttribute('bigSmallRatioVals', '%0.6f' % val)
                        val = energyData[tem, row, fe, (5 + (end * 2))]
                        bs.setAttribute('bigSmallRatioSigs', '%0.6f' % val)

                        me.appendChild(bs)
                        
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL MevPerDac XML file\
        
        Returns: A Numeric array containing the energy conversion data
                 of shape (16, 8, 12, 8) The last dimension contains
                 the following data for each crystal:
                     0 = bigVal value
                     1 = smallVal value
                     2 = bigSig value
                     3 = smallSig value
                     4 = NEG end bigSmallRatioVals value
                     5 = NEG end bigSmallRatioSigs value
                     6 = POS end bigSmallRatioVals value
                     7 = POS end bigSmallRatioSigs value
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_MevPerDac':
            raise calFileReadExcept, "XML file is type %s (expected CAL_MevPerDac)" % type

        # create empty data array        

        energyData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE, 8),
                                   Numeric.Float32)

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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
                    fLen = len(fList)
                    if fLen != 1:
                        raise calFileReadExcept, "found %d <face> elements (expected 1)" % fLen
                    f = fList[0]
                    face = f.getAttribute('end')
                    
                    # find <mevPerDac> elements

                    meList = f.getElementsByTagName('mevPerDac')
                    meLen = len(meList)
                    if meLen != 1:
                        raise calFileReadExcept, "found %d <mevPerDac> elements (expected 1)" % meLen
                    me = meList[0]
                    eng = me.getAttribute('bigVal')
                    energyData[tem, row, fe, 0] = float(eng)
                    eng = me.getAttribute('smallVal')
                    energyData[tem, row, fe, 1] = float(eng)
                    eng = me.getAttribute('bigSig')
                    energyData[tem, row, fe, 2] = float(eng)
                    eng = me.getAttribute('smallSig')
                    energyData[tem, row, fe, 3] = float(eng)

                    # find <bigSmall> elements

                    bsList = me.getElementsByTagName('bigSmall')
                    for bs in bsList:

                        end = f.getAttribute('end')
                        if end == 'NEG':
                            eng = bs.getAttribute('bigSmallRatioVals')
                            energyData[tem, row, fe, 4] = float(eng)
                            eng = bs.getAttribute('bigSmallRatioSigs')
                            energyData[tem, row, fe, 5] = float(eng)
                        else:
                            eng = bs.getAttribute('bigSmallRatioVals')
                            energyData[tem, row, fe, 6] = float(eng)
                            eng = bs.getAttribute('bigSmallRatioSigs')
                            energyData[tem, row, fe, 7] = float(eng)
                                    
        return energyData



class calPedCalibXML(calCalibXML):
    """
    CAL Ped calibration XML file class.

    This class provides methods for accessing CAL pedestal
    calibration data stored in XML format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL Ped calibration XML file.

        Param: fileName - The XML file name.
        Param: mode - The XML file access mode.
        """
        
        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, pedData, tems = (0,)):
        """
        Write data to a CAL Ped XML file

        Param: pedData -
            A Numeric array containing the pedestal data
            of shape (16, 8, 2, 12, 4, 3) The last dimension contains
            the following data for each crystal end and energy
            range:
                0 = avg value
                1 = sig value
                2 = cos values
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElement('calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_Ped')
        r.appendChild(g)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4, nFace = 2)
        r.appendChild(d)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElement('tower')
            t.setAttribute('iRow', str(iRow))
            t.setAttribute('iCol', str(iCol))
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElement('layer')
                l.setAttribute('iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElement('xtal')
                    x.setAttribute('iXtal', str(fe))
                    l.appendChild(x)

                    for end in range(2):

                        # insert <face> elements

                        f = doc.createElement('face')
                        f.setAttribute('end', POSNEG[end])
                        x.appendChild(f)

                        # insert <calPed> elements

                        for erng in range(calConstant.NUM_RNG):

                            p = doc.createElement('calPed')
                            p.setAttribute('range', calConstant.CRNG[erng])
                            ped = pedData[tem, row, end, fe, erng, 0]
                            p.setAttribute('avg', '%0.6f' % ped)
                            ped = pedData[tem, row, end, fe, erng, 1]
                            p.setAttribute('sig', '%0.6f' % ped)
                            ped = pedData[tem, row, end, fe, erng, 2]
                            p.setAttribute('cos', '%0.6f' % ped)
                            f.appendChild(p)
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL Ped XML file
        
        Returns: A Numeric array containing the pedestal data
                 of shape (16, 8, 2, 12, 4, 3) The last dimension contains
                 the following data for each crystal end and energy
                 range:
                     0 = avg value
                     1 = sig value
                     2 = cos value
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_Ped':
            raise calFileReadExcept, "XML file is type %s (expected CAL_Ped)" % type

        # create empty data array

        pedData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, calConstant.NUM_RNG, 3), Numeric.Float32)

        # find <tower> elements

        tList = self.getDoc().getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            tem = towerToTem(tCol, tRow)

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
        
                        # find <calPed> elements

                        pList = f.getElementsByTagName('calPed')
                        for p in pList:

                            erngName = p.getAttribute('range')
                            erng = ERNG_MAP[erngName]

                            avg = float(p.getAttribute('avg'))
                            pedData[tem, row, end, fe, erng, 0] = avg

                            sig = float(p.getAttribute('sig'))
                            pedData[tem, row, end, fe, erng, 1] = sig

                            #cos = float(p.getAttribute('cos'))
                            pedData[tem, row, end, fe, erng, 2] = 2.0
                           
                                    
        return pedData           


        
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

    
def towerToTem(twrCol, twrRow):
    """
    Translate tower row and column indicies to TEM number.

    Param: twrCol - The tower column index (0 - 3)
    Param: twrRow - The tower row index (0 - 3)

    Returns: The TEM number (0 - 15)
    """

    return twrCol + (4 * twrRow)


def temToTower(temNum):
    """
    Translate tower row and column indicies to TEM number.

    Param: temNum - The TEM number (0 - 15)

    Returns: A tuple (twrCol, twrRow):
        twrCol - The tower column index (0 - 3)
        twrRow - The tower row index (0 - 3)
    """

    twrCol = (temNum % 4)
    twrRow = (temNum / 4)
    return (twrCol, twrRow)



def insertDTD(outName, dtdName):
    """
    Fixup calibration XML file - insert DTD info

    Param: outName - XML output file name
    Param: dtdName - DTD file name
    """

    outFile = open(outName, 'r')
    lines = outFile.readlines()
    outFile.close()

    dtdStr = '<!DOCTYPE calCalib ['
    dtdFile = open(dtdName, 'r')
    dtdLines = dtdFile.readlines()
    dtdFile.close()
    for l in dtdLines:
        dtdStr += l
    dtdStr += ']>\n'

    outFile = open(outName, 'w')
    outFile.write(lines[0])
    outFile.write(dtdStr)
    outFile.writelines(lines[1:])
    outFile.close()

    
