// Edge...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmedge_t *, csmedge_crea, (unsigned long *id_nuevo_elemento));

CONSTRUCTOR(struct csmedge_t *, csmedge_duplicate, (
                        const struct csmedge_t *edge,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));

void csmedge_destruye(struct csmedge_t **edge);

unsigned long csmedge_id(const struct csmedge_t *edge);

void csmedge_reassign_id(struct csmedge_t *edge, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);


// Topología...

struct csmhedge_t *csmedge_hedge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado);
const struct csmhedge_t *csmedge_hedge_lado_const(const struct csmedge_t *edge, enum csmedge_lado_hedge_t lado);

void csmedge_set_edge_lado(struct csmedge_t *edge, enum csmedge_lado_hedge_t lado, struct csmhedge_t *hedge);

void csmedge_remove_hedge(struct csmedge_t *edge, struct csmhedge_t *hedge);

struct csmhedge_t *csmedge_mate(struct csmedge_t *edge, const struct csmhedge_t *hedge);
