// Half-Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhedge_t *, csmhedge_new, (unsigned long *id_new_element));

CONSTRUCTOR(struct csmhedge_t *, csmhedge_duplicate, (
                        const struct csmhedge_t *hedge,
                        struct csmloop_t *loop,
                        unsigned long *id_new_element,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));

CONSTRUCTOR(struct csmhedge_t *, csmhedge_new_from_writeable_hedge, (
                        const struct csmwriteablesolid_hedge_t *w_hedge,
                        struct csmloop_t *loop,
                        struct csmhashtb(csmvertex_t) *svertexs,
                        struct csmhashtb(csmhedge_t) *created_shedges));

unsigned long csmhedge_id(const struct csmhedge_t *hedge);

void csmhedge_reassign_id(struct csmhedge_t *hedge, unsigned long *id_new_element, unsigned long *new_id_opc);

CSMBOOL csmhedge_equal_id(const struct csmhedge_t *hedge1, const struct csmhedge_t *hedge2);


// Topology...

struct csmedge_t *csmhedge_edge(struct csmhedge_t *hedge);
void csmhedge_set_edge(struct csmhedge_t *hedge, struct csmedge_t *edge);

struct csmvertex_t *csmhedge_vertex(struct csmhedge_t *hedge);
const struct csmvertex_t *csmhedge_vertex_const(const struct csmhedge_t *hedge);

void csmhedge_set_vertex(struct csmhedge_t *hedge, struct csmvertex_t *vertex);

struct csmloop_t *csmhedge_loop(struct csmhedge_t *hedge);
void csmhedge_set_loop(struct csmhedge_t *hedge, struct csmloop_t *loop);

// Setop...

void csmhedge_clear_algorithm_mask(struct csmhedge_t *hedge);

void csmhedge_setop_set_loose_end(struct csmhedge_t *hedge, CSMBOOL is_loose_end);
CSMBOOL csmhedge_setop_is_loose_end(const struct csmhedge_t *hedge);


// Lista...

struct csmhedge_t *csmhedge_next(struct csmhedge_t *hedge);
void csmhedge_set_next(struct csmhedge_t *hedge, struct csmhedge_t *next_hedge);

struct csmhedge_t *csmhedge_prev(struct csmhedge_t *hedge);
void csmhedge_set_prev(struct csmhedge_t *hedge, struct csmhedge_t *prev_hedge);
