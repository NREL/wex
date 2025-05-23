/***************************************************************************/
/*                                                                         */
/*  pfrgload.h                                                             */
/*                                                                         */
/*    FreeType PFR glyph loader (specification).                           */
/*                                                                         */
/*  Copyright 2002-2015 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __PFRGLOAD_H__
#define __PFRGLOAD_H__

#include "pfrtypes.h"

FT_BEGIN_HEADER


FT_LOCAL(void)
        pfr_glyph_init(PFR_Glyph glyph,
                       FT_GlyphLoader loader);

                FT_LOCAL(void)

pfr_glyph_done( PFR_Glyph
glyph );


FT_LOCAL( FT_Error )
pfr_glyph_load( PFR_Glyph
glyph,
FT_Stream stream,
        FT_ULong
gps_offset,
FT_ULong offset,
        FT_ULong
size );


FT_END_HEADER


#endif /* __PFRGLOAD_H__ */


/* END */
