"""
Classes to represent CAL calibration XML documents.
"""


__facility__  = "Offline"
__abstract__  = "Classes to represent CAL calibration XML documents."
__author__    = "D.L.Wood"
__date__      = "$Date: 2008/04/29 14:01:59 $"
__version__   = "$Revision: 1.20 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import time

import numarray

import calXML
import calConstant
from calExcept import *



MODE_CREATE     = calXML.MODE_CREATE
MODE_READONLY   = calXML.MODE_READONLY



_POSNEG = ('NEG', 'POS')

_ERNG_MAP = {'LEX8' : calConstant.CRNG_LEX8, 
             'LEX1' : calConstant.CRNG_LEX1, 
             'HEX8' : calConstant.CRNG_HEX8, 
             'HEX1' : calConstant.CRNG_HEX1}

_POSNEG_MAP = {'NEG' : 0, 
               'POS' : 1}

_DRNG_MAP = {'COARSE' : calConstant.CDAC_COARSE, 
            'FINE'  : calConstant.CDAC_FINE}


INTNONLIN_MAX_DATA = 256



class calCalibXML(calXML.calXML):
    """
    CAL calibration XML file class.

    This class provides methods for accessing CAL calibration data stored
    in XML format.
    """
  
    def __init__(self, fileName, mode = MODE_READONLY, validating = True):
        """
        Open a CAL calibration XML file

        \param fileName The XML file name.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        calXML.calXML.__init__(self, fileName, mode, validating)


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

        g = self.getDoc().createElementNS(None, 'generic')
        g.setAttributeNS(None, 'instrument', 'LAT')
        g.setAttributeNS(None, 'calibType', str(type))
        g.setAttributeNS(None, 'fmtVersion', 'v2r3')
        g.setAttributeNS(None, 'creator', 'myname')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        g.setAttributeNS(None, 'timestamp', ts)

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

        d = self.getDoc().createElementNS(None, 'dimension')
        d.setAttributeNS(None, 'nRow', '4')
        d.setAttributeNS(None, 'nCol', '4')
        d.setAttributeNS(None, 'nLayer', '8')
        d.setAttributeNS(None, 'nXtal', '12')
        d.setAttributeNS(None, 'nFace', str(nFace))
        d.setAttributeNS(None, 'nRange', str(nRange))
        d.setAttributeNS(None, 'exact', 'false')
        d.setAttributeNS(None, 'nDacCol', str(nDacCol))
        d.setAttributeNS(None, 'nXpos', str(nXpos))

        return d


    def getType(self):
        """
        Get CAL calibration XML file type.

        Returns: The <generic> calibType attribute value.        
        """

        # find <generic> element

        gList = self.getDoc().xpath('.//generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        value = g.getAttributeNS(None, 'calibType')
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

        gList = self.getDoc().xpath('.//generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        # fill in dictionary        

        value = g.getAttributeNS(None, 'instrument')
        if len(value) == 0:
            infoDict['instrument'] = None
        else:
            infoDict['instrument'] = str(value)

        value = g.getAttributeNS(None, 'calibType')
        if len(value) == 0:
            infoDict['calibType'] = None
        else:
            infoDict['calibType'] = str(value)

        value = g.getAttributeNS(None, 'timestamp')
        if len(value) == 0:
            infoDict['timestamp'] = None
        else:
            infoDict['timestamp'] = str(value)

        value = g.getAttributeNS(None, 'fmtVersion')
        if len(value) == 0:
            infoDict['fmtVersion'] = None
        else:
            infoDict['fmtVersion'] = str(value)

        value = g.getAttributeNS(None, 'DTDVersion')
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

        tList = self.getDoc().xpath('.//tower')
        for t in tList:

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)
            towers.append(tem)

        return towers
        
        
    def read(self):
        """
        Stub for document read method.
        Must be provided by derived classes.
        """
        
        raise calFileReadExcept, "calCalibXML-derived classes must implement read() method"
        
        
    def write(self):
        """
        Stub for document write method.
        Must be provided by derived classes.
        """
        
        raise calFileWriteExcept, "calCalibXML-derived classes must implement write() method"

    
    
class calTholdCICalibXML(calCalibXML):
    """
    CAL TholdCI calibration XML file class.

    This class provides methods for accessing CAL threshold charge
    injection calibration data stored in XML format.
    """

    def generate(self, dacData, adcData, intNonlinData, intNonlinLength, pedData,
                 lrefGain, hrefGain, tems = (0,)):
        """
        Calculate threshold values in ADC units & write data to a CAL TholdCI XML file

        Param: dacData - A list of numarray arrays of DAC settings data
            (16, 8, 2, 12):
            dacData[0] = ULD DAC settings data
            dacData[1] = LAC DAC settings data
            dacData[2] = FLE DAC settings data
            dacData[3] = FHE DAC settings data
        Param: adcData - A list of numarray arrays of ADC/DAC characterization data.
            adcData[0] = A numarray array of ULD ADC/DAC characterization data
                (3, 16, 8, 2, 12, 128).
            adcData[1] = A numarray array of LAC ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[2] = A numarray array of FLE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
            adcData[3] = A numarray array of FHE ADC/DAC characterization data
                (16, 8, 2, 12, 128). \n
        Param: intNonlinData - A numarray array of ADC non-linearity characterization
            data for HEX1 energy range (16, 8, 2, 12, <N>).
        Param: intNonlinLength = A numarray array of ADC intnonlin data lengths for
            HEX1 energy range (16, 8, 2, 12, 1)
        Param: pedData - A numarray array of pedestal value data as returned from calPedCalibXML.read()
        Param: lrefGain A numarray array of LE gain index settings data
            (16, 8, 2, 12)
        Param: hrefGain A numarray array of HE gain index settings data
            (16, 8, 2, 12).
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

        r = doc.createElementNS(None, 'calCalib')
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
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        cs = ""                        

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        # insert <tholdCI> element

                        tc = doc.createElementNS(None, 'tholdCI')
                        
                        dac = int(lacDac[tem, row, end, fe])
                        adc = lacAdc[tem, row, end, fe, dac]
                        cs += 'LAC DAC = %d, ' % dac
                        tc.setAttributeNS(None, 'LACVal', "%0.3f" % adc)
                        tc.setAttributeNS(None, 'LACSig', '0')
                        
                        dac = int(fleDac[tem, row, end, fe])
                        adc = fleAdc[tem, row, end, fe, dac]
                        cs += 'FLE DAC = %d, ' % dac
                        tc.setAttributeNS(None, 'FLEVal', "%0.3f" % adc)
                        tc.setAttributeNS(None, 'FLESig', '0')

                        dac = int(fheDac[tem, row, end, fe])
                        adc = fheAdc[tem, row, end, fe, dac]
                        cs += 'FHE DAC = %d, ' % dac
                        tc.setAttributeNS(None, 'FHEVal', "%0.3f" % adc)
                        tc.setAttributeNS(None, 'FHESig', '0')

                        dac = int(uldDac[tem, row, end, fe])    
                        cs += 'ULD DAC = %d, ' % dac

                        cs += 'LE gain = %d, ' % lrefGain[tem, row, end, fe]
                        cs += 'HE gain = %d, ' % hrefGain[tem, row, end, fe]

                        c = doc.createComment(cs)                        
                        tc.appendChild(c)
                        
                        f.appendChild(tc)
                        
                        for erng in range(3):
                       
                            # insert <tholdCIRange> elements

                            tcr = doc.createElementNS(None, 'tholdCIRange')
                            
                            tcr.setAttributeNS(None, 'range', calConstant.CRNG[erng])
                            
                            dac = int(uldDac[tem, row, end, fe])
                            adc = uldAdc[erng, tem, row, end, fe, dac]
                
                            tcr.setAttributeNS(None, 'ULDVal', "%0.3f" % adc)
                            tcr.setAttributeNS(None, 'ULDSig', '30')

                            ped = pedData[tem, row, end, fe, erng, calPedCalibXML.PED_VAL_IDX]
                            pedSig = pedData[tem, row, end, fe, erng, calPedCalibXML.PED_SIG_IDX]
                            tcr.setAttributeNS(None, 'pedVal', "%0.3f" % ped)
                            tcr.setAttributeNS(None, 'pedSig', "%0.3f" % pedSig)
                            
                            tc.appendChild(tcr)

                        # insert <tholdCIRange> element for HEX1 range
                        # use top element from intNonlin ADC value list

                        tcr = doc.createElementNS(None, 'tholdCIRange')
                        
                        tcr.setAttributeNS(None, 'range', calConstant.CRNG[calConstant.CRNG_HEX1])

                        size = int(intNonlinLength[tem, row, end, fe, 0])
                        adc = intNonlinData[tem, row, end, fe, (size - 1)]
                        tcr.setAttributeNS(None, 'ULDVal', '%0.3f' % adc)
                        tcr.setAttributeNS(None, 'ULDSig', '30')

                        ped = pedData[tem, row, end, fe, calConstant.CRNG_HEX1, calPedCalibXML.PED_VAL_IDX]
                        pedSig = pedData[tem, row, end, fe, calConstant.CRNG_HEX1, calPedCalibXML.PED_SIG_IDX]
                        tcr.setAttributeNS(None, 'pedVal', "%0.3f" % ped)
                        tcr.setAttributeNS(None, 'pedSig', "%0.3f" % pedSig)
                        
                        tc.appendChild(tcr)
                    
        # write output XML file

        self.writeFile()

    def write(self, adcData, uldData, pedData, tems = (0,)):
        """
        write ADC threshold values directly to XML without any calculations.
        hopefully will replace other implemenation of write() method which is not cohesive and
        forces coupling between calcuation of thresholds and file write

        params: (similar to return values of read() method)
            adcData - A numarray array of shape (16, 8, 2, 12, 3).  The last
            dimension holds the LAC, FLE, and FHE values for the channel:
                [:, 0] = LAC ADC threshold values
                [:, 1] = FLE ADC threshold values
                [:, 2] = FHE ADC threshold values
            uldData - A numarray array of shape (16, 8, 2, 12, 4).  The last
            dimension holds the ULD values for each energy range:
                [:, 0] = LEX8 ULD ADC threshold values
                [:, 1] = LEX1 ULD ADC threshold values
                [:, 2] = HEX8 ULD ADC threshold values
                [:, 3] = HEX1 ULD ADC threshold values
            pedData - A numarray array of shape (16, 8, 2, 12, 4).  The last
            dimension holds the pedestal values for each energy range:
                [:, 0] = LEX8 ULD pedestal values
                [:, 1] = LEX1 ULD pedestal values
                [:, 2] = HEX8 ULD pedestal values
                [:, 3] = HEX1 ULD pedestal values

            tems: a list of TEM ID's to write out
            
        """
        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElementNS(None, 'calCalib')
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
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        cs = ""                        

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        # insert <tholdCI> element

                        tc = doc.createElementNS(None, 'tholdCI')

                        lac = adcData[tem,row,end,fe,0]
                        fle = adcData[tem,row,end,fe,1]
                        fhe = adcData[tem,row,end,fe,2]

                        tc.setAttributeNS(None, 'LACVal', "%0.3f" % lac)
                        tc.setAttributeNS(None, 'LACSig', '0')

                        tc.setAttributeNS(None, 'FLEVal', "%0.3f" % fle)
                        tc.setAttributeNS(None, 'FLESig', '0')

                        tc.setAttributeNS(None, 'FHEVal', "%0.3f" % fhe)
                        tc.setAttributeNS(None, 'FHESig', '0')

                        f.appendChild(tc)
                                               
                        for erng in range(calConstant.NUM_RNG):

                            # insert <tholdCIRange> elements

                            tcr = doc.createElementNS(None, 'tholdCIRange')
                            
                            tcr.setAttributeNS(None, 'range', calConstant.CRNG[erng])

                            uld = uldData[tem,row,end,fe,erng]
                            tcr.setAttributeNS(None, 'ULDVal', "%0.3f" % uld)
                            tcr.setAttributeNS(None, 'ULDSig', '0')

                            ped = pedData[tem,row,end,fe,erng]
                            tcr.setAttributeNS(None, 'pedVal', "%0.3f" % ped)
                            tcr.setAttributeNS(None, 'pedSig', '0')                            
                            
                            tc.appendChild(tcr)

        # write output XML file

        self.writeFile()

    # array indices
    ADCDATA_LAC = 0
    ADCDATA_FLE = 1
    ADCDATA_FHE = 2

    def read(self):
        """
        Read data from a CAL TholdCI XML file

        Returns: A tuple of references to numarray arrays and containing the
        calibration data (adcData, uldData, pedData):
            adcData - A numarray array of shape (16, 8, 2, 12, 3).  The last
            dimension holds the LAC, FLE, and FHE values for the channel:
                [:, 0] = LAC ADC threshold values
                [:, 1] = FLE ADC threshold values
                [:, 2] = FHE ADC threshold values
            uldData - A numarray array of shape (16, 8, 2, 12, 4).  The last
            dimension holds the ULD values for each energy range:
                [:, 0] = LEX8 ULD ADC threshold values
                [:, 1] = LEX1 ULD ADC threshold values
                [:, 2] = HEX8 ULD ADC threshold values
                [:, 3] = HEX1 ULD ADC threshold values
            pedData - A numarray array of shape (16, 8, 2, 12, 4).  The last
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
        
        adcData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 3), numarray.Float32)
        uldData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 4), numarray.Float32)
        pedData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 4), numarray.Float32)

        # find <tower> elements

        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements
 
                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    for f in x.xpath('.//face'):

                        end = _POSNEG_MAP[f.getAttributeNS(None, 'end')]
        
                        # find <tholdCI> elements

                        for ci in f.xpath('.//tholdCI'):
                            
                            ad = adcData[tem, row, end, fe, :]
                            
                            ad[0] = float(ci.getAttributeNS(None, 'LACVal'))
                            ad[1] = float(ci.getAttributeNS(None, 'FLEVal'))
                            ad[2] = float(ci.getAttributeNS(None, 'FHEVal'))

                            # find <tholdCIRange> elements
 
                            for cir in ci.xpath('.//tholdCIRange'):
                            
                                erngName = cir.getAttributeNS(None, 'range')
                                erng = _ERNG_MAP[erngName]
                                adc = cir.getAttributeNS(None, 'ULDVal')
                                uldData[tem, row, end, fe, erng] = float(adc)
                                adc = cir.getAttributeNS(None, 'pedVal')
                                pedData[tem, row, end, fe, erng] = float(adc)

        return (adcData, uldData, pedData)   

        
        
class calIntNonlinCalibXML(calCalibXML):
    """
    CAL IntNonlin calibration XML file class.

    This class provides methods for accessing CAL int non-linearity
    calibration data stored in XML format.
    """


    def write(self, lengthData, dacData, adcData, inputSample = None, tems = (0,)):
        """
        Write data to a CAL IntNonlin XML file

        Param: lengthData - A list of 4 elements, each a reference to a numarray
                            array of DAC values. The shape of each array is
                            (16, 8, 2, 12, 1), where the last dimension holds
                            the length of the ADC and DAC value lists for that
                            channel.
        Param: dacData - A list of 4 elements, each a reference to a numarray
                         array of DAC values. The shape of each array is
                         (16, 8, 2, 12, <size>), where <size> is the length of
                         the corresponding dacData array for that energy range.
        Param: adcData - A list of 4 elements, each a reference to a numarray
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

        r = doc.createElementNS(None, 'calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_IntNonlin')
        r.appendChild(g)

        # insert <inputSample> element

        if inputSample is not None:
            s = doc.createElementNS(None, 'inputSample')
            if startTime is not None:
                s.setAttributeNS(None, 'startTime', inputSample['startTime'])
            if stopTime is not None:
                s.setAttributeNS(None, 'stopTime', inputSample['stopTime'])
            if triggers is not None:
                s.setAttributeNS(None, 'triggers', inputSample['triggers'])
            if mode is not None:
                s.setAttributeNS(None, 'mode', inputSample['mode'])
            if source is not None:
                s.setAttributeNS(None, 'source', inputSample['source'])
            g.appendChild(s)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4)
        r.appendChild(d)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            # insert <intNonlin> elements

                            n = doc.createElementNS(None, 'intNonlin')
                            n.setAttributeNS(None, 'range', calConstant.CRNG[erng])

                            # get length of data lists for this channel

                            length = lengthData[erng]                            

                            # insert 'values' attribute holding ADC data
                            
                            s = ''
                            data = adcData[erng]
                            for xi in range(length[tem, row, end, fe, 0]): 
                                adc = data[tem, row, end, fe, xi]               
                                s += '%0.2f ' % float(adc)
                            n.setAttributeNS(None, 'values', s.rstrip(' '))

                            # insert 'sdacs' attribute holding DAC data
                            
                            s = ''
                            data = dacData[erng]
                            for xi in range(length[tem, row, end, fe, 0]): 
                                dac = data[tem, row, end, fe, xi]               
                                s += '%0.2f ' % float(dac)
                            n.setAttributeNS(None, 'sdacs', s.rstrip(' '))                            
                            
                            n.setAttributeNS(None, 'error', '0.10')
                            f.appendChild(n)
    
        # write output XML file

        self.writeFile()
        

    def __write_v2r2(self, dacData, adcData, inputSample = None, tems = (0,)):             
        """
        Write data to a CAL IntNonlin XML file, version v2r2

        Param: dacData - A list of DAC values. The length of this array is the
                         number of data points for that energy range.
        Param: adcData - A list of 4 elements, each a reference to a numarray
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

        r = doc.createElementNS(None, 'calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_IntNonlin')
        r.appendChild(g)

        # insert <inputSample> element

        if inputSample is not None:
            s = doc.createElementNS(None, 'inputSample')
            if startTime is not None:
                s.setAttributeNS(None, 'startTime', inputSample['startTime'])
            if stopTime is not None:
                s.setAttributeNS(None, 'stopTime', inputSample['stopTime'])
            if triggers is not None:
                s.setAttributeNS(None, 'triggers', inputSample['triggers'])
            if mode is not None:
                s.setAttributeNS(None, 'mode', inputSample['mode'])
            if source is not None:
                s.setAttributeNS(None, 'source', inputSample['source'])
            g.appendChild(s)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4, nDacCol = 4)
        r.appendChild(d)

        # insert <dac> elements

        for erng in range(calConstant.NUM_RNG):

            dr = doc.createElementNS(None, 'dac')
            dr.setAttributeNS(None, 'range', calConstant.CRNG[erng])
            s = ''
            data = dacData[erng]
            for dac in data:
                s += '%s ' % str(dac)
            dr.setAttributeNS(None, 'values', s.rstrip(' '))
            dr.setAttributeNS(None, 'error', '0.0')
            r.appendChild(dr)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            # insert <intNonlin> elements

                            n = doc.createElementNS(None, 'intNonlin')
                            n.setAttributeNS(None, 'range', calConstant.CRNG[erng])
                            s = ''
                            data = adcData[erng]
                            for adc in data[tem, row, end, fe, :]:
                                if adc < 0:
                                    continue
                                s += '%0.2f ' % float(adc)
                            n.setAttributeNS(None, 'values', s.rstrip(' '))
                            n.setAttributeNS(None, 'error', '0.00')
                            f.appendChild(n)
    
        # write output XML file

        self.writeFile()
        

    def read(self):
        """
        Read data from a CAL IntNonlin XML file

        Returns: A tuple of references to numarray arrays and containing the
        calibration data (lengthData, dacData, adcData):
            lengthData - A list of 4 elements, each a reference to a numarray
                         array of length values.  The shape of each array
                         is (16, 8, 2, 12, 1), where the last dimension
                         contains the length of the DAC list for that
                         channel.  This value may be used to determine the
                         number of valid data values in the following
                         dacData and adcData arrays.
            dacData -   A list of 4 elements, each a reference to a numarray
                        array of DAC values. The shape of each array is
                        (16, 8, 2, 12, 256)  The last dimension contains the
                        DAC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
            adcData -   A list of 4 elements, each a reference to a numarray
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

        gList = self.getDoc().xpath('.//generic')
        gLen = len(gList)
        if gLen != 1:
            raise calFileReadExcept, "found %d <generic> elements (expected 1)" % gLen
        g = gList[0]

        # get format version

        value = g.getAttributeNS(None, 'fmtVersion')
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
                dacData[erng] = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, size), numarray.Float32)
                lengthData[erng] = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, 1), numarray.Int16)

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

        Returns: A tuple of references to numarray arrays and containing the
        calibration data (dacData, adcData):
            dacData -   A list of DAC values. The length of this array
                        is the number of data points for that energy range.
            adcData -   A list of 4 elements, each a reference to a numarray
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

        dList = self.getDoc().xpath('.//dac')
        for d in dList:
            erngName = d.getAttributeNS(None, 'range')
            erng = _ERNG_MAP[erngName]
            valueStr = d.getAttributeNS(None, 'values')
            valueList = valueStr.split()
            data = []
            for dac in valueList:
                data.append(int(dac))
            dacData[erng] = data
            dataSize[erng] = len(data)

        # create empty ADC data arrays

        for erng in range(calConstant.NUM_RNG):
            size = dataSize[erng]
            data = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, size), numarray.Float32)
            adcData[erng] = data

        # find <tower> elements

        tList = self.getDoc().xpath('.//tower')
        for t in tList:

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            lList = t.xpath('.//layer')
            for l in lList:

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                xList = l.xpath('.//xtal')
                for x in xList:

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    fList = x.xpath('.//face')
                    for f in fList:

                        face = f.getAttributeNS(None, 'end')
                        end = _POSNEG_MAP[face]

                        # find <intNonlin> elements

                        nList = f.xpath('.//intNonlin')
                        for n in nList:

                            erngName = n.getAttributeNS(None, 'range')
                            erng = _ERNG_MAP[erngName]
                            valueStr = n.getAttributeNS(None, 'values')
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

        Returns: A tuple of references to numarray arrays and containing the
        calibration data (dacData, adcData):
            lengthData - A list of 4 elements, each a reference to a numarray
                         array of length values.  The shape of each array
                         is (16, 8, 2, 12, 1), where the last dimension
                         contains the length of the DAC list for that
                         channel.  This value may be used to determine the
                         number of valid data values in the following
                         dacData and adcData arrays.
            dacData -   A list of 4 elements, each a reference to a numarray
                        array of DAC values. The shape of each array is
                        (16, 8, 2, 12, 256)  The last dimension contains the
                        DAC values.  The number of valid values is determined
                        by the corresponding value from the lengthData arrays.
            adcData -   A list of 4 elements, each a reference to a numarray
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

            lengthData[erng] = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, 1), numarray.Int16)
            dacData[erng] = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, INTNONLIN_MAX_DATA), numarray.Float32)
            adcData[erng] = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                  calConstant.NUM_FE, INTNONLIN_MAX_DATA), numarray.Float32)

        # find <tower> elements
 
        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    for f in x.xpath('.//face'):

                        face = f.getAttributeNS(None, 'end')
                        end = _POSNEG_MAP[face]

                        # find <intNonlin> elements

                        for n in f.xpath('.//intNonlin'):

                            erngName = n.getAttributeNS(None, 'range')
                            erng = _ERNG_MAP[erngName]
                            
                            adata = adcData[erng]
                            ddata = dacData[erng]
                            ldata = lengthData[erng]
                            
                            valueStr = n.getAttributeNS(None, 'values')
                            valueList = valueStr.split()

                            sdacStr = n.getAttributeNS(None, 'sdacs')
                            sdacList = sdacStr.split()

                            if len(valueList) != len(sdacList):
                                raise calFileReadExcept, "<intNonlin> sdac and value lists different lengths"

                            ldata[tem, row, end, fe, 0] = len(valueList)
                                                    
                            x = 0
                            ad = adata[tem, row, end, fe, :]
                            dd = ddata[tem, row, end, fe, :]
                            
                            for adc in valueList:
                                ad[x] = float(adc) 
                                dd[x] = float(sdacList[x])
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

        isList = g.xpath('.//inputSample')
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
        
        value = isn.getAttributeNS(None, 'startTime')
        if len(value) == 0:
            i['startTime'] = None
        else:
            i['startTime'] = str(value)

        value = isn.getAttributeNS(None, 'stopTime')
        if len(value) == 0:
            i['stopTime'] = None
        else:
            i['stopTime'] = str(value)

        value = isn.getAttributeNS(None, 'triggers')
        if len(value) == 0:
            i['triggers'] = None
        else:
            i['triggers'] = str(value)

        value = isn.getAttributeNS(None, 'mode')
        if len(value) == 0:
            i['mode'] = None
        else:
            i['mode'] = str(value)
            
        value = isn.getAttributeNS(None, 'source')
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
        

    def write(self, xposData, asymData, tems = (0,)):
        """
        Write data to a CAL Asym XML file

        Param: xposData - A list of position values. The length of this array is the
                          number of data points for each crystal.
        Param: asymData -   A numarray array of shape (16, 8, 12, 8, <size>), where
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

        r = doc.createElementNS(None, 'calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_Asym')
        r.appendChild(g)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 1, nFace = 1, nXpos = 1)
        r.appendChild(d)

        # insert <xpos> element

        xp = doc.createElementNS(None, 'xpos')
        s = ''
        for pos in xposData:
            s += '%s ' % str(pos)
        xp.setAttributeNS(None, 'values', s.rstrip(' '))
        r.appendChild(xp)        

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)

                    # insert <face> element

                    f = doc.createElementNS(None, 'face')
                    f.setAttributeNS(None, 'end', 'NA')
                    x.appendChild(f)

                    # insert <asym> element

                    a = doc.createElementNS(None, 'asym')
                    f.appendChild(a)
                    pLen = len(xposData)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 0, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'bigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 1, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'smallVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 2, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'NsmallPbigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 3, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'PsmallNbigVals', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 4, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'bigSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 5, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'smallSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 6, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'NsmallPbigSigs', s)

                    s = ''                    
                    for d in range(pLen):
                        ad = asymData[tem, row, fe, 7, d]
                        s += '%0.6f ' % ad
                    a.setAttributeNS(None, 'PsmallNbigSigs', s)                    
                      
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL Asym XML file
        
        Returns: A tuple of references to numarray arrays and containing the
        calibration data (xposData, asymData):
            xposData -   A list of position values. The length of this array
                         is the number of data points for each crystal.
            asymData -   A numarray array of shape (16, 8, 12, 8, <size>), where
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

        xp = self.getDoc().xpath('.//xpos')[0]

        valueStr = xp.getAttributeNS(None, 'values')
        values = valueStr.split(' ')
        xposData = []
        for pos in values:
            xposData.append(float(pos))

        # create empty asymmetry data array

        asymData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE,
                                  8, len(xposData)), numarray.Float32)           
        
        # find <tower> elements

        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements
             
            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements
 
                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    f = x.xpath('.//face')[0]
                    
                    # find <asym> elements

                    as = f.xpath('.//asym')[0]
                    ad = asymData[tem, row, fe, ...]

                    valueList = as.getAttributeNS(None, 'bigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[0, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'smallVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[1, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'NsmallPbigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[2, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'PsmallNbigVals')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[3, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'bigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[4, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'smallSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[5, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'NsmallPbigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[6, v] = float(asym)
                        v += 1

                    valueList = as.getAttributeNS(None, 'PsmallNbigSigs')
                    values = valueList.split(' ')
                    v = 0
                    for asym in values:
                        ad[7, v] = float(asym)
                        v += 1                        

        return (xposData, asymData)



class calMevPerDacCalibXML(calCalibXML):
    """
    CAL MevPerDac calibration XML file class.

    This class provides methods for accessing CAL energy conversion
    calibration data stored in XML format.
    """
        

    def write(self, energyData, tems = (0,)):
        """
        Write data to a CAL MevPerDac XML file

        Param: energyData -
            A numarray array containing the energy conversion data
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

        r = doc.createElementNS(None, 'calCalib')
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
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)

                    # insert <face> element

                    f = doc.createElementNS(None, 'face')
                    f.setAttributeNS(None, 'end', 'NA')
                    x.appendChild(f)

                    # insert <mevPerDac> element

                    me = doc.createElementNS(None, 'mevPerDac')
                    
                    val = energyData[tem, row, fe, 0]
                    me.setAttributeNS(None, 'bigVal', '%0.6f' % val)
                    val = energyData[tem, row, fe, 1]
                    me.setAttributeNS(None, 'smallVal', '%0.6f' % val)
                    val = energyData[tem, row, fe, 2]
                    me.setAttributeNS(None, 'bigSig', '%0.6f' % val)
                    val = energyData[tem, row, fe, 3]
                    me.setAttributeNS(None, 'smallSig', '%0.6f' % val)

                    f.appendChild(me)

                    # insert <bigSmall> elements

                    for end in range(calConstant.NUM_END):

                        bs = doc.createElementNS(None, 'bigSmall')

                        bs.setAttributeNS(None, 'end', _POSNEG[end])
                        val = energyData[tem, row, fe, (4 + (end * 2))]
                        bs.setAttributeNS(None, 'bigSmallRatioVals', '%0.6f' % val)
                        val = energyData[tem, row, fe, (5 + (end * 2))]
                        bs.setAttributeNS(None, 'bigSmallRatioSigs', '%0.6f' % val)

                        me.appendChild(bs)
                        
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL MevPerDac XML file\
        
        Returns: A numarray array containing the energy conversion data
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

        energyData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE, 8),
                                   numarray.Float32)

        # find <tower> elements
 
        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements
 
                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    f = x.xpath('.//face')[0]
                    
                    # find <mevPerDac> elements

                    me = f.xpath('.//mevPerDac')[0]
                    ed = energyData[tem, row, fe, :]
                    
                    ed[0] = float(me.getAttributeNS(None, 'bigVal'))
                    ed[1] = float(me.getAttributeNS(None, 'smallVal'))
                    ed[2] = float(me.getAttributeNS(None, 'bigSig'))
                    ed[3] = float(me.getAttributeNS(None, 'smallSig'))

                    # find <bigSmall> elements
 
                    for bs in me.xpath('.//bigSmall'):

                        end = f.getAttributeNS(None, 'end')
                        if end == 'NEG':
                            ed[4] = float(bs.getAttributeNS(None, 'bigSmallRatioVals'))
                            ed[5] = float(bs.getAttributeNS(None, 'bigSmallRatioSigs'))
                        else:
                            ed[6] = float(bs.getAttributeNS(None, 'bigSmallRatioVals'))
                            ed[7] = float(bs.getAttributeNS(None, 'bigSmallRatioSigs'))
                                    
        return energyData



class calPedCalibXML(calCalibXML):
    """
    CAL Ped calibration XML file class.

    This class provides methods for accessing CAL pedestal
    calibration data stored in XML format.
    """

    PED_VAL_IDX = 0
    PED_SIG_IDX = 1
            

    def write(self, pedData, tems = (0,)):
        """
        Write data to a CAL Ped XML file

        Param: pedData -
            A numarray array containing the pedestal data
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

        r = doc.createElementNS(None, 'calCalib')
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
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)

                    for end in range(2):

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        # insert <calPed> elements

                        for erng in range(calConstant.NUM_RNG):

                            p = doc.createElementNS(None, 'calPed')
                            p.setAttributeNS(None, 'range', calConstant.CRNG[erng])
                            ped = pedData[tem, row, end, fe, erng, 0]
                            p.setAttributeNS(None, 'avg', '%0.6f' % ped)
                            ped = pedData[tem, row, end, fe, erng, 1]
                            p.setAttributeNS(None, 'sig', '%0.6f' % ped)
                            ped = pedData[tem, row, end, fe, erng, 2]
                            p.setAttributeNS(None, 'cos', '%0.6f' % ped)
                            f.appendChild(p)
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL Ped XML file
        
        Returns: A numarray array containing the pedestal data
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

        pedData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, calConstant.NUM_RNG, 3), numarray.Float32)

        # find <tower> elements
 
        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    for f in x.xpath('.//face'):

                        face = f.getAttributeNS(None, 'end')
                        end = _POSNEG_MAP[face]
        
                        # find <calPed> elements

                        for p in f.xpath('.//calPed'):

                            erng = _ERNG_MAP[p.getAttributeNS(None, 'range')]
                            pd = pedData[tem, row, end, fe, erng, :]

                            pd[0] = float(p.getAttributeNS(None, 'avg')) 
                            pd[1] = float(p.getAttributeNS(None, 'sig'))
                            pd[2] = float(p.getAttributeNS(None, 'cos'))
                                                 
        return pedData
        
        
        
class calMuSlopeCalibXML(calCalibXML):
    """
    CAL MuSlope calibration XML file class.

    This class provides methods for accessing CAL simplified gain
    calibration data stored in XML format.
    """
        

    def write(self, slopeData, tems = (0,)):
        """
        Write data to a CAL MuSlope XML file

        Param: slopeData -
            A numarray array containing the simplified gain data
            of shape (16, 8, 2, 12, 4, 2) The last dimension contains
            the following data for each crystal end and energy
            range:
                0 = slope value
                1 = error value
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElementNS(None, 'calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_MuSlope')
        r.appendChild(g)

        # insert <dimension> element  
            
        d = self.dimensionWrite(nRange = 4, nFace = 2)
        r.appendChild(d)

        for tem in tems:
            
            # insert <tower> elements

            (iCol, iRow) = temToTower(tem)
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                # translate index

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)

                    for end in range(2):

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        # insert <muSlope> elements

                        for erng in range(calConstant.NUM_RNG):

                            m = doc.createElementNS(None, 'muSlope')
                            m.setAttributeNS(None, 'range', calConstant.CRNG[erng])
                            slope = slopeData[tem, row, end, fe, erng, 0]
                            m.setAttributeNS(None, 'slope', '%0.6f' % slope)
                            error = slopeData[tem, row, end, fe, erng, 1]
                            m.setAttributeNS(None, 'error', '%0.6f' % error)
                            f.appendChild(m)
        
        # write output XML file

        self.writeFile()


    def read(self):
        """
        Read data from a CAL MuSlope XML file
        
        Returns: A numarray array containing the simplified gain data
                 of shape (16, 8, 2, 12, 4, 2) The last dimension contains
                 the following data for each crystal end and energy
                 range:
                     0 = slope value
                     1 = error value
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_MuSlope':
            raise calFileReadExcept, "XML file is type %s (expected CAL_MuSlope)" % type

        # create empty data array

        slopeData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, calConstant.NUM_RNG, 2), numarray.Float32)

        # find <tower> elements
 
        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements
 
            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    for f in x.xpath('.//face'):

                        face = f.getAttributeNS(None, 'end')
                        end = _POSNEG_MAP[face]
        
                        # find <muSlope> elements
                        
                        for m in f.xpath('.//muSlope'):

                            erng = _ERNG_MAP[m.getAttributeNS(None, 'range')]
                            sd = slopeData[tem, row, end, fe, erng, :]

                            sd[0] = float(m.getAttributeNS(None, 'slope'))
                            sd[1] = float(m.getAttributeNS(None, 'error'))
                                   
        return slopeData                   



class calDacSlopesCalibXML(calCalibXML):
    """
    CAL DacSlopes calibration XML file class.

    This class provides methods for accessing CAL DAC/energy
    calibration data stored in XML format.
    """

    # array indices
    DACDATA_LACDAC_SLOPE = 0
    DACDATA_LACDAC_OFFSET = 1
    DACDATA_FLEDAC_SLOPE = 2
    DACDATA_FLEDAC_OFFSET = 3
    DACDATA_FHEDAC_SLOPE = 4
    DACDATA_FHEDAC_OFFSET = 5
    DACDATA_LACDAC_SLOPE_ERR = 6
    DACDATA_LACDAC_OFFSET_ERR = 7
    DACDATA_FLEDAC_SLOPE_ERR = 8
    DACDATA_FLEDAC_OFFSET_ERR = 9
    DACDATA_FHEDAC_SLOPE_ERR = 10
    DACDATA_FHEDAC_OFFSET_ERR = 11

    ULDDATA_SLOPE = 0
    ULDDATA_OFFSET = 1
    ULDDATA_SAT = 2
    ULDDATA_SLOPE_ERR = 3
    ULDDATA_OFFSET_ERR = 4
    ULDDATA_SAT_ERR = 5

    RNGDATA_LACDAC = 0
    RNGDATA_FLEDAC = 1
    RNGDATA_FHEDAC = 2
    RNGDATA_ULDDAC_LEX8 = 3
    RNGDATA_ULDDAC_LEX1 = 4
    RNGDATA_ULDDAC_HEX8 = 5


    def write(self, dacData, uldData, rangeData, tems = (0,)):
        """
        Write data to a CAL DacSlopes XML file
        Param: dacData - A numarray array of shape (16, 8, 2, 12, 12).  The last
               dimension holds the LAC, FLE, and FHE values for the channel:
                0  = LAC DAC/energy slope
                1  = LAC energy offset
                2  = FLE DAC/energy slope
                3  = FLE energy offset
                4  = FHE DAC/energy slope
                5  = FHE energy offset
                6  = LAC DAC/energy slope error
                7  = LAC energy offset error
                8  = FLE DAC/energy slope error
                9  = FLE energy offset error
                10 = FHE DAC/energy slope error
                11 = FHE energy offset error
                
        Param: uldData - A numarray array of shape (3, 16, 8, 2, 12, 6).  The last
               dimension holds the ULD values for each energy range:
                0 = ULD DAC/energy slope
                1 = ULD energy offset
                2 = ULD energy saturation
                3 = ULD DAC/energy slope error 
                4 = ULD energy offset error
                5 = ULD energy saturation error
                
        Param: rangeData - A numarray array of shape (16, 8, 2, 12, 6).  The last
               dimension holds the DAC range (0=FINE, 1=COARSE) for each DAC:
                0 = LAC DAC range
                1 = FLE DAC range
                2 = FHE DAC range
                3 = LEX8 ULD DAC range
                4 = LEX1 ULD DAC range
                5 = HEX8 ULD DAC range
                
        Param: tems A list of TEM ID's to write out.
        """

        doc = self.getDoc()        

        # insert root document element <calCalib>

        r = doc.createElementNS(None, 'calCalib')
        doc.appendChild(r)

        # insert <generic> element

        g = self.genericWrite('CAL_DacSlopes')
        r.appendChild(g)

        # insert <dimension> element  

        d = self.dimensionWrite(nRange = 3)            
        r.appendChild(d)

        for tem in tems:

            # insert <tower> element

            (iCol, iRow) = temToTower(tem)
            t = doc.createElementNS(None, 'tower')
            t.setAttributeNS(None, 'iRow', str(iRow))
            t.setAttributeNS(None, 'iCol', str(iCol))
            c = doc.createComment('tem number = %d, module = %s' % (tem, calConstant.CMOD[tem]))
            t.appendChild(c)
            r.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):

                row = layerToRow(layer) 

                # insert <layer> elements

                l = doc.createElementNS(None, 'layer')
                l.setAttributeNS(None, 'iLayer', str(layer))
                t.appendChild(l)

                c = doc.createComment('layer name = %s' % calConstant.CROW[row])
                l.appendChild(c)
                    
                for fe in range(calConstant.NUM_FE):

                    # insert <xtal> elements

                    x = doc.createElementNS(None, 'xtal')
                    x.setAttributeNS(None, 'iXtal', str(fe))
                    l.appendChild(x)
                        
                    for end in range(calConstant.NUM_END):                      

                        # insert <face> elements

                        f = doc.createElementNS(None, 'face')
                        f.setAttributeNS(None, 'end', _POSNEG[end])
                        x.appendChild(f)

                        # insert <dacSlopes> element

                        ds = doc.createElementNS(None, 'dacSlopes')
                        
                        dsVal = dacData[tem,row,end,fe,0]
                        ds.setAttributeNS(None, 'LACSlopeVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,1]
                        ds.setAttributeNS(None, 'LACOffsetVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,2]
                        ds.setAttributeNS(None, 'FLESlopeVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,3]
                        ds.setAttributeNS(None, 'FLEOffsetVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,4]
                        ds.setAttributeNS(None, 'FHESlopeVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,5]
                        ds.setAttributeNS(None, 'FHEOffsetVal', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,6]
                        ds.setAttributeNS(None, 'LACSlopeErr', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,7]
                        ds.setAttributeNS(None, 'LACOffsetErr', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,8]
                        ds.setAttributeNS(None, 'FLESlopeErr', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,9]
                        ds.setAttributeNS(None, 'FLEOffsetErr', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,10]
                        ds.setAttributeNS(None, 'FHESlopeErr', "%.3f" % dsVal)
                        dsVal = dacData[tem,row,end,fe,11]
                        ds.setAttributeNS(None, 'FHEOffsetErr', "%.3f" % dsVal)
                        dsVal = int(rangeData[tem,row,end,fe,0])
                        ds.setAttributeNS(None, 'LACRange', calConstant.CDAC[dsVal])
                        dsVal = int(rangeData[tem,row,end,fe,1])
                        ds.setAttributeNS(None, 'FLERange', calConstant.CDAC[dsVal])
                        dsVal = int(rangeData[tem,row,end,fe,2])
                        ds.setAttributeNS(None, 'FHERange', calConstant.CDAC[dsVal]) 
                        f.appendChild(ds)
                        
                        for erng in range(3):
                        
                            # insert <dacSlopesRange> element
                        
                            dsr = doc.createElementNS(None, 'dacSlopesRange')
                            dsr.setAttributeNS(None, 'range', calConstant.CRNG[erng])
                            dsrVal = uldData[erng,tem,row,end,fe,0]
                            dsr.setAttributeNS(None, 'ULDSlopeVal', "%.3f" % dsrVal)
                            dsrVal = uldData[erng,tem,row,end,fe,1]
                            dsr.setAttributeNS(None, 'ULDOffsetVal', "%.3f" % dsrVal)
                            dsrVal = uldData[erng,tem,row,end,fe,2]
                            dsr.setAttributeNS(None, 'ULDSatVal', "%.3f" % dsrVal)
                            dsrVal = uldData[erng,tem,row,end,fe,3]
                            dsr.setAttributeNS(None, 'ULDSlopeErr', "%.3f" % dsrVal)
                            dsrVal = uldData[erng,tem,row,end,fe,4]
                            dsr.setAttributeNS(None, 'ULDOffsetErr', "%.3f" % dsrVal)
                            dsrVal = uldData[erng,tem,row,end,fe,5]
                            dsr.setAttributeNS(None, 'ULDSatErr', "%.3f" % dsrVal)
                            dsrVal = int(rangeData[tem,row,end,fe,erng+3])
                            dsr.setAttributeNS(None, 'ULDRange', calConstant.CDAC[dsrVal])
                            ds.appendChild(dsr)
                        

        # write output XML file

        self.writeFile()
        
        
    def read(self):
        """
        Read data from a CAL DacSlopes XML file.
        Returns: A tuple of references to numarray arrays and containing the
        calibration data (dacData, uldData, rangeData):
        
            dacData - A numarray array of shape (16, 8, 2, 12, 12).  The last
            dimension holds the LAC, FLE, and FHE values for the channel:
                0  = LAC DAC/energy slope
                1  = LAC energy offset
                2  = FLE DAC/energy slope
                3  = FLE energy offset
                4  = FHE DAC/energy slope
                5  = FHE energy offset
                6  = LAC DAC/energy slope error
                7  = LAC energy offset error
                8  = FLE DAC/energy slope error
                9  = FLE energy offset error
                10 = FHE DAC/energy slope error
                11 = FHE energy offset error
                
            uldData - A numarray array of shape (3, 16, 8, 2, 12, 6).  The last
            dimension holds the ULD values for each energy range:
                0 = ULD DAC/energy slope
                1 = ULD energy offset
                2 = ULD energy saturation
                3 = ULD DAC/energy slope error 
                4 = ULD energy offset error
                5 = ULD energy saturation error
                
            rangeData - A numarray array of shape (16, 8, 2, 12, 6).  The last
            dimension holds the DAC range (0=FINE, 1=COARSE) for each DAC:
                0 = LAC DAC range
                1 = FLE DAC range
                2 = FHE DAC range
                3 = LEX8 ULD DAC range
                4 = LEX1 ULD DAC range
                5 = HEX8 ULD DAC range     
        """
        
        # verify file type

        type =  self.getType()
        if type != 'CAL_DacSlopes':
            raise calFileReadExcept, "XML file is type %s (expected CAL_DacSlopes)" % type
            
        # create empty data arrays
        
        dacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW,
            calConstant.NUM_END, calConstant.NUM_FE, 12), numarray.Float32)
        uldData = numarray.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW,
            calConstant.NUM_END, calConstant.NUM_FE, 6), numarray.Float32)
        rangeData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW,
            calConstant.NUM_END, calConstant.NUM_FE, 6), numarray.Int16)

        # find <tower> elements
 
        for t in self.getDoc().xpath('.//tower'):

            tRow = int(t.getAttributeNS(None, 'iRow'))
            tCol = int(t.getAttributeNS(None, 'iCol'))
            tem = towerToTem(tCol, tRow)

            # find <layer> elements

            for l in t.xpath('.//layer'):

                layer = int(l.getAttributeNS(None, 'iLayer'))
                row = layerToRow(layer)

                # find <xtal> elements

                for x in l.xpath('.//xtal'):

                    fe = int(x.getAttributeNS(None, 'iXtal'))

                    # find <face> elements

                    for f in x.xpath('.//face'):

                        face = f.getAttributeNS(None, 'end')
                        end = _POSNEG_MAP[face]
        
                        # find <dacSlopes> elements

                        ds = f.xpath('.//dacSlopes')[0]
                        dd = dacData[tem, row, end, fe, :]
                        rd = rangeData[tem, row, end, fe, :]
                         
                        dd[0] = float(ds.getAttributeNS(None, 'LACSlopeVal'))
                        dd[1] = float(ds.getAttributeNS(None, 'LACOffsetVal'))
                        dd[2] = float(ds.getAttributeNS(None, 'FLESlopeVal'))
                        dd[3] = float(ds.getAttributeNS(None, 'FLEOffsetVal'))
                        dd[4] = float(ds.getAttributeNS(None, 'FHESlopeVal'))
                        dd[5] = float(ds.getAttributeNS(None, 'FHEOffsetVal'))
                        dd[6] = float(ds.getAttributeNS(None, 'LACSlopeErr'))
                        dd[7] = float(ds.getAttributeNS(None, 'LACOffsetErr'))
                        dd[8] = float(ds.getAttributeNS(None, 'FLESlopeErr'))
                        dd[9] = float(ds.getAttributeNS(None, 'FLEOffsetErr'))
                        dd[10] = float(ds.getAttributeNS(None, 'FHESlopeErr'))
                        dd[11] = float(ds.getAttributeNS(None, 'FHEOffsetErr'))
                        
                        rd[0] = _DRNG_MAP[str(ds.getAttributeNS(None, 'LACRange'))]
                        rd[1] = _DRNG_MAP[str(ds.getAttributeNS(None, 'FLERange'))]
                        rd[2] = _DRNG_MAP[str(ds.getAttributeNS(None, 'FHERange'))]

                        # find <dacSlopesRange> elements

                        for dsr in ds.xpath('.//dacSlopesRange'):
                            
                            erngName = dsr.getAttributeNS(None, 'range')
                            erng = _ERNG_MAP[erngName]
                            ud = uldData[erng, tem, row, end, fe, :]

                            ud[0] = float(dsr.getAttributeNS(None, 'ULDSlopeVal'))
                            ud[1] = float(dsr.getAttributeNS(None, 'ULDOffsetVal'))
                            ud[2] = float(dsr.getAttributeNS(None, 'ULDSatVal'))
                            ud[3] = float(dsr.getAttributeNS(None, 'ULDSlopeErr'))
                            ud[4] = float(dsr.getAttributeNS(None, 'ULDOffsetErr'))
                            ud[5] = float(dsr.getAttributeNS(None, 'ULDSatErr'))
                            
                            rd[erng+3] = _DRNG_MAP[str(dsr.getAttributeNS(None, 'ULDRange'))]
                               
                                        
        return (dacData, uldData, rangeData)
        
        
        
        
def layerToRow(layer):
    """
    Translate CAL layer number to CAL row number
    Param: layer - Layer number (0 - 7)
    Returns: The row number (0 - 7)
    """
    
    if (layer < 0) or (layer > 7):
        raise ValueError, "layer parameter limited to range [0,7]"

    row = (layer / 2)
    if (layer % 2):
        row += 4
    return row



def rowToLayer(row):
    """
    Translate CAL row number to CAL layer number
    Param: row - Row number (0 - 7)
    Returns: The layer number (0 - 7)
    """
    
    if (row < 0) or (row > 7):
        raise ValueError, "row parameter limited to range [0,7]"

    if row < 4:
        return (row * 2)
    else:
        return (((row - 4) * 2) + 1)

    
    
def towerToTem(twrCol, twrRow):
    """
    Translate tower row and column indicies to TEM number.

    Param: twrCol - The tower column index (0 - 3)
    Param: twrRow - The tower row index (0 - 3)

    Returns: The TEM number (0 - 15)
    """

    if (twrCol < 0) or (twrCol > 3):
        raise ValueError, "twrCol parameter limited to range [0,3]"
    if (twrRow < 0) or (twrRow > 3):
        raise ValueError, "twrRow parameter limited to range [0,3]"

    return (twrCol + (4 * twrRow))



def temToTower(temNum):
    """
    Translate tower row and column indicies to TEM number.

    Param: temNum - The TEM number (0 - 15)

    Returns: A tuple (twrCol, twrRow):
        twrCol - The tower column index (0 - 3)
        twrRow - The tower row index (0 - 3)
    """
    
    if (temNum < 0) or (temNum > 15):
        raise ValueError, "temNum parameter limited to range [0,15]"
 
    return ((temNum % 4), (temNum / 4))



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

    
