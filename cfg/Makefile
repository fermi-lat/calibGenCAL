# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/cfg/Makefile,v 1.6 2007/04/10 21:26:41 fewtrell Exp $
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS ##

### A1: MUON GAIN PEDESTALS
MUPED_DIGI_FILES = muon_digi_files.txt
# options 'PERIODIC' 'EXTERNAL' 'ALL'
MUPED_EVENT_FILTER     = PERIODIC
# 1000 events minimum per hist is sufficient
MUPED_NENTRIES_PERHIST = 1000


## SECTION B: INT-NONLIN (CIDAC2ADC) ##
### B1: MUON GAIN INT-NONLIN
# input LE singlex16 digi root event file (muon gain, calibGain=on, calibGen element#201)
INL_MUON_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014468/v7r0913p10/digi/digitization-licos-v3r9p2_077014468_digi_DIGI.root
# input HE singlex16 digi root event file (muon gain), calibGain=off, calibGen element#204)
INL_MUON_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014471/v7r0913p10/digi/digitization-licos-v3r9p2_077014471_digi_DIGI.root

### B2: FLIGHT GAIN INT-NONLIN
# input LE singlex16 digi root event file (flight gain, calibGain=on, fle=nominal, calibGen element#101)
INL_FLIGHT_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014460/v7r0913p10/digi/digitization-licos-v3r9p2_077014460_digi_DIGI.root
# input HE singlex16 digi root event file (flight gain, calibGain=on, fle=nominla, calibGen element#102)
INL_FLIGHT_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014461/v7r0913p10/digi/digitization-licos-v3r9p2_077014461_digi_DIGI.root

### B3: 'NEIGHBOR' XTALK
# input 'NeighborXtalk' singlex16 digi root event file (flight gain, le_pulse_only, columnwise)
INPUT_NEIGHBOR_XTALK = /nfs/slac/g/glast/users/glground/bregeon/DIGICALCI/700002700.root



## SECTION C: OPTICAL RESPONSE (MUON) ##
### C1: MEVPERDAC / ASYM COMBINED USING TRACKER TRACKS
MUOPT_DIGI_FILES = muon_digi_files.txt
MUOPT_SVAC_FILES = muon_svac_files.txt
MUOPT_NENTRIES_PERHIST = 10000

## SECTION D: FLIGHT GAIN EXTRAPOLATION ##
### D1: CALIBGAIN ##
# input HE singlex16 digi root event file (flight_gain, calibGain=on, fle=127, calibGen element #106)
CG_DIGI_CGON  = /nfs/farm/g/glast/u41/Integration/rootData/077014465/v7r0913p10/digi/digitization-licos-v3r9p2_077014465_digi_DIGI.root
# input HE singlex16 digi root event file (flight_gain, calibGain=off, fle=127, calibGen element #108)
CG_DIGI_CGOFF = /nfs/farm/g/glast/u41/Integration/rootData/077014467/v7r0913p10/digi/digitization-licos-v3r9p2_077014467_digi_DIGI.root


#############
## OPTIONS ##
#############

# set to '-c' if inl data is in column mode, blank if not
#INL_COLMODE    = -c

#########################
### AUTOGEN_FILENAMES ###
#########################
INL_SFX     = .cidac2adc
PED_SFX     = .calPed
ASYM_SFX    = .calAsym
MPD_SFX     = .calMPD
MUSLOPE_SFX = .muSlope
ADC2NRG_SFX = .adc2nrg
MUOPT_SFX   = .muonOptical
CG_SFX = .calibGain

INL_MUON_BASE = $(addsuffix ${INL_SFX}, $(basename $(notdir ${INL_MUON_DIGI_LE})))
INL_MUON_TXT  = $(addsuffix .txt, ${INL_MUON_BASE})
INL_MUON_XML  = $(addsuffix .xml, ${INL_MUON_BASE})
INL_MUON_VAL  = $(addsuffix .val.root, ${INL_MUON_BASE})

INL_FLIGHT_BASE = $(addsuffix ${INL_SFX}, $(basename $(notdir ${INL_FLIGHT_DIGI_LE})))
INL_FLIGHT_TXT  = $(addsuffix .txt, ${INL_FLIGHT_BASE})
INL_FLIGHT_XML  = $(addsuffix .xml, ${INL_FLIGHT_BASE})
INL_FLIGHT_VAL  = $(addsuffix .val.root, ${INL_FLIGHT_BASE})

CG_ON_BASE = $(addsuffix ${INL_SFX}, $(basename $(notdir ${CG_DIGI_CGON})))
CG_ON_TXT = $(addsuffix .txt, ${CG_ON_BASE})
CG_ON_XML = $(addsuffix .xml, ${CG_ON_BASE})


CG_OFF_BASE = $(addsuffix ${INL_SFX}, $(basename $(notdir ${CG_DIGI_CGOFF})))
CG_OFF_TXT = $(addsuffix .txt, ${CG_OFF_BASE})
CG_OFF_XML = $(addsuffix .xml, ${CG_OFF_BASE})

CG_TXT = $(addsuffix .txt, $(addsuffix ${CG_SFX}, ${CG_ON_BASE}))

NEIGHBOR_XTALK_BASE = $(addsuffix ${NBR_XTALK_SFX}, $(basename $(notdir ${INPUT_NEIGHBOR_XTALK})))
NEIGHBOR_XTALK_TXT  = $(addsuffix .txt, ${NEIGHBOR_XTALK_BASE})

FIRST_MUPED_DIGI = $(shell head -n 1 ${MUPED_DIGI_FILES})
PED_MUON_BASE = $(addsuffix ${PED_SFX}, $(basename $(notdir ${FIRST_MUPED_DIGI})))
PED_MUON_TXT  = $(addsuffix .txt, ${PED_MUON_BASE})
PED_MUON_XML  = $(addsuffix .xml, ${PED_MUON_BASE})
PED_MUON_VAL  = $(addsuffix .val.root, ${PED_MUON_BASE})

FIRST_MUOPT_DIGI = $(shell head -n 1 ${MUOPT_DIGI_FILES})
MUOPT_BASE = $(addsuffix ${MUOPT_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI})))
ASYM_MUON_BASE = $(addsuffix ${ASYM_SFX}, ${MUOPT_BASE})
ASYM_MUON_TXT  = $(addsuffix .txt, ${ASYM_MUON_BASE})
ASYM_MUON_XML  = $(addsuffix .xml, ${ASYM_MUON_BASE})
ASYM_MUON_VAL  = $(addsuffix .val.root, ${ASYM_MUON_BASE})

MPD_MUON_BASE = $(addsuffix ${MPD_SFX}, ${MUOPT_BASE})
MPD_MUON_TXT  = $(addsuffix .txt, ${MPD_MUON_BASE})
MPD_MUON_XML  = $(addsuffix .xml, ${MPD_MUON_BASE})
MPD_MUON_VAL  = $(addsuffix .val.root, ${MPD_MUON_BASE})

MUSLOPE_BASE = $(addsuffix ${MUSLOPE_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI})))
MUSLOPE_XML  = $(addsuffix .xml, ${MUSLOPE_BASE})
MUSLOPE_VAL  = $(addsuffix .val.root, ${MUSLOPE_BASE})

ADC2NRG_BASE = $(addsuffix ${ADC2NRG_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI})))
ADC2NRG_XML  = $(addsuffix .xml, ${ADC2NRG_BASE})
ADC2NRG_VAL  = $(addsuffix .val.root, ${ADC2NRG_BASE})

#########################
## General ENV SETUP ####
#########################
#DISPLAY =

####################
## GENERIC RULES ###
####################
%${INL_SFX}.val.root : %${INL_SFX}.xml
	intNonlinVal.sh -R $@ $<

%${INL_SFX}.xml : %${INL_SFX}.txt
	inlTXT2XML.sh $< $@

# %${INL_SFX}.txt : %${ADCMEAN_SFX}.txt
# 	smoothINL.exe $< $@

%${PED_SFX}.xml : %${PED_SFX}.txt
	pedTXT2XML.sh $< $@

%${PED_SFX}.val.root : %${PED_SFX}.xml
	pedVal.sh -R $@ $<


%${ASYM_SFX}.xml : %${ASYM_SFX}.txt
	asymTXT2XML.sh $< $@

%${ASYM_SFX}.val.root : %${ASYM_SFX}.xml
	asymVal.sh -R $@ $<


%${MPD_SFX}.xml : %${MPD_SFX}.txt
	mpdTXT2XML.sh $< $@

%${MPD_SFX}.val.root : %${MPD_SFX}.xml
	mevPerDacVal.sh -R $@ $<

%${MUSLOPE_SFX}.val.root : %${MUSLOPE_SFX}.xml
	muSlopeVal.sh -R $@ $<

%${ADC2NRG_SFX}.val.root : %${ADC2NRG_SFX}.xml
	adc2nrgVal.sh -R $@ $<


##########################
### TARGET OUTPUTS #######
##########################
.PHONY: all
all: ped inl muopt

.PHONY: inl
inl : inl_muon inl_flight

.PHONY : inl_muon
inl_muon: inl_muon_val

.PHONY: inl_muon_val
inl_muon_val : ${INL_MUON_VAL}

.PHONY: inl_muon_xml
inl_muon_xml : ${INL_MUON_XML} 

.PHONY: inl_muon_txt
inl_muon_txt : ${INL_MUON_TXT} 

${INL_MUON_TXT} : ${INL_MUON_DIGI_LE} ${INL_MUON_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${INL_MUON_DIGI_LE} -h  ${INL_MUON_DIGI_HE} ${INL_MUON_BASE}

.PHONY: inl_flight
inl_flight : inl_flight_val

.PHONY: inl_flight_val
inl_flight_val : ${INL_FLIGHT_VAL}

.PHONY: inl_flight_xml
inl_flight_xml : ${INL_FLIGHT_XML} 

.PHONY: inl_flight_txt
inl_flight_txt : ${INL_FLIGHT_TXT} 

${INL_FLIGHT_TXT} : ${INL_FLIGHT_DIGI_LE} ${INL_FLIGHT_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${INL_FLIGHT_DIGI_LE} -h  ${INL_FLIGHT_DIGI_HE} ${INL_FLIGHT_BASE}

.PHONY: calib_gain
calib_gain : cg_txt

.PHONY: cg_txt
cg_txt: ${CG_TXT}
${CG_TXT}: ${CG_ON_XML} ${CG_OFF_XML}
	calibGainCoeff.sh ${CG_ON_XML} ${CG_OFF_XML} > ${CG_TXT}

.PHONY : cg_on_txt
cg_on_txt: ${CG_ON_TXT}
${CG_ON_TXT}:
	genCIDAC2ADC.exe  ${INL_COLMODE} -h ${CG_DIGI_CGON} ${CG_ON_BASE}

.PHONY : cg_off_txt
cg_off_txt: ${CG_OFF_TXT}
${CG_OFF_TXT}:
	genCIDAC2ADC.exe  ${INL_COLMODE} -h ${CG_DIGI_CGOFF} ${CG_OFF_BASE}




.PHONY: neighbor_xtalk
neighbor_xtalk : ${NEIGHBOR_XTALK_TXT}

${NEIGHBOR_XTALK_TXT} : ${INPUT_NEIGHBOR_XTALK}
	genNeighborXtalk.exe $< ${NEIGHBOR_XTALK_BASE}


.PHONY: peds
peds : ped

.PHONY: ped
ped : ped_muon_val

.PHONY: ped_muon_val
ped_muon_val : ${PED_MUON_VAL}

.PHONY: ped_muon_xml
ped_muon_xml : ${PED_MUON_XML}

.PHONY: ped_muon_txt
ped_muon_txt : ${PED_MUON_TXT}

${PED_MUON_TXT} : ${MUPED_DIGI_FILES}
	genMuonPed.exe ${MUPED_DIGI_FILES} ${PED_MUON_BASE} --entriesPerHist=${MUPED_NENTRIES_PERHIST} --triggerCut=${MUPED_EVENT_FILTER}




muopt : muopt_val

.PHONY: muopt_val 
muopt_val : ${ASYM_MUON_VAL} ${MPD_MUON_VAL} ${MUSLOPE_VAL} ${ADC2NRG_VAL}

.PHONY: asym_muon_val 
asym_muon_val : ${ASYM_MUON_VAL}

.PHONY: mpd_muon_val 
mpd_muon_val : ${MPD_MUON_VAL}

.PHONY: muslope_val 
muslope_val : ${MUSLOPE_VAL}
.PHONY: adc2nrg_val 
adc2nrg_val : ${ADC2NRG_VAL}


.PHONY: muopt_xml 
muopt_xml : ${ASYM_MUON_XML} ${MPD_MUON_XML} ${MUSLOPE_XML}

.PHONY: asym_muon_xml 
asym_muon_xml : ${ASYM_MUON_XML}

.PHONY: muslope_xml 
muslope_xml : ${MUSLOPE_XML}

.PHONY: mpd_muon_xml 
mpd_muon_xml : ${MPD_MUON_XML}

# adc2nrg.xml & muslope.xml are created simultaneously
${ADC2NRG_XML} : ${MUSLOPE_XML}

${MUSLOPE_XML} :  ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML}
	genADC2NRG.sh ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML} ${ADC2NRG_XML} ${MUSLOPE_XML}


.PHONY: asym_muon_txt
asym_muon_txt : ${ASYM_MUON_TXT}

.PHONY: mpd_muon_txt 
mpd_muon_txt : ${MPD_MUON_TXT}

.PHONY: muopt_txt 
muopt_txt : ${ASYM_MUON_TXT} ${MPD_MUON_TXT}


# asym.txt & mpd.txt are created simultaneously
${ASYM_MUON_TXT} : ${MPD_MUON_TXT}

${MPD_MUON_TXT} : ${PED_MUON_TXT} ${INL_MUON_TXT} ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} 
	genMuonCalibTkr.exe ${PED_MUON_TXT} ${INL_MUON_TXT}  ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} ${MUOPT_BASE} --entriesPerHist=${MUOPT_NENTRIES_PERHIST}

