#! /usr/bin/env python
"""
Summarize Cal_Mode precinct settings

Provide library class for utilization by other python scripts and main routine
for simple commandline execution.

output to stdout

The commandline is:
summarize_LATC_CAL_Mode.py latc_rootfile

where:
     latc_rootfile       - ROOT file containing LATC configuration.
"""

__facility__    = "Offline"
__abstract__    = "Diff 2 LATC CFE XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/11/29 21:24:21 $"
__version__     = "$Revision: 1.8 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

def split_ccc_idx(idx):
    """
    split ccc index into tem/ccc pair
    """
    return (idx/4,idx%4)

def split_crc_idx(idx):
    """
    split crc index into tem/ccc/rc tuple
    """
    return (idx/16, (idx/4)%4, idx%4)

def split_cfe_idx(idx):
    """
    split cfe index into tem/ccc/rc/fe tuple
    """
    return (idx/(12*4*4), (idx/(12*4))%4, (idx/12)%4, idx%12)
    

class Cal_Mode_LATC:
    """
    Represent all settings in Cal_Mode LATC configuration precinct.
    """
    import Numeric

    layer_mask_0 = Numeric.zeros([16,4],Numeric.UInt32)
    layer_mask_0_bcast = None
    layer_mask_1 = Numeric.zeros([16,4],Numeric.UInt32)
    layer_mask_1_bcast = None
    ccc_configuration = Numeric.zeros([16,4],Numeric.UInt32)
    ccc_configuration_bcast = None
    crc_dac = Numeric.zeros([16,4],Numeric.UInt32)
    crc_dac_bcast = None
    #config = Numeric.zeros([16,4],Numeric.UInt32)
    #config_bcast = None
    config_0 = Numeric.zeros([16,4,4,12],Numeric.UInt32)
    config_0_bcast = None
    config_1 = Numeric.zeros([16,4,4,12],Numeric.UInt32)
    config_1_bcast = None
    ref_dac = Numeric.zeros([16,4,4,12],Numeric.UInt32)
    ref_dac_bcast = None


    def getField(fieldName):
        """
        return LAT-wide array of values for given Cal_Mode field name
        """

    def readFromROOT(self, rootFileName):
        """
        Populate all Cal_Mode LATC settings from LATC ROOT file
        """
        import ROOT
        
        f = ROOT.TFile(rootFileName)
        t = f.Get("Config")
        t.GetEntry(0)

        l = t.GetLeaf("layer_mask_0")
        for idx in range(l.GetLen()):
            (tem,ccc) = split_ccc_idx(idx)
            self.layer_mask_0[tem][ccc] = l.GetValue(idx)
        l = t.GetLeaf("layer_mask_0_bcast")
        self.layer_mask_0_bcast = int(l.GetValue())
            
        l = t.GetLeaf("layer_mask_1")
        for idx in range(l.GetLen()):
            (tem,ccc) = split_ccc_idx(idx)
            self.layer_mask_1[tem][ccc] = l.GetValue(idx)
        l = t.GetLeaf("layer_mask_1_bcast")
        self.layer_mask_1_bcast = int(l.GetValue())

        l = t.GetLeaf("ccc_configuration")
        for idx in range(l.GetLen()):
            (tem,ccc) = split_ccc_idx(idx)
            self.ccc_configuration[tem][ccc] = int(l.GetValue(idx))
        l = t.GetLeaf("ccc_configuration_bcast")
        self.ccc_configuration_bcast = int(l.GetValue())

        l = t.GetLeaf("crc_dac")
        for idx in range(l.GetLen()):
            (tem,ccc) = split_ccc_idx(idx)
            self.crc_dac[tem][ccc] = l.GetValue(idx)
        l = t.GetLeaf("crc_dac_bcast")
        self.crc_dac_bcast = int(l.GetValue())

        l = t.GetLeaf("config_0")
        for idx in range(l.GetLen()):
            (tem,ccc,rc,fe) = split_cfe_idx(idx)
            self.config_0[tem][ccc][rc][fe] = l.GetValue(idx)
        l = t.GetLeaf("config_0_bcast")
        self.config_0_bcast = int(l.GetValue())

        l = t.GetLeaf("config_1")
        for idx in range(l.GetLen()):
            (tem,ccc,rc,fe) = split_cfe_idx(idx)
            self.config_1[tem][ccc][rc][fe] = l.GetValue(idx)
        l = t.GetLeaf("config_1_bcast")
        self.config_1_bcast = int(l.GetValue())

        l = t.GetLeaf("ref_dac")
        for idx in range(l.GetLen()):
            (tem,ccc,rc,fe) = split_cfe_idx(idx)
            self.ref_dac[tem][ccc][rc][fe] = l.GetValue(idx)
        l = t.GetLeaf("ref_dac_bcast")
        self.ref_dac_bcast = int(l.GetValue())


    def __init__(self, rootFileName):
        self.readFromROOT(rootFileName)
        
def gen_layer_mask_0_rpt(calMode):
    print "\nField: layer_mask_0"
    print "BCAST: %X" %(calMode.layer_mask_0_bcast)
    print "EXCEPT:"
    for tem in range(16):
        for ccc in range(4):
            val = calMode.layer_mask_0[tem][ccc]
            if val != calMode.layer_mask_0_bcast:
                print " TEM=%d CCC=%d VAL=%X"%(tem,ccc,val)

def gen_layer_mask_1_rpt(calMode):
    print "\nField: layer_mask_1"
    print "BCAST: %X"%(calMode.layer_mask_1_bcast)
    print "EXCEPT:"
    for tem in range(16):
        for ccc in range(4):
            val = calMode.layer_mask_1[tem][ccc]
            if val != calMode.layer_mask_1_bcast:
                print " TEM=%d CCC=%d VAL=%X"%(tem,ccc,val)


def gen_ccc_configuration_rpt(calMode):
    print "\nField: ccc_configuration"
    print "BCAST: %X"%(calMode.ccc_configuration_bcast)
    for tem in range(16):
        for ccc in range(4):
            val = int(calMode.ccc_configuration[tem][ccc])
            if val != calMode.ccc_configuration_bcast:
                print " TEM=%d CCC=%d VAL=%X"%(tem,ccc,val)

def gen_crc_dac_rpt(calMode):
    print "\nField: crc_dac"
    print "BCAST: %X"%(calMode.crc_dac_bcast)
    print "EXCEPT:"
    for tem in range(16):
        for ccc in range(4):
            val = calMode.crc_dac[tem][ccc]
            if val != calMode.crc_dac_bcast:
                print " TEM=%d CCC=%d VAL=%X"%(tem,ccc,val)

def gen_crc_config_rpt(calMode):
    print "\nField: crc_config"
    print "BCAST: %X"%(calMode.crc_config_bcast)
    for tem in range(16):
        for ccc in range(4):
            for crc in range(4):
                for cfe in range(4):
                    val = calMode.config_0[tem][ccc][cfe]
                    if val != calMode.config_0_bcast:
                        print " TEM=%d CCC=%d CRC=%d CFE%d VAL=%X"%(tem,ccc,crc,cfe,val)
                        
def gen_config_0_rpt(calMode):
    print "\nField: config_0"
    print "BCAST: %X"%(calMode.config_0_bcast)
    print "EXCEPT:"
    for tem in range(16):
        for ccc in range(4):
            for crc in range(4):
                for cfe in range(4):
                    val = calMode.config_0[tem][ccc][crc][cfe]
                    if val != calMode.config_0_bcast:
                        print " TEM=%d CCC=%d CRC=%d CFE%d VAL=%X"%(tem,ccc,crc,cfe,val)
                        
def gen_config_1_rpt(calMode):
    print "\nField: config_1"
    print "BCAST: %X"%(calMode.config_1_bcast)
    for tem in range(16):
        for ccc in range(4):
            for crc in range(4):
                for cfe in range(12):
                    val = calMode.config_1[tem][ccc][crc][cfe]
                    if val != calMode.config_1_bcast:
                        print " TEM=%d CCC=%d CRC=%d CFE%d VAL=%X"%(tem,ccc,crc,cfe,val)

def gen_ref_dac_rpt(calMode):
    print "\nField: ref_dac"
    print "BCAST: %X"%(calMode.ref_dac_bcast)
    for tem in range(16):
        for ccc in range(4):
            for crc in range(4):
                for cfe in range(12):
                    val = calMode.ref_dac[tem][ccc][crc][cfe]
                    if val != calMode.ref_dac_bcast:
                        print " TEM=%d CCC=%d CRC=%d CFE%d VAL=%X"%(tem,ccc,crc,cfe,val)



def genCalModeReport(calMode):
    """
    Generate Cal_Mode precint report from LATC root file.
    """

    gen_layer_mask_0_rpt(calMode)

    gen_layer_mask_1_rpt(calMode)

    gen_ccc_configuration_rpt(calMode)

    gen_crc_dac_rpt(calMode)

    #gen_crc_config_rpt(calMode)

    gen_config_0_rpt(calMode)

    gen_config_1_rpt(calMode)

    gen_ref_dac_rpt(calMode)


if __name__ == '__main__':
    usage = 'summarize_LATC_CAL_Mode.py latc_rootfile'

    # check command line
    import sys
    args = sys.argv[1:]
    if len(args) != 1:
        print usage
        sys.exit(1)

    # get filenames
    inputPath = args[0]

    # retrieve LATC data from file
    calMode = Cal_Mode_LATC(inputPath)

    # generate report
    print "Filename: %s\n"%inputPath
    genCalModeReport(calMode)

