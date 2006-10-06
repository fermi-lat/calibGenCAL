"""
Global calorimeter constant values
"""


__facility__      = "Online"
__abstract__      = "Global calorimeter constant values"
__author__        = "D.L. Wood"
__date__          = "$Date: 2006/07/18 17:49:15 $"
__version__       = "$Revision: 1.3 $, $Author: dwood $"
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

# Calorimeter symbolic names for DAC range indicies

CDAC_FINE   = 0
CDAC_COARSE = 1

# Number of calorimeter faces

NUM_FACE = 4

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

# Number of calorimeter energy ranges

NUM_RNG = 4

# Symbolic names for the towers/tems as modules

CMOD = ('FM104', 'FM103', 'FM117', 'FM118', 'FM105', 'FM102', 'FM115', 'FM116',
        'FM107', 'FM106', 'FM111', 'FM112', 'FM110', 'FM108', 'FM113', 'FM114')
         



def temToModule(tem):
    """
    Gets a module name based on the TEM number.
    Param: tem - the TEM number (0 - 15).
    Returns: The module name string.
    """
    
    if tem < 0 or tem > 15:
        raise ValueError, "tem value %d not 0<>15." % tem
        
    return CMOD[tem]
    
        
        
def rowToName(row):
    """
    Gets a layer name based on the row number.
    Param: row - the row number (0 - 7).
    Returns: The layer name string.
    """
    
    if row < 0 or row > 7:
        raise ValueError, "row value %d not 0<>7" % row
        
    return CROW[row]
                 