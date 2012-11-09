VPATH = ../src/dview ../src/plot ../src/pdf ../src
CC = gcc
CXX = g++
CFLAGS = -fPIC -Wall -O2 `wx-config-3 --cflags` -I../include
CXXFLAGS = $(CFLAGS)

LIBNAME = wexosxwx3
TARGET = ../$(LIBNAME).a
	
OBJECTS = \
	jsonreader.o \
	jsonval.o \
	jsonwriter.o \
	md5.o \
	numeric.o \
	ribbonbook.o \
	dvautocolourassigner.o \
	dvcolourmap.o \
	dvdcctrl.o \
	dvdmapctrl.o \
	dvfiledataset.o \
	dvplotctrl.o \
	dvplotctrlsettings.o \
	dvplothelper.o \
	dvpncdfctrl.o \
	dvprofilectrl.o \
	dvscatterplotctrl.o \
	dvselectionlist.o \
	dvtimeseriesctrl.o \
	dvtimeseriesdataset.o \
	plaxis.o \
	plhistplot.o \
	pllineplot.o \
	plplotctrl.o \
	plscatterplot.o \
	pdfannotation.o \
	pdfbarcode.o \
	pdfcffdecoder.o \
	pdfcffindex.o \
	pdfcolour.o \
	pdfdc.o \
	pdfdecode.o \
	pdfdocument.o \
	pdfencoding.o \
	pdfencrypt.o \
	pdffont.o \
	pdffontdata.o \
	pdffontdatacore.o \
	pdffontdataopentype.o \
	pdffontdatatruetype.o \
	pdffontdatatype0.o \
	pdffontdatatype1.o \
	pdffontdescription.o \
	pdffontdetails.o \
	pdffontextended.o \
	pdffontmanager.o \
	pdffontparser.o \
	pdffontparsertruetype.o \
	pdffontparsertype1.o \
	pdffontsubsetcff.o \
	pdffontsubsettruetype.o \
	pdffontvolt.o \
	pdfform.o \
	pdfgradient.o \
	pdfgraphics.o \
	pdfimage.o \
	pdfkernel.o \
	pdflayer.o \
	pdfobjects.o \
	pdfocg.o \
	pdfparser.o \
	pdfpattern.o \
	pdfprint.o \
	pdfrijndael.o \
	pdftemplate.o \
	pdfutility.o \
	pdfxml.o


$(TARGET) : $(OBJECTS)
	ar rs $@ $^

clean:
	rm -rf $(OBJECTS) $(TARGET)
