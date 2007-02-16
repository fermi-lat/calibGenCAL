"""
collection of simple utilities shared throughout my code
"""

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to asymmetry xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2007/02/08 16:37:30 $"
__version__   = "$Revision: 1.11 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import calConstant
import calCalibXML
import array
import Numeric
import ROOT

### CONSTANTS ###
mpdBigValIdx   = 0
mpdSmallValIdx = 1
mpdBigSigIdx   = 2
mpdSmallSigIdx = 3
N_MPD_IDX      = 4

"""
calCalibAsym array idx dictionary
tuple idx keys are (pos_diode, neg_diode, sigma)
"""
asymIdx = {
    (0,0,False):0,\
    (1,1,False):1,\
    (0,1,False):2,\
    (1,0,False):3,\
    (0,0,True):4,\
    (1,1,True):5,\
    (0,1,True):6,\
    (1,0,True):7}

"""
Number of CIDAC values sampled in standard singlex16 run
"""
N_DAC_PTS = 173


"""
build set of spline objects from intNonlin data
return tuple of 2 dictionaries of ROOT.TSpline3 objects (adc2dac, dac2adc)
dictionaries are indexed by tuples (twr,row,online_face,col,rng)
input is intNonlin data as returned by calCalibXML.calIntNonlinCalibXML.read()
"""
def build_inl_splines(data, twrSet):
    adc2dac = dict()
    dac2adc = dict()

    (lenData, dac, adc) = data
    
    for twr in twrSet:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for rng in range(calConstant.NUM_RNG):
                        length = int(lenData[rng][twr,row,online_face,col])

                        # skip empty channels: HACK WARNING!
                        # unfortunately i fear that calCalib .dtd requires that all channels have
                        # some entry, so sometimes I have put in empty channels w/ single point
                        # 0,0  This seems to break the TSpline objects in this script
                        # so I skip any channel w/ either 0 _or_ 1 entry in it.
                        if length <= 1:
                            continue

                        dacArray = array.array('d', dac[rng][twr,row,online_face,col,0:length])
                        adcArray = array.array('d', adc[rng][twr,row,online_face,col,0:length])

                        a2dSpline = ROOT.TSpline3("%d_%d_%d_%d_%d_adc2dac"%(twr,lyr,col,face,rng),
                                                  adcArray,
                                                  dacArray,
                                                  length)

                        d2aSpline = ROOT.TSpline3("%d_%d_%d_%d_%d_dac2adc"%(twr,lyr,col,face,rng),
                                                  dacArray,
                                                  adcArray,
                                                  length)
                        key = (twr,row,online_face,col,rng)
                        adc2dac[key] = a2dSpline
                        dac2adc[key] = d2aSpline


    return (adc2dac, dac2adc)

"""
read in txt file w/ per-xtal face cal data & return Numeric array

return tuple of (coeffs, twrSet) where:
 - coeffs array shape is [16,8,2,12] for online twr,lyr,col,face
 - twrSet is python set of active LAT towers.

"""
def read_perFace_txt(filename):
    # constants
    nTXTFields = 5
    
    # intialize output array
    outData = Numeric.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_END,
                             calConstant.NUM_FE),
                            Numeric.Float32)
    twrSet = set()

    # read input file
    inFile = open(filename, 'r')

    lines = inFile.readlines()

    nLine = -1
    for line in lines:
        nLine+=1
        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face ratio] " + line)
            sys.exit(-1)

        # convert vals array to floats instead of strings
        for i in range(len(vals)):
            vals[i] = float(vals[i])

        (twr, lyr, col, face, ratio) = vals
        
        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)
        ratio = float(ratio)

        # convert offline face numbering to online face numbering
        face = calConstant.offline_face_to_online[face]
        # convert offline lyr to row
        row = calCalibXML.layerToRow(lyr)

        # make sure current tower is on list
        twrSet.add(twr)

        outData[twr, row, face, col] = ratio

    return (outData, twrSet)



"""
return y3 such that (y2 - y1)/(x2 - x1) = (y3 - y2)/(x3 - x2)
"""

def linear_extrap(x1, x2, x3, y1, y2):
    return (x3-x2)*(y2-y1)/(x2-x1) + y2;


"""
build set of spline objects from cal asymmetry data
return tuple of 2 dictionaries of ROOT.TSpline3 objects (pos2asym, asym2pos)
dictionaries are indexed by tuples (twr, row, online_face, col, diode_size)
input is cal light asymmetry data as returned by calCalibXML.calIntNonlinCalibXML.read()
"""
def build_asym_splines(data, twrSet):
    pos2asym = dict()
    asym2pos = dict()

    (xpos, asym) = data
    posArray = array.array('d', xpos)
    

    # indexes i'm interested in
    
    for twr in twrSet:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                # diode var(0,1) will match 0,1 indexes in calCalibXML.asym data
                for diode in range(calConstant.NUM_DIODE):
                    asym_data = asym[twr,row,col,diode]
                    length = len(asym_data)

                    # skip empty channels
                    if length < 1:
                        continue
                        
                    asymArray = array.array('d', asym_data)
                    
                    p2aSpline = ROOT.TSpline3("%d_%d_%d_%d_pos2asym"%(twr,lyr,col,diode),
                                              posArray,
                                              asymArray,
                                              length)
                    
                    a2pSpline = ROOT.TSpline3("%d_%d_%d_%d_asym2pos"%(twr,lyr,col,diode),
                                              asymArray,
                                              posArray,
                                              length)

                    tpl = (twr,row,col,diode)
                    pos2asym[tpl] = p2aSpline
                    asym2pos[tpl] = a2pSpline

                    # find 'overall' light asymmetry @ center of xtal
                    p2a = pos2asym[tpl]

    return (pos2asym, asym2pos)

# list of CIDAC values probed in order during singlex16 test
CIDAC_TEST_VALS = \
    [ \
             0,    2,     4,      6,      8,      10,      12,      14,      16,      18,     20,    22,    24,    26,   28,   30, 32, \
             34,   36,    38,     40,     42,     44,      46,      48,      50,      52,     54,    56,    58,    60,   62,   64,\
             80,   96,    112,    128,    144,    160,     176,     192,     208,     224,    240,   256,   272,   288,\
             304,  320,   336,    352,    368,    384,     400,     416,     432,     448,    464,   480,   496,   512,\
             543,  575,   607,    639,    671,    703,     735,     767,     799,     831,    863,   895,   927,   959,\
             991,  1023,  1055,   1087,   1119,   1151,    1183,    1215,    1247,    1279,\
             1311, 1343,  1375,   1407,   1439,   1471,    1503,    1535,    1567,    1599,\
             1631, 1663,  1695,   1727,   1759,   1791,    1823,    1855,    1887,    1919,\
             1951, 1983,  2015,   2047,   2079,   2111,    2143,    2175,    2207,    2239,\
             2271, 2303,  2335,   2367,   2399,   2431,    2463,    2495,    2527,    2559,\
             2591, 2623,  2655,   2687,   2719,   2751,    2783,    2815,    2847,    2879,\
             2911, 2943,  2975,   3007,   3039,   3071,    3103,    3135,    3167,    3199,\
             3231, 3263,  3295,   3327,   3359,   3391,    3423,    3455,    3487,    3519,\
             3551, 3583,  3615,   3647,   3679,   3711,    3743,    3775,    3807,    3839,\
             3871, 3903,  3935,   3967,   3999,   4031,    4063,    4095
             ]
