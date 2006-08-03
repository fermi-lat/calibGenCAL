"""
collection of simple utilities shared throughout my code
"""

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to asymmetry xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/06/29 16:02:41 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

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
