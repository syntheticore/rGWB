// Vertex...

#include "csmvertex.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmvertex_t
{
    unsigned long id;
    
    double x, y, z;
    struct csmhedge_t *hedge;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmvertex_t *, i_crea, (
						unsigned long id,
                        double x, double y, double z,
                        struct csmhedge_t *hedge))
{
    struct csmvertex_t *vertex;
    
    vertex = MALLOC(struct csmvertex_t);
    
    vertex->id = id;

    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    
    vertex->hedge = hedge;

    return vertex;
}

// ----------------------------------------------------------------------------------------------------

struct csmvertex_t *csmvertex_crea(double x, double y, double z, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmhedge_t *hedge;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    
    hedge = NULL;
    
    return i_crea(id, x, y, z, hedge);
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_destruye(struct csmvertex_t **vertex)
{
    assert_no_null(vertex);
    assert_no_null(*vertex);

    FREE_PP(vertex, struct csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

unsigned long csmvertex_id(const struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->id;
}

// ----------------------------------------------------------------------------------------------------

struct csmhedge_t *csmvertex_hedge(struct csmvertex_t *vertex)
{
    assert_no_null(vertex);
    return vertex->hedge;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_hedge(struct csmvertex_t *vertex, struct csmhedge_t *hedge)
{
    assert_no_null(vertex);
    vertex->hedge = hedge;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_get_coordenadas(const struct csmvertex_t *vertex, double *x, double *y, double *z)
{
    assert_no_null(vertex);
    assert_no_null(x);
    assert_no_null(y);
    assert_no_null(z);
    
    *x = vertex->x;
    *y = vertex->y;
    *z = vertex->z;
}

// ----------------------------------------------------------------------------------------------------

void csmvertex_set_coordenadas(struct csmvertex_t *vertex, double x, double y, double z)
{
    assert_no_null(vertex);
    
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
}
