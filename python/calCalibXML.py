"""
calCalibXML
\brief Classes to represent CAL calibration XML documents.
"""


import sys, time
import logging

import Numeric

import xml.dom.minidom
import xml.dom.ext

import calConstant
from calExcept import *


MODE_CREATE     = 0
MODE_READONLY   = 2


POSNEG = ('NEG', 'POS')



##########################################################################################
##
## \class calCalibXML
##
## \brief CAL calibration XML file class.
##
## This class provides methods for accessing CAL calibration data stored in XML
## format.
##
##########################################################################################

class calCalibXML:   
  
    def __init__(self, fileName, mode = MODE_READONLY):
        """
        \brief Open a CAL DAC configuration XML file

        \param fileName The XML file name.
        \param type The type of CAL calibration XML file.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        self.__log = logging.getLogger()    

        self.__doc = None
        self.__xmlFile = None
        self.__mode = mode

        if mode == MODE_CREATE:

            # create ouput XML file

            self.__xmlFile = open(fileName, 'w')            
            
            # create output XML document

            impl = xml.dom.minidom.getDOMImplementation()
            self.__doc = impl.createDocument(None, None, None)

        elif mode == MODE_READONLY:

            # open input XML file

            self.__xmlFile = open(fileName, "r")

            # parse into DOM document

            self.__doc = xml.dom.minidom.parse(self.__xmlFile)

        else:
            raise calFileOpenExcept, "calCalibXML: mode %s not supported" % str(mode)            
        
                
    def close(self):              
        """
        \brief Close a CAL calibration XML file
        """
        
        self.__xmlFile.close()
        self.__doc.unlink()
    

    def getDoc(self):
        """
        \brief Get the XML DOM document object.

        \returns A reference to the DOM document.
        """

        return self.__doc
    

    def getFile(self):
        """
        \brief Get the XML file object.

        \returns A reference to the XML file handle.
        """

        return self.__xmlFile    
    

    def getMode(self):
        """
        \brief Get the XML file access mode.

        \returns MODE_CREATE or MODE_READONLY.
        """

        return self.__mode


    def getLog(self):
        """
        \brief Get the CAL calibration logger object.

        \returns A reference to a logging object.
        """

        return self.__log
    


##########################################################################################
##
## \class calTholdCICalibXML
##
## \brief CAL Threshold CI calibration XML file class.
##
## This class provides methods for accessing CAL threshold charge injection calibration
## data stored in XML format.
##
##########################################################################################
    
class calTholdCICalibXML(calCalibXML):


    def __init__(self, fileName, mode = MODE_READONLY):

        calCalibXML.__init__(self, fileName, mode)
        

    def write(self, dacData, adcData, pedData, lrefGain, hrefGain):
        """
        \brief Write data to a CAL calibration XML file

        \param dacData A list of Numeric arrays of DAC settings data (16, 8, 2, 12): \n
            dacData[0] = ULD DAC settings data \n
            dacData[1] = LAC DAC settings data \n
            dacData[2] = FLE DAC settings data \n
            dacData[3] = FHE DAC settings data \n
        \param adcData A list of Numeric arrays of ADC/DAC characterization data. \n
            adcData[0] = A Numeric array of ULD ADC/DAC characterization data (3, 16, 8, 2, 12, 128). \n
            adcData[1] = A Numeric array of LAC ADC/DAC characterization data (16, 8, 2, 12, 128). \n
            adcData[2] = A Numeric array of FLE ADC/DAC characterization data (16, 8, 2, 12, 128). \n
            adcData[3] = A Numeric array of FHE ADC/DAC characterization data (16, 8, 2, 12, 128). \n
        \param pedData A Numeric array of pedestal value data (9, 4, 8, 2, 12).
        \param lrefGain The LE gain value.
        \param hrefGain The HE gain value.
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
        g.setAttribute('creator', '$Name:    $')
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
        d.setAttribute('nRange', '1')
        r.appendChild(d)

        # insert <tower> element

        t = doc.createElement('tower')
        t.setAttribute('iRow', '0')
        t.setAttribute('iCol', '0')
        r.appendChild(t)
        
        for layer in range(8):

            # translate index

            row = (layer / 2)
            xy = (layer % 2)
            if xy == 1:
                row += 4 

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
                    
                    dac = int(lacDac[0, row, end, fe])
                    adc = lacAdc[0, row, end, fe, dac]
                    c = doc.createComment('LAC DAC = %d' % dac)
                    tc.appendChild(c)
                    tc.setAttribute('LACVal', "%0.3f" % adc)
                    tc.setAttribute('LACSig', '1')
                    
                    dac = int(fleDac[0, row, end, fe])
                    adc = fleAdc[0, row, end, fe, dac]
                    c = doc.createComment('FLE DAC = %d' % dac)
                    tc.appendChild(c)
                    tc.setAttribute('FLEVal', "%0.3f" % adc)
                    tc.setAttribute('FLESig', '1')

                    dac = int(fheDac[0, row, end, fe])
                    adc = fheAdc[0, row, end, fe, dac]
                    c = doc.createComment('FHE DAC = %d' % dac)
                    tc.appendChild(c)
                    tc.setAttribute('FHEVal', "%0.3f" % adc)
                    tc.setAttribute('FHESig', '1')                     
                    
                    f.appendChild(tc)

                    for erng in range(3):

                        if erng == 0:
                            gain = lrefGain
                        else:
                            gain = (hrefGain - 8)

                        # insert <tholdCIRange> elements

                        tcr = doc.createElement('tholdCIRange')
                        
                        tcr.setAttribute('range', calConstant.CRNG[erng])
                        
                        dac = int(uldDac[0, row, end, fe])
                        adc = uldAdc[erng, 0, row, end, fe, dac]
                        c = doc.createComment('ULD DAC = %d' % dac)
                        tcr.appendChild(c)
                        tcr.setAttribute('ULDVal', "%0.3f" % adc)
                        tcr.setAttribute('ULDSig', '30')

                        ped = pedData[gain, erng, row, end, fe]                        
                        tcr.setAttribute('PEDVal', "%0.3f" % ped)
                        tcr.setAttribute('PEDSig', '1')
                        
                        tc.appendChild(tcr)

                    # insert fake <tholdCIRange> element for HEX1 range

                    tcr = doc.createElement('tholdCIRange')
                    
                    tcr.setAttribute('range', 'HEX1')
                    
                    tcr.setAttribute('ULDVal', '4095.000')
                    tcr.setAttribute('ULDSig', '30')
                    
                    ped = pedData[lrefGain, 3, row, end, fe]
                    tcr.setAttribute('PEDVal', "%0.3f" % ped)
                    tcr.setAttribute('PEDSig', '1')
                    
                    tc.appendChild(tcr)
                    

        # write output XML file

        xml.dom.ext.PrettyPrint(doc, self.getFile())


    def read(self):
        """
        \brief Read data from a CAL calibration XML file

        \returns A tuple of references to Numeric arrays and containing the calibration data: \n
        """
        
        doc = self.getDoc()

        # find <tower> elements

        tList = doc.getElementsByTagName('tower')
        for t in tList:

            tRow = int(t.getAttribute('iRow'))
            tCol = int(t.getAttribute('iCol'))
            print "tower: %d %d" % (tRow, tCol)
        
        # find <tholdCI> elements

        
        
            
    
    
    
    
