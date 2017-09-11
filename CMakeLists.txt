Project(wex C CXX)
cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 11)

set(LK_DIR CACHE PATH "LK root directory")
set(CURL_DIR CACHE PATH "Curl root directory")

find_package(wxWidgets REQUIRED net gl core base adv html xml)
include(${wxWidgets_USE_FILE})

if( WIN32 )
  find_package(OpenGL REQUIRED)
endif()

include_directories(include src/freetype/include ${LK_DIR}/include ${CURL_DIR}/include)

if( WIN32 )
  add_definitions(-DLK_USE_WXWIDGETS -D_UNICODE -D_MBCS -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NON_CONFORMING_SWPRINTFS=1 -D_SCL_SECURE_NO_WARNINGS=1 -D__WXMSW__  -DNOPCH -DFT2_BUILD_LIBRARY -DOPJ_STATIC -DFT_CONFIG_MODULES_H=\"slimftmodules.h\" -DFT_CONFIG_OPTIONS_H=\"slimftoptions.h\")
else()
  add_definitions(-DFT2_BUILD_LIBRARY -DOPJ_STATIC -DFT_CONFIG_MODULES_H=\"slimftmodules.h\" -DFT_CONFIG_OPTIONS_H=\"slimftoptions.h\")
endif()

set(WEX_SRC
  src/clipper/clipper.cpp
  src/codeedit.cpp
  src/csv.cpp
  src/dclatex.cpp
  src/dview/dvautocolourassigner.cpp
  src/dview/dvdcctrl.cpp
  src/dview/dvdmapctrl.cpp
  src/dview/dvfilereader.cpp
  src/dview/dvplotctrl.cpp
  src/dview/dvplotctrlsettings.cpp
  src/dview/dvplothelper.cpp
  src/dview/dvpncdfctrl.cpp
  src/dview/dvprofilectrl.cpp
  src/dview/dvscatterplotctrl.cpp
  src/dview/dvselectionlist.cpp
  src/dview/dvstatisticstablectrl.cpp
  src/dview/dvtimeseriesctrl.cpp
  src/dview/dvtimeseriesdataset.cpp
  src/easycurl.cpp
  src/extgrid.cpp
  src/exttext.cpp
  src/exttree.cpp
  src/gleasy.cpp
  src/jsonreader.cpp
  src/jsonval.cpp
  src/jsonwriter.cpp
  src/label.cpp
  src/lkscript.cpp
  src/metro.cpp
  src/mswfatal.cpp
  src/pagelayout.cpp
  src/pageobjects.cpp
  src/plot/mplcontour.cpp
  src/plot/plannotation.cpp
  src/plot/plcontourplot.cpp
  src/plot/ploutdev.cpp
  src/plot/plplot.cpp
  src/plot/plsectorplot.cpp
  src/plot/pltext.cpp
  src/plot/plwindrose.cpp
  src/radiochoice.cpp
  src/diurnal.cpp
  src/numeric.cpp
  src/pdf/pdfannotation.cpp
  src/pdf/pdfbarcode.cpp
  src/pdf/pdfcffdecoder.cpp
  src/pdf/pdfcffindex.cpp
  src/pdf/pdfcolour.cpp
  src/pdf/pdfdc.cpp
  src/pdf/pdfdecode.cpp
  src/pdf/pdfdocument.cpp
  src/pdf/pdfencoding.cpp
  src/pdf/pdfencrypt.cpp
  src/pdf/pdffont.cpp
  src/pdf/pdffontdata.cpp
  src/pdf/pdffontdatacore.cpp
  src/pdf/pdffontdataopentype.cpp
  src/pdf/pdffontdatatruetype.cpp
  src/pdf/pdffontdatatype0.cpp
  src/pdf/pdffontdatatype1.cpp
  src/pdf/pdffontdescription.cpp
  src/pdf/pdffontdetails.cpp
  src/pdf/pdffontextended.cpp
  src/pdf/pdffontmanager.cpp
  src/pdf/pdffontparser.cpp
  src/pdf/pdffontparsertruetype.cpp
  src/pdf/pdffontparsertype1.cpp
  src/pdf/pdffontsubsetcff.cpp
  src/pdf/pdffontsubsettruetype.cpp
  src/pdf/pdffontvolt.cpp
  src/pdf/pdfform.cpp
  src/pdf/pdfgradient.cpp
  src/pdf/pdfgraphics.cpp
  src/pdf/pdfimage.cpp
  src/pdf/pdfkernel.cpp
  src/pdf/pdflayer.cpp
  src/pdf/pdfobjects.cpp
  src/pdf/pdfocg.cpp
  src/pdf/pdfparser.cpp
  src/pdf/pdfpattern.cpp
  src/pdf/pdfprint.cpp
  src/pdf/pdfrijndael.cpp
  src/pdf/pdftemplate.cpp
  src/pdf/pdfutility.cpp
  src/pdf/pdfxml.cpp
  src/plot/plaxis.cpp
  src/plot/plcolourmap.cpp
  src/plot/plbarplot.cpp
  src/plot/plhistplot.cpp
  src/plot/pllineplot.cpp
  src/plot/plplotctrl.cpp
  src/plot/plscatterplot.cpp
  src/registration.cpp
  src/snaplay.cpp
  src/tpdlg.cpp
  src/uiform.cpp
  src/utils.cpp
  src/freetype/src/base/ftbase.c
  src/freetype/src/base/ftbbox.c
  src/freetype/src/base/ftbitmap.c
  src/freetype/src/base/ftfntfmt.c
  src/freetype/src/base/ftgasp.c
  src/freetype/src/base/ftglyph.c
  src/freetype/src/base/ftinit.c
  src/freetype/src/base/ftstroke.c
  src/freetype/src/base/ftsynth.c
  src/freetype/src/base/ftsystem.c
  src/freetype/src/base/fttype1.c
  src/freetype/src/cff/cff.c
  src/freetype/src/cid/type1cid.c
  src/freetype/src/psaux/psaux.c
  src/freetype/src/pshinter/pshinter.c
  src/freetype/src/psnames/psnames.c
  src/freetype/src/raster/raster.c
  src/freetype/src/sfnt/sfnt.c
  src/freetype/src/smooth/smooth.c
  src/freetype/src/truetype/truetype.c
  src/freetype/src/type1/type1.c
  src/freetype/src/base/ftlcdfil.c
  src/plot/qhull/geom.c
  src/plot/qhull/geom2.c
  src/plot/qhull/global.c
  src/plot/qhull/io.c
  src/plot/qhull/libqhull.c
  src/plot/qhull/mem.c
  src/plot/qhull/merge.c
  src/plot/qhull/poly.c
  src/plot/qhull/poly2.c
  src/plot/qhull/qset.c
  src/plot/qhull/random.c
  src/plot/qhull/rboxlib.c
  src/plot/qhull/stat.c
  src/plot/qhull/user.c
  src/plot/qhull/usermem.c
  src/plot/qhull/userprintf.c
  src/plot/qhull/userprintf_rbox.c
  ${LK_DIR}/src/sqlite3.c
)

if( WIN32 )
 list(APPEND WEX_SRC
   src/ole/activex.cpp
   src/ole/excelauto.cpp
   src/ole/iehtmlwin.cpp
   ${LK_DIR}/src/absyn.cpp
   ${LK_DIR}/src/codegen.cpp
   ${LK_DIR}/src/env.cpp
   ${LK_DIR}/src/eval.cpp
   ${LK_DIR}/src/invoke.cpp
   ${LK_DIR}/src/lex.cpp
   ${LK_DIR}/src/parse.cpp
   ${LK_DIR}/src/stdlib.cpp
   ${LK_DIR}/src/vm.cpp
 )
endif()

add_library(wex STATIC ${WEX_SRC})
target_link_libraries(wex ${wxWidgets_LIBRARIES})

set(DVIEW_SRC
  tools/dview/dview.cpp
)

set(MACOSX_BUNDLE_ICON_FILE DView.icns)

if( APPLE )
  set(DVIEW_ICON build_osx/DView.icns)
  set_source_files_properties(${DVIEW_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set(APPS "\${CMAKE_INSTALL_PREFIX}/DView.app")
elseif( WIN32 )
  set(DVIEW_ICON build_vc2013/dview.rc)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/DView.exe")
endif()

add_executable(DView
  WIN32
  MACOSX_BUNDLE
  ${DVIEW_SRC}
  ${DVIEW_ICON}
)

target_link_libraries(DView wex)

if( WIN32 )
  target_link_libraries(DView Winhttp)
endif()

install(TARGETS DView DESTINATION . )

if(WIN32 OR APPLE)
  install(CODE "
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS true)
    fixup_bundle(\"${APPS}\" \"\" \"${wxWidgets_LIBRARY_DIRS}\")
  ")
endif()

