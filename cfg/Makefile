# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/cfg/Makefile,v 1.1 2007/02/28 19:53:14 fewtrell Exp $
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS ##

### A1: MUON GAIN PEDESTALS
# cfg file used to drive most C++ muon calibration applications
MUON_CGC_CFG = calibGenCAL_suite.cfg

# first digi root input file for pedestal calibration 
# (this file will be used for output filename generation) - the full list of files should be found in MUON_CGC_CFG file
FIRST_PED_ROOT_INPUT = /nfs/farm/g/glast/u34/Integration/rootData/077003627/v6r070329p16/grRoot/digitization-licos-v3r6p2_077003627_digi_DIGI.root



## SECTION B: INT-NONLIN (CIDAC2ADC) ##

### B1: MUON GAIN INT-NONLIN
# input LE singlex16 digi root event file (muon gain, calibGain=on, calibGen element#201)
INPUT_INL_LE_MUON = /nfs/farm/g/glast/u34/Integration/rootData/077005401/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005401_digi_DIGI.root
# input HE singlex16 digi root event file (muon gain), calibGain=off, calibGen element#204)
INPUT_INL_HE_MUON = /nfs/farm/g/glast/u34/Integration/rootData/077005404/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005404_digi_DIGI.root

### B2: FLIGHT GAIN INT-NONLIN
# input LE singlex16 digi root event file (flight gain)
INPUT_INL_LE_FLIGHT = 
# input HE singlex16 digi root event file (flight gain)
INPUT_INL_HE_FLIGHT = 

### B3: 'NEIGHBOR' XTALK
# input 'NeighborXtalk' singlex16 digi root event file (flight gain, le_pulse_only, columnwise)
INPUT_NEIGHBOR_XTALK = /nfs/slac/g/glast/users/glground/bregeon/DIGICALCI/700002700.root



## SECTION C: OPTICAL RESPONSE (MUON) ##

### C1: MEVPERDAC / ASYM COMBINED USING TRACKER TRACKS
# first digi root input file for muon optical response calibration
# (this file will be used for output filename generation) - the full list of files should be found in MUON_CGC_CFG file
INPUT_FIRST_MUOPT_ROOT = /nfs/farm/g/glast/u34/Integration/rootData/077003627/v6r070329p16/grRoot/digitization-licos-v3r6p2_077003627_digi_DIGI.root
# first digi root input file for muon optical response calibration
# (this file will be used for output filename generation) - the full list of files should be found in MUON_CGC_CFG file

INPUT_FIRST_MUOPT_SVAC = /nfs/farm/g/glast/u34/Integration/rootData/077003627/v6r070329p16/calib-v1r0/svacTuple/emRootv0r0/svacTuple-v3r6p2_077003627_svac_svac.root



#############
## OPTIONS ##
#############

# set to '-b' if inl data is in broadcast mode, blank if not
INL_MUON_BCAST   = -b
INL_FLIGHT_BCAST = -b


#########################
### AUTOGEN_FILENAMES ###
#########################
INL_PFX   = cidac2adc
PED_PFX   = calPeds
ASYM_PFX  = calAsym
MPD_PFX   = calMPD

INL_MUON_BASE = ${INL_PFX}.$(basename $(notdir ${INPUT_INL_LE_MUON}))
INL_MUON_TXT  = $(addsuffix .txt, ${INL_MUON_BASE})
INL_MUON_XML  = $(addsuffix .xml, ${INL_MUON_BASE})
INL_MUON_VAL  = $(addsuffix .val.root, ${INL_MUON_BASE})

INL_FLIGHT_BASE = ${INL_PFX}.$(basename $(notdir ${INPUT_INL_LE_FLIGHT}))
INL_FLIGHT_TXT  = $(addsuffix .txt, ${INL_FLIGHT_BASE})
INL_FLIGHT_XML  = $(addsuffix .xml, ${INL_FLIGHT_BASE})
INL_FLIGHT_VAL  = $(addsuffix .val.root, ${INL_FLIGHT_BASE})

NEIGHBOR_XTALK_BASE = neighborXtalk.$(basename $(notdir ${INPUT_NEIGHBOR_XTALK}))
NEIGHBOR_XTALK_TXT  = $(addsuffix .txt, ${NEIGHBOR_XTALK_BASE})

MUON_PED_BASE = ${PED_PFX}.$(basename $(notdir ${FIRST_PED_ROOT_INPUT}))
MUON_PED_TXT  = $(addsuffix .txt, ${MUON_PED_BASE})
MUON_PED_XML  = $(addsuffix .xml, ${MUON_PED_BASE})
MUON_PED_VAL  = $(addsuffix .val.root, ${MUON_PED_BASE})

MUON_ASYM_BASE = ${ASYM_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
MUON_ASYM_TXT  = $(addsuffix .txt, ${MUON_ASYM_BASE})
MUON_ASYM_XML  = $(addsuffix .xml, ${MUON_ASYM_BASE})
MUON_ASYM_VAL  = $(addsuffix .val.root, ${MUON_ASYM_BASE})

MUON_MPD_BASE = ${MPD_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
MUON_MPD_TXT  = $(addsuffix .txt, ${MUON_MPD_BASE})
MUON_MPD_XML  = $(addsuffix .xml, ${MUON_MPD_BASE})
MUON_MPD_VAL  = $(addsuffix .val.root, ${MUON_MPD_BASE})



#########################
## General ENV SETUP ####
#########################
#DISPLAY =





####################
## GENERIC RULES ###
####################
${INL_PFX}.%.val.root : ${INL_PFX}.%.xml
	intNonlinVal.sh -R $@ $<

${INL_PFX}.%.xml : ${INL_PFX}.%.txt
	inlTXT2XML.sh $< $@

# ${INL_PFX}.%.txt : ${INL_PFX}.%.adcmean.txt
# 	smoothINL.exe $< $@

${PED_PFX}.%.xml : ${PED_PFX}.%.txt
	pedTXT2XML.sh $< $@

${PED_PFX}.%.val.root : ${PED_PFX}.%.xml
	pedVal.sh -R $@ $<


${ASYM_PFX}.%.xml : ${ASYM_PFX}.%.txt
	asymTXT2XML.sh $< $@

${ASYM_PFX}.%.val.root : ${ASYM_PFX}.%.xml
	asymVal.sh -R $@ $<


${MPD_PFX}.%.xml : ${MPD_PFX}.%.txt
	asymTXT2XML.sh $< $@

${MPD_PFX}.%.val.root : ${MPD_PFX}.%.xml
	asymVal.sh -R $@ $<



##########################
### TARGET OUTPUTS #######
##########################
all: val

val : muopt_val muon_inl_val muon_ped_val

muon_inl : muon_inl_val

muon_inl_val : ${INL_MUON_VAL} 


muon_inl_xml : ${INL_MUON_XML} 


muon_inl_txt : ${INL_MUON_TXT} 
${INL_MUON_TXT} : ${INPUT_INL_LE_MUON} ${INPUT_INL_HE_MUON}
	genCIDAC2ADC.exe ${MUON_CGC_CFG}

neighbor_xtalk : ${NEIGHBOR_XTALK_TXT}
${NEIGHBOR_XTALK_TXT} : ${INPUT_NEIGHBOR_XTALK}
	genNeighborXtalk.exe $< ${NEIGHBOR_XTALK_BASE}

peds : muon_ped_val

muon_ped_val : ${MUON_PED_VAL}

muon_ped_xml : ${MUON_PED_XML}

muon_ped_txt : ${MUON_PED_TXT}
${MUON_PED_TXT} :
	genMuonPed.exe ${MUON_CGC_CFG}




muopt : muopt_val

muopt_val : ${MUON_ASYM_VAL} ${MUON_MPD_VAL}

muon_asym_val : ${MUON_ASYM_VAL}

muon_mpd_val : ${MUON_MPD_VAL}

muopt_xml : ${MUON_ASYM_XML} ${MUON_MPD_XML}

muon_asym_xml : ${MUON_ASYM_XML}

muon_mpd_xml : ${MUON_MPD_XML}

muon_asym_txt : ${MUON_ASYM_TXT}

muon_mpd_txt : ${MUON_MPD_TXT}


${MUON_ASYM_TXT} ${MUON_MPD_TXT} : muopt_txt
muopt_txt: muon_ped_txt muon_inl_txt
	genMuonCalibTkr.exe ${MUON_CGC_CFG}

