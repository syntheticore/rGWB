// low kill face make ring hole...
//
// Merges two faces f1, f2 by making the loop of the latter a ring into the former. Face2 is removed and
// has only 1 loop.
//

#include "csmeuler_lkfmrh.inl"

#include "csmface.inl"
#include "csmloop.inl"
#include "csmsolid.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// --------------------------------------------------------------------------------

void csmeuler_lkfmrh(struct csmface_t *face_to_add_loop, struct csmface_t **face_to_remove)
{
    struct csmface_t *face_to_remove_loc;
    struct csmsolid_t *face_to_remove_solid;
    struct csmloop_t *loop_iterator;
    unsigned long no_iterations;
    
    face_to_remove_loc = ASSIGN_POINTER_PP_NOT_NULL(face_to_remove, struct csmface_t);
    face_to_remove_solid = csmface_fsolid(face_to_remove_loc);
    
    loop_iterator = csmface_floops(face_to_remove_loc);
    no_iterations = 0;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        
        assert(no_iterations < 10000);
        no_iterations++;
        
        next_loop = csmloop_next(loop_iterator);
        csmface_add_loop_while_removing_from_old(face_to_add_loop, loop_iterator);
        
        loop_iterator = next_loop;
    }
    
    csmface_set_flout(face_to_remove_loc, NULL);
    csmface_set_floops(face_to_remove_loc, NULL);
    csmsolid_remove_face(face_to_remove_solid, &face_to_remove_loc);
}
