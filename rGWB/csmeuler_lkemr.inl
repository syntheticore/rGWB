// low level kill edge make ring

#include "csmfwddecl.hxx"

void csmeuler_lkemr(
                struct csmhedge_t **he_to_ring, struct csmhedge_t **he_from_ring,
                struct csmhedge_t **he_to_ring_next, struct csmhedge_t **he_from_ring_next,
                struct csmloop_t **new_loop_opt);
