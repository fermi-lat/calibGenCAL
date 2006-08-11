"""
collection of simple utilities shared throughout my code
"""

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to asymmetry xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/08/10 18:06:43 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import calConstant
import calCalibXML
import array
import Numeric
import ROOT

### CONSTANTS ###
DIODE_LRG  = 0
DIODE_SM   = 1

mpdBigValIdx   = 0
mpdSmallValIdx = 1
mpdBigSigIdx   = 2
mpdSmallSigIdx = 3



"""
Convert offline xtal face indexing to online xtal face indexing  (they're reversed, don't blame me :)
"""
offline_face_to_online = {0:1,1:0}

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
                    online_face = offline_face_to_online[face]
                    for rng in range(calConstant.NUM_RNG):
                        length = int(lenData[rng][twr,row,online_face,col])

                        # skip empty channels: HACK WARNING!
                        # unfortunately i fear that calCalib .dtd requires that all channels have
                        # some entry, so sometimes I have put in empty channels w/ single point
                        # 0,0  This seems to break the TSpline objects in this script
                        # so I skip any channel w/ either 0 _or_ 1 entry in it.
                        if length <= 1:
                            continue

                        dacArray = array.array('d', dac[rng][twr,row,online_face,col,0:length].tolist())
                        adcArray = array.array('d', adc[rng][twr,row,online_face,col,0:length].tolist())

                        a2dSpline = ROOT.TSpline3("%d_%d_%d_%d_adc2dac"%(twr,lyr,col,face),
                                                  adcArray,
                                                  dacArray,
                                                  length)

                        d2aSpline = ROOT.TSpline3("%d_%d_%d_%d_dac2adc"%(twr,lyr,col,face),
                                                  dacArray,
                                                  adcArray,
                                                  length)

                        adc2dac[(twr,row,online_face,col,rng)] = a2dSpline
                        dac2adc[(twr,row,online_face,col,rng)] = d2aSpline


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
        face = offline_face_to_online[face]
        # convert offline lyr to row
        row = calCalibXML.layerToRow(lyr)

        # make sure current tower is on list
        twrSet.add(twr)

        outData[twr, row, face, col] = ratio

    return (outData, twrSet)

