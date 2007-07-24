"""
Classes to represent CAL hardware settings XML documents.
"""


__facility__  = "Offline"
__abstract__  = "Classes to represent CAL DAC settings XML documents"
__author__    = "D.L.Wood"
__date__      = "$Date: 2007/03/21 20:10:31 $"
__version__   = "$Revision: 1.6 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import time

import Numeric

import calXML
import calConstant
from calExcept import *



MODE_CREATE     = calXML.MODE_CREATE
MODE_READONLY   = calXML.MODE_READONLY


DAC_FILE_TYPES = ('fle_dac', 'fhe_dac', 'log_acpt', 'rng_uld_dac', 'config_0')
ENG_FILE_TYPES = ('thrBias', 'adc2nrg')




class calSnapshotXML(calXML.calXML):
    """
    CAL hardware configuration snapshot XML data file class.
    
    This class provides methods for accessing CAL HW configuration
    data stored in XML snapshot file format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL HW snapshot configuration XML file

        \param fileName The XML file name.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """
        
        calXML.calXML.__init__(self, fileName, mode)       


    def getTowers(self):
        """
        Get the ID's of towers contributing to the data file.

        Returns: A list of tower ID's.        
        """

        towers = []

        # find <GTEM> elements

        doc = self.getDoc()

        latList = doc.xpath('.//GLAT')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <GLAT> elements, expected 1" % latLen
        temList = latList[0].xpath('.//GTEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <GTEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<GTEM> ID attribute value %d, expected (0 - 15)" % tem
            towers.append(tem)

        return towers            

        
    def read(self, dacName):
        """
        Read CAL data from a snapshot XML file

        Param: dacName - The name of the HW configuration element to read.

        Returns: A Numeric array of data (16, 8, 2, 12) read from the
        XML file.
        """

        # create empty DAC data array

        dacData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE), Numeric.UInt8)

        # find <fle_dac> elements

        doc = self.getDoc()

        latList = doc.xpath('.//GLAT')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <GLAT> elements, expected 1" % latLen
        temList = latList[0].xpath('.//GTEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <GTEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<GTEM> ID attribute value %d, expected (0 - 15)" % tem
            cccList = t.xpath('.//GCCC')
            cccLen = len(cccList)
            if cccLen > 4:
                raise calFileReadExcept, "found %d <GCCC> elements, expected <= 4" % cccLen
            for c in cccList:
                ccc = int(c.getAttributeNS(None, 'ID'))
                if ccc < 0 or ccc > 3:
                    raise calFileReadExcept, "<GCCC> ID attribute value %d, expected (0 - 3)" % ccc
                rcList = c.xpath('.//GCRC')
                rcLen = len(rcList)
                if rcLen > 4:
                    raise calFileReadExcept, "found %d <GCRC> elements, expected <= 4" % rcLen
                for r in rcList:
                    rc = int(r.getAttributeNS(None, 'ID'))
                    if rc < 0 or rc > 3:
                        raise calFileReadExcept, "<GCRC> ID attribute value %d, expected (0 - 3)" % rc
                    feList = r.xpath('.//GCFE')
                    feLen = len(feList)
                    if feLen > 12:
                        raise calFileReadExcept, "found %d <GCFE> elements, expected <= 12" % feLen
                    for f in feList:
                        fe = int(f.getAttributeNS(None, 'ID'))
                        if fe < 0 or fe > 11:
                            raise calFileReadExcept, "<GCFE> ID attribute value %d, expected (0 - 11)" % fe
                        dacList = f.xpath('.//%s' % dacName)
                        dacLen = len(dacList)
                        if dacLen != 1:
                            if dacLen == 0:
                                continue
                            else:
                                raise calFileReadExcept, "found %d %s elements, expected 1" % (dacLen, dacName)
                        d = dacList[0]
                        dd = d.childNodes[0]
                        dac = int(dd.data.strip(), 16)
                        (row, end) = ccToRow(ccc, rc)
                        dacData[tem, row, end, fe] = dac

        return dacData

    

class calDacXML(calSnapshotXML):   
    """
    CAL DAC configuration XML data file class.
    
    This class provides methods for accessing CAL DAC configuration
    data stored in XML format.  These files are fragments of
    full HW snapshot records.

    The following file types are supported:

        fle_dac - CAL FLE DAC setttings
        fhe_dac - CAL FHE DAC settings
        log_acpt - CAL LAC DAC settings
        rng_uld_dac - CAL ULD DAC settings
    """

    def __init__(self, fileName, dacName, mode = MODE_READONLY):
        """
        Open a CAL DAC configuration XML file

        Param: fileName - The XML file name.
        Param: dacName - The name of the bottom level XML data element.
        Param: mode - The file access mode (MODE_READONLY or MODE_CREATE).
        """

        calSnapshotXML.__init__(self, fileName, mode)  

        if dacName not in DAC_FILE_TYPES:
            raise calFileOpenExcept, "DAC name %s not supported" % str(dacName)
        self.__dacName = dacName


    def read(self):
        """
        Read data from a CAL DAC configuration XML file

        Returns: A Numeric array of data (16, 8, 2, 12) read from the
        XML file.
        """

        return calSnapshotXML.read(self, self.__dacName)


    def write(self, dacData, filename = None, cfgfilename = None, leGain = None, heGain = None,
              energy = None, adcfilename = None, relgainfilename = None, engfilename = None,
              biasfilename = None, adcmargin = None, method = None, tems = (0,)):
        """
        Write CAL data to a snapshot fragment XML file

        Param: dacData - A Numeric array of data (16, 8, 2, 12) to write to the
        XML file.
        Param: filename - <configuration> element filename value
        Param: cfgfilename - <configuration> element cfgfilename value 
        Param: leGain - <configuration> element heGain value
        Param: heGain - <configuration> element heGain value
        Param: energy - <configuration> element heGain value
        Param: adcfilename - <configuration> element adcfilename value
        Param: relgainfilename - <configuration> element relgainfilename value
        Param: engfilename = <configuration> element engfilename value
        Param: biasfilename = <configuration> element engfilename value
        Param: adcmargin = <configuration> element adcmargin value
        Param: method = <configuration> element method value
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()

        # insert <configuration> element
        # this is the root element of the XML output document

        ce = doc.createElementNS(None, 'configuration')
        
        s = '[\'GCCC\',\'GCRC\',\'GCFE\',\'%s\']' % self.__dacName
        ce.setAttributeNS(None, 'name', 'NA')
        ce.setAttributeNS(None, 'hierarchy', s)
        ce.setAttributeNS(None, 'type', 's')
        ce.setAttributeNS(None, 'shape', '(8,2,12)')
        ce.setAttributeNS(None, 'version', 'NA')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        ce.setAttributeNS(None, 'time', ts)

        if filename is not None:
            ce.setAttributeNS(None, 'filename', filename)

        if cfgfilename is not None:
            ce.setAttributeNS(None, 'cfgfilename', cfgfilename)
        
        if leGain is not None:
            ce.setAttributeNS(None, 'leGain', str(leGain))
            
        if heGain is not None:
            ce.setAttributeNS(None, 'heGain', str(heGain))

        if energy is not None:
            ce.setAttributeNS(None, 'energy', "%s MeV" % str(energy))

        if adcfilename is not None:
            ce.setAttributeNS(None, 'adcfilename', adcfilename)
            
        if relgainfilename is not None:
            ce.setAttributeNS(None, 'relgainfilename', relgainfilename)
            
        if engfilename is not None:
            ce.setAttributeNS(None, 'engfilename', engfilename)
            
        if biasfilename is not None:
            ce.setAttributeNS(None, 'biasfilename', biasfilename)

        if adcmargin is not None:
            ce.setAttributeNS(None, 'adcmargin', str(adcmargin))

        if method is not None:
            ce.setAttributeNS(None, 'method', method)            
            
        doc.appendChild(ce)
        
        # insert <GLAT> element  
            
        gl = doc.createElementNS(None, 'GLAT')
        ce.appendChild(gl)     

        for tem in tems:
                        
            # insert <GTEM> elements

            gt = doc.createElementNS(None, 'GTEM')
            gt.setAttributeNS(None, 'ID', str(tem))
            gl.appendChild(gt)
            
            c = doc.createComment("module = %s" % calConstant.CMOD[tem])
            gt.appendChild(c)
            
            for ccc in range(4):

                # insert <GCCC> elements

                gc = doc.createElementNS(None, 'GCCC')
                gc.setAttributeNS(None, 'ID', str(ccc))
                gt.appendChild(gc)                

                for rc in range(4):

                    # insert <GCRC> elements

                    gr = doc.createElementNS(None, 'GCRC')
                    gr.setAttributeNS(None, 'ID', str(rc))
                    gc.appendChild(gr)
                                    
                    # translate index

                    (row, end) = ccToRow(ccc, rc) 
                    c = doc.createComment("layer = %s%s " % (calConstant.CROW[row], calConstant.CPM[end]))
                    gr.appendChild(c) 
                    
                    for fe in range(12):

                        # insert <GCFE> elements

                        gf = doc.createElementNS(None, 'GCFE')
                        gf.setAttributeNS(None, 'ID', str(fe))
                        gr.appendChild(gf)

                        # insert <xxx> elements

                        dv = doc.createElementNS(None, self.__dacName)
                        t = doc.createTextNode('0x%x' % int(dacData[tem, row, end, fe]))
                        dv.appendChild(t)
                        gf.appendChild(dv)
                        
        
        # write output XML file

        self.writeFile()



class calEnergyXML(calXML.calXML):
    """
    CAL ADC to energy conversion data.
    
    This class provides methods for accessing CAL ADC to energy
    data stored in XML pseudo-snapshot file format.

    The following file types are supported:
    
        thrBias - CAL threshold bias values
        adc2nrg - CAL ADC to energy conversion values
    """

    def __init__(self, fileName, engName, mode = MODE_READONLY):
        """
        Open a CAL energy configuration XML file

        Param: fileName - The XML file name.
        Param: engName - The name of the bottom level XML data element.
        Param: mode - The file access mode (MODE_READONLY or MODE_CREATE).
        """

        calXML.calXML.__init__(self, fileName, mode)  

        if engName not in ENG_FILE_TYPES:
            raise calFileOpenExcept, "ENG name %s not supported" % str(engName)
        self.__engName = engName


    def getTowers(self):
        """
        Get the ID's of towers contributing to the data file.

        Returns: A list of tower ID's.        
        """

        towers = []

        # find <GTEM> elements

        doc = self.getDoc()

        latList = doc.xpath('.//GLAT')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <GLAT> elements, expected 1" % latLen
        temList = latList[0].xpath('.//GTEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <GTEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<GTEM> ID attribute value %d, expected (0 - 15)" % tem
            towers.append(tem)

        return towers                    

        
    def read(self):
        """
        Read CAL data from a energy/bias XML file

        Returns: A Numeric array of data (16, 8, 2, 12, 2) read from the
        XML file.  The last dimension is as follows:
            0 = LE conversion value
            1 = HE conversion value
        """

        doc = self.getDoc()        

        # create empty energy data array

        engData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, 
            calConstant.NUM_FE, 2), Numeric.Float32)

        # find elements

        latList = doc.xpath('.//GLAT')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <GLAT> elements, expected 1" % latLen
        temList = latList[0].xpath('.//GTEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <GTEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<GTEM> ID attribute value %d, expected (0 - 15)" % tem
            if self.__engName == 'adc2nrg':
                eName = 'low_hi_nrg'
            else:
                eName = 'fle_fhe'
            eList = t.xpath('.//%s' % eName)
            eLen = len(eList)
            if eLen != 2:
                raise calFileReadExcept, "found %d <%s> elements, expected 2" % (eLen, eName)
            for e in eList:
                eng = int(e.getAttributeNS(None, 'ID'))
                if eng < 0 or eng > 1:
                    raise calFileReadExcept, "%s ID attribute value %d, expected (0 - 1)" % (eName, eng)
                cccList = e.xpath('.//GCCC')
                cccLen = len(cccList)
                if cccLen > 4:
                    raise calFileReadExcept, "found %d <GCCC> elements, expected <= 4" % cccLen
                for c in cccList:
                    ccc = int(c.getAttributeNS(None, 'ID'))
                    if ccc < 0 or ccc > 3:
                        raise calFileReadExcept, "<GCCC> ID attribute value %d, expected (0 - 3)" % ccc
                    rcList = c.xpath('.//GCRC')
                    rcLen = len(rcList)
                    if rcLen > 4:
                        raise calFileReadExcept, "found %d <GCRC> elements, expected <= 4" % rcLen
                    for r in rcList:
                        rc = int(r.getAttributeNS(None, 'ID'))
                        if rc < 0 or rc > 3:
                            raise calFileReadExcept, "<GCRC> ID attribute value %d, expected (0 - 3)" % rc
                        feList = r.xpath('.//GCFE')
                        feLen = len(feList)
                        if feLen > 12:
                            raise calFileReadExcept, "found %d <GCFE> elements, expected <= 12" % feLen
                        for f in feList:
                            fe = int(f.getAttributeNS(None, 'ID'))
                            if fe < 0 or fe > 11:
                                raise calFileReadExcept, "<GCFE> ID attribute value %d, expected (0 - 11)" % fe
                            dacList = f.xpath('.//%s' % self.__engName)
                            dacLen = len(dacList)
                            if dacLen != 1:
                                raise calFileReadExcept, "found %d %s elements, expected 1" % (dacLen, self.__engName)
                            d = dacList[0]
                            dd = d.childNodes[0]
                            dac = float(dd.data.strip())
                            (row, end) = ccToRow(ccc, rc)
                            engData[tem, row, end, fe, eng] = dac

        return engData



    def write(self, data, tems = (0,)):
        """
        Write data to a CAL energy/bias XML file.
        Param: data - A Numeric array of data (16, 8, 2, 12, 2) to write to the
                      XML file.  The last dimension is as follows:
                          0 = LE conversion value
                          1 = HE conversion value
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()

        # insert <configuration> element
        # this is the root element of the XML output document

        ce = doc.createElementNS(None, 'configuration')

        if self.__engName == 'adc2nrg':
             eName = 'low_hi_nrg'
        else:
             eName = 'fle_fhe'

        s = '[\'%s\',\'GCCC\',\'GCRC\',\'GCFE\',\'%s\']' % (eName, self.__engName)
        ce.setAttributeNS(None, 'name', '')
        ce.setAttributeNS(None, 'hierarchy', s)
        ce.setAttributeNS(None, 'type', 's')
        ce.setAttributeNS(None, 'shape', '(2,16,8,2,12)')
        ce.setAttributeNS(None, 'version', 'NA')
        ts = time.strftime('%Y-%m-%d-%H:%M', time.gmtime())
        ce.setAttributeNS(None, 'time', ts)

        doc.appendChild(ce)

        # insert <GLAT> element

        gl = doc.createElementNS(None, 'GLAT')
        ce.appendChild(gl)

        for tem in tems:

            # insert <GTEM> elements

            gt = doc.createElementNS(None, 'GTEM')
            gt.setAttributeNS(None, 'ID', str(tem))
            gl.appendChild(gt)
            
            c = doc.createComment("module = %s" % calConstant.CMOD[tem])
            gt.appendChild(c)

            for rng in range(2): 

                # insert <low_hi_nrg> or <fle_fhe> element

                ee = doc.createElementNS(None, eName)
                ee.setAttributeNS(None, 'ID', str(rng))
                gt.appendChild(ee)

                for ccc in range(4):

                    # insert <GCCC> elements

                    gc = doc.createElementNS(None, 'GCCC')
                    gc.setAttributeNS(None, 'ID', str(ccc))
                    ee.appendChild(gc)

                    for rc in range(4):

                        # insert <GCRC> elements

                        gr = doc.createElementNS(None, 'GCRC')
                        gr.setAttributeNS(None, 'ID', str(rc))
                        gc.appendChild(gr)

                        # translate index

                        (row, end) = ccToRow(ccc, rc) 
                        c = doc.createComment("layer = %s%s " % (calConstant.CROW[row], calConstant.CPM[end]))
                        gr.appendChild(c)
                                                          
                        for fe in range(12):

                            # insert <GCFE> elements
            
                            gf = doc.createElementNS(None, 'GCFE')
                            gf.setAttributeNS(None, 'ID', str(fe))
                            gr.appendChild(gf)

                            # insert <xxx> elements

                            dv = doc.createElementNS(None, self.__engName)
                            t = doc.createTextNode('%0.3f' % data[tem, row, end, fe, rng])
                            dv.appendChild(t)
                            gf.appendChild(dv)
 

        # write output XML file

        self.writeFile()



class calSettingsXML(calXML.calXML):
    """
    CAL ConfigSystem settings XML data file class.
    
    This class provides methods for accessing CAL HW configuration
    data stored in XML file format produced for use with the ConfigSystem
    online tool.
    """

    def __init__(self, fileName, dacName, mode = MODE_READONLY):
        """
        Open a CAL configuration XML file

        Param: fileName - The XML file name.
        Param: mode - The file access mode (MODE_READONLY or MODE_CREATE).
        Param: dacName - The name of the DAC element.
        
        The following DAC types are supported:

            fle_dac - CAL FLE DAC setttings
            fhe_dac - CAL FHE DAC settings
            log_acpt - CAL LAC DAC settings
            rng_uld_dac - CAL ULD DAC settings
        """
        
        if dacName not in DAC_FILE_TYPES:
            raise calFileOpenExcept, "DAC name %s not supported" % str(dacName)
        self.__dacName = dacName
        
        calXML.calXML.__init__(self, fileName, mode)       


    def getTowers(self):
        """
        Get the ID's of towers contributing to the data file.

        Returns: A list of tower ID's.        
        """

        towers = []

        # find <GTEM> elements

        doc = self.getDoc()

        latList = doc.xpath('.//LATC_XML')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <LATC_XML> elements, expected 1" % latLen
        temList = latList[0].xpath('.//TEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <TEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<TEM> ID attribute value %d, expected (0 - 15)" % tem
            towers.append(tem)

        return towers            

        
    def read(self):
        """
        Read CAL DAC data from a ConfigSystem settings XML file

        Returns: A Numeric array of data (16, 8, 2, 12) read from the
        XML file.
        """

        # create empty DAC data array

        dacData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE), Numeric.UInt8)

        # find <fle_dac> elements

        doc = self.getDoc()

        latList = doc.xpath('.//LATC_XML')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <LATC_XML> elements, expected 1" % latLen
        temList = latList[0].xpath('.//TEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <TEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttributeNS(None, 'ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<TEM> ID attribute value %d, expected (0 - 15)" % tem
            cccList = t.xpath('.//CCC')
            cccLen = len(cccList)
            if cccLen > 4:
                raise calFileReadExcept, "found %d <CCC> elements, expected <= 4" % cccLen
            for c in cccList:
                ccc = int(c.getAttributeNS(None, 'ID'))
                if ccc < 0 or ccc > 3:
                    raise calFileReadExcept, "<CCC> ID attribute value %d, expected (0 - 3)" % ccc
                rcList = c.xpath('.//CRC')
                rcLen = len(rcList)
                if rcLen > 4:
                    raise calFileReadExcept, "found %d <CRC> elements, expected <= 4" % rcLen
                for r in rcList:
                    rc = int(r.getAttributeNS(None, 'ID'))
                    if rc < 0 or rc > 3:
                        raise calFileReadExcept, "<CRC> ID attribute value %d, expected (0 - 3)" % rc
                    feList = r.xpath('.//CFE')
                    feLen = len(feList)
                    if feLen > 12:
                        raise calFileReadExcept, "found %d <CFE> elements, expected <= 12" % feLen
                    for f in feList:
                        fe = int(f.getAttributeNS(None, 'ID'))
                        if fe < 0 or fe > 11:
                            raise calFileReadExcept, "<CFE> ID attribute value %d, expected (0 - 11)" % fe
                        dacList = f.xpath('.//%s' % self.__dacName)
                        dacLen = len(dacList)
                        if dacLen != 1:
                            if dacLen == 0:
                                continue
                            else:
                                raise calFileReadExcept, "found %d %s elements, expected 1" % (dacLen, self.__dacName)
                        d = dacList[0]
                        dd = d.childNodes[0]
                        dac = int(dd.data.strip(), 16)
                        (row, end) = ccToRow(ccc, rc)
                        dacData[tem, row, end, fe] = dac

        return dacData
        
        
        
    def write(self, dacData, tems = (0,)):
        """
        Write CAL data to a ConfigSystem settings XML file

        Param: dacData - A Numeric array of data (16, 8, 2, 12) to write to the
        XML file.
        Param: tems - A list of TEM ID values to include in the output data set.
        """

        doc = self.getDoc()
        
        # insert <LATC_XML> element  
            
        gl = doc.createElementNS(None, 'LATC_XML')
        doc.appendChild(gl)     

        for tem in tems:
                        
            # insert <TEM> elements

            gt = doc.createElementNS(None, 'TEM')
            gt.setAttributeNS(None, 'ID', str(tem))
            gl.appendChild(gt)
            
            #c = doc.createComment("module = %s" % calConstant.CMOD[tem])
            #gt.appendChild(c)
            
            for ccc in range(4):

                # insert <CCC> elements

                gc = doc.createElementNS(None, 'CCC')
                gc.setAttributeNS(None, 'ID', str(ccc))
                gt.appendChild(gc)                

                for rc in range(4):

                    # insert <CRC> elements

                    gr = doc.createElementNS(None, 'CRC')
                    gr.setAttributeNS(None, 'ID', str(rc))
                    gc.appendChild(gr)
                                    
                    # translate index

                    (row, end) = ccToRow(ccc, rc) 
                    #c = doc.createComment("layer = %s%s " % (calConstant.CROW[row], calConstant.CPM[end]))
                    #gr.appendChild(c) 
                    
                    for fe in range(12):

                        # insert <CFE> elements

                        gf = doc.createElementNS(None, 'CFE')
                        gf.setAttributeNS(None, 'ID', str(fe))
                        gr.appendChild(gf)

                        # insert <xxx> elements

                        dv = doc.createElementNS(None, self.__dacName)
                        t = doc.createTextNode('0x%x' % int(dacData[tem, row, end, fe]))
                        dv.appendChild(t)
                        gf.appendChild(dv)
                        
        
        # write output XML file

        self.writeFile()
        
        


def ccToRow(ccc, rc):
    """
    Translate GCCC and GCRC numbers to CAL row and end numbers

    Param: ccc - The GCCC number (0 - 3)
    Param: rc - The GCRC number (0 - 3)

    Returns: (row, end)
        row - The row number (0 - 7)
        end - The end number (0 - 1)
    """
    
    if (ccc < 0) or (ccc > 3):
        raise ValueError, "ccc parameter limited to range [0,3]"
        
    if (rc < 0) or (rc > 3):
        raise ValueError, "rc parameter limited to range [0,3]"

    if (ccc % 2) != 0:
        row = (rc + 4)
    else:
        row = rc
        
    if ccc < 2:
        end = 1
    else:
        end = 0

    return(row, end)


def rowToCC(row, end):
    """
    Translate CAL row and end numbers to GCCC and GCRC numbers.
    
    Param: row - The CAL row number (0 - 7)
    Param: end - The CAL end number (0 - 1)
    
    Returns: (ccc, rc)
        ccc - The GCCC number (0 - 3)
        rc - The GCRC number (0 - 3)
    """
    
    if (row < 0) or (row > 7):
        raise ValueError, "row parameter limited to range [0,7]"
        
    if (end < 0) or (end > 1):
        raise ValueError, "end parameter limited to range [0,1]"
    
    if end == 0:
        ccc = 2
    else:
        ccc = 0
        
    if row >= 4:
        rc = (row - 4)
        ccc += 1
    else:
        rc = row
        
    
    return (ccc, rc)
    

        
