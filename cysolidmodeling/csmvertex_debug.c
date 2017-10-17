//
//  csmvertex_debug.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmvertex_debug.inl"
#include "csmvertex.tli"

#include "csmassert.inl"
#include "csmdebug.inl"

// ----------------------------------------------------------------------------------------------------

void csmvertex_debug_print_debug_info(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    
    csmdebug_print_debug_info("\tVertex %6lu\t%6.3lf\t%6.3lf\t%6.3lf [%lu]", vertex->id, vertex->x, vertex->y, vertex->z, vertex->algorithm_attrib_mask);
    
    if (vertex->algorithm_attrib_mask & CSMVERTEX_MASK_VERTEX_ON_HOLE_LOOP)
        csmdebug_print_debug_info(" [H] ");
    
    if (vertex->algorithm_attrib_mask & CSMVERTEX_MASK_SETOP_COMMON_VERTEX)
        csmdebug_print_debug_info(" [CV] ");
    
    if (vertex->algorithm_attrib_mask &     CSMVERTEX_MASK_SETOP_VTX_FAC_CLASS)
        csmdebug_print_debug_info(" [VTXFACC] ");
}




