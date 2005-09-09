"""
Global calorimeter constant values
"""


__facility__      = "Online"
__abstract__      = "Global calorimeter constant values"
__author__        = "D.L. Wood"
__date__          = "$Date: 2005/09/09 16:36:29 $"
__version__       = "$Revision: 1.6 $, $Author: dwood $"
__release__       = "$Name:  $"
__credits__       = "NRL code 7650"



# Calorimeter face number to name mapping.

CFACE = ('X+','Y+','X-','Y-')

# Calorimeter side number to name mapping.

CXY = ('X','Y','X','Y')

# Calorimter log end number to name mapping

CPM = ('-','+','-/+')

# Calorimeter range number to name mapping

CRNG = ('LEX8','LEX1','HEX8','HEX1')

# Calorimeter symbolic names for energy range indicies

CRNG_LEX8 = 0
CRNG_LEX1 = 1
CRNG_HEX8 = 2
CRNG_HEX1 = 3

# Calorimeter layer number to string mapping

CLAYER = ('0','1','2','3')

# Calorimeter row number to string mapping

CROW = ('X0','X1','X2','X3','Y0','Y1','Y2','Y3')

# Calorimter log number to string mapping

CLOG = ('0','1','2','3','4','5','6','7','8','9','10','11')

# Calorimeter energy range number to name mapping

CLEHE = ('LE','HE', 'BOTH')

# Calorimeter DAC range to name mapping

CDAC = ('FINE', 'COARSE')

# Number of calorimeter faces

NUM_FACE = 8

# Number of calorimeter layers

NUM_LAYER = 8

# Number of calorimeter rows

NUM_ROW = 8

# Number of calorimeter ends

NUM_END = 2

# Number of calorimeter FE's

NUM_FE = 12

# Number of possible calorimeters TEM's

NUM_TEM = 16


