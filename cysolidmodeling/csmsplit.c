// Split operation...

#include "csmsplit.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmloop.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmekr.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lmfkrh.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_laringmv.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmopbas.inl"
#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsolid.tli"
#include "csmtolerance.inl"
#include "csmvertex.inl"

#include "a_punter.h"
#include "cyassert.h"
#include "cypespy.h"
#include "defmath.tlh"

ArrEstructura(csmvertex_t);
ArrEstructura(csmedge_t);
ArrEstructura(csmhedge_t);
ArrEstructura(csmface_t);

enum i_position_t
{
    i_POSITION_ABOVE,
    i_POSITION_ON,
    i_POSITION_BELOW
};

struct i_neighborhood_t
{
    struct csmhedge_t *hedge;
    enum i_position_t position;
};

static CYBOOL i_DEBUG = CIERTO;

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_neighborhood_t *, i_create_neighborhod, (struct csmhedge_t *hedge, enum i_position_t position))
{
    struct i_neighborhood_t *neighborhood;
    
    neighborhood = MALLOC(struct i_neighborhood_t);
    
    neighborhood->hedge = hedge;
    neighborhood->position = position;
    
    return neighborhood;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_neighborhood(struct i_neighborhood_t **neighborhood)
{
    assert_no_null(neighborhood);
    assert_no_null(*neighborhood);
    
    FREE_PP(neighborhood, struct i_neighborhood_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_classify_point_respect_to_plane(
                        double x, double y, double z,
                        double A, double B, double C, double D,
                        double tolerance,
                        double *dist_to_plane_opc, enum i_position_t *cl_resp_plane_opc)
{
    double dist_to_plane_loc;
    enum i_position_t cl_resp_plane_loc;
    
    dist_to_plane_loc = csmmath_signed_distance_point_to_plane(x, y, z, A, B, C, D);
    
    switch (csmmath_compare_doubles(dist_to_plane_loc, 0., tolerance))
    {
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            cl_resp_plane_loc = i_POSITION_BELOW;
            break;
            
        case CSMMATH_EQUAL_VALUES:
            
            cl_resp_plane_loc = i_POSITION_ON;
            break;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            cl_resp_plane_loc = i_POSITION_ABOVE;
            break;
            
        default_error();
    }

    ASIGNA_OPC(dist_to_plane_opc, dist_to_plane_loc);
    ASIGNA_OPC(cl_resp_plane_opc, cl_resp_plane_loc);
    
}

// ----------------------------------------------------------------------------------------------------

static double i_classification_tolerance(struct csmhedge_t *hedge)
{
    struct csmface_t *face_hedge;
    double face_tolerance, general_tolerance;
    
    face_hedge = csmopbas_face_from_hedge(hedge);
    face_tolerance = csmface_tolerace(face_hedge);
    general_tolerance = csmtolerance_equal_coords();
    
    return MAX(face_tolerance, general_tolerance);
}

// ----------------------------------------------------------------------------------------------------

static void i_classify_hedge_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D,
                        struct csmvertex_t **vertex_opc, double *dist_to_plane_opc,
                        enum i_position_t *cl_resp_plane_opc)
{
    double tolerance;
    struct csmvertex_t *vertex_loc;
    double x_loc, y_loc, z_loc;
    
    tolerance = i_classification_tolerance(hedge);

    vertex_loc = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex_loc, &x_loc, &y_loc, &z_loc);
    
    i_classify_point_respect_to_plane(
                        x_loc, y_loc, z_loc,
                        A, B, C, D,
                        tolerance,
                        dist_to_plane_opc, cl_resp_plane_opc);

    ASIGNA_OPC(vertex_opc, vertex_loc);
}

// ----------------------------------------------------------------------------------------------------

static enum i_position_t i_classify_hedge_bisector_respect_to_plane(
                        struct csmhedge_t *hedge,
                        double A, double B, double C, double D)
{
    enum i_position_t cl_resp_plane;
    double tolerance;
    struct csmvertex_t *vertex;
    double x, y, z;
    struct csmhedge_t *hedge_prv, *hedge_next;
    struct csmvertex_t *vertex_prv, *vertex_next;
    double x_prv, y_prv, z_prv;
    double x_nxt, y_nxt, z_nxt;
    double Ux_to_prv, Uy_to_prv, Uz_to_prv;
    double Ux_to_nxt, Uy_to_nxt, Uz_to_nxt;
    double Ux_bisector, Uy_bisector, Uz_bisector;
    double x_bisector, y_bisector, z_bisector;
    
    tolerance = i_classification_tolerance(hedge);

    vertex = csmhedge_vertex(hedge);
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    hedge_prv = csmhedge_prev(hedge);
    vertex_prv = csmhedge_vertex(hedge_prv);
    csmvertex_get_coordenadas(vertex_prv, &x_prv, &y_prv, &z_prv);
    
    hedge_next = csmhedge_next(hedge);
    vertex_next = csmhedge_vertex(hedge_next);
    csmvertex_get_coordenadas(vertex_next, &x_nxt, &y_nxt, &z_nxt);
    
    csmmath_vector_between_two_3D_points(x, y, z, x_prv, y_prv, z_prv, &Ux_to_prv, &Uy_to_prv, &Uz_to_prv);
    csmmath_vector_between_two_3D_points(x, y, z, x_nxt, y_nxt, z_nxt, &Ux_to_nxt, &Uy_to_nxt, &Uz_to_nxt);
    
    Ux_bisector = .5 * (Ux_to_prv + Ux_to_nxt);
    Uy_bisector = .5 * (Uy_to_prv + Uy_to_nxt);
    Uz_bisector = .5 * (Uz_to_prv + Uz_to_nxt);
    
    csmmath_make_unit_vector3D(&Ux_bisector, &Uy_bisector, &Uz_bisector);
    
    x_bisector = x + 10. * Ux_bisector;
    y_bisector = y + 10. * Uy_bisector;
    z_bisector = z + 10. * Uz_bisector;
    
    i_classify_point_respect_to_plane(
                        x_bisector, y_bisector, z_bisector,
                        A, B, C, D,
                        tolerance,
                        NULL, &cl_resp_plane);

    return cl_resp_plane;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_equals_vertices(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2)
{
    if (vertex1 == vertex2)
        return CIERTO;
    else
        return FALSO;
}

// ----------------------------------------------------------------------------------------------------

static bool i_equals_vertices2(const struct csmvertex_t *vertex1, const struct csmvertex_t *vertex2)
{
    return i_equals_vertices(vertex1, vertex2) == CIERTO ? true: false;
}

// ----------------------------------------------------------------------------------------------------

static void i_append_vertex_if_not_exists(struct csmvertex_t *vertex, ArrEstructura(csmvertex_t) *set_of_on_vertices)
{
    if (arr_ExisteEstructuraST(set_of_on_vertices, csmvertex_t, vertex, struct csmvertex_t, i_equals_vertices2, NULL) == FALSO)
        arr_AppendPunteroST(set_of_on_vertices, vertex, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(csmvertex_t) *, i_split_edges_by_plane, (
                        struct csmsolid_t *work_solid,
                        double A, double B, double C, double D))
{
    ArrEstructura(csmvertex_t) *set_of_on_vertices;
    struct csmhashtb_iterator(csmedge_t) *edge_iterator;
    
    assert_no_null(work_solid);

    edge_iterator = csmhashtb_create_iterator(work_solid->sedges, csmedge_t);
    set_of_on_vertices = arr_CreaPunteroST(0, csmvertex_t);
    
    while (csmhashtb_has_next(edge_iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        struct csmhedge_t *hedge_pos, *hedge_neg;
        struct csmvertex_t *vertex_pos, *vertex_neg;
        double dist_to_plane_pos, dist_to_plane_neg;
        enum i_position_t cl_resp_plane_pos, cl_resp_plane_neg;
        
        csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
        
        hedge_pos = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
        i_classify_hedge_respect_to_plane(hedge_pos, A, B, C, D, &vertex_pos, &dist_to_plane_pos, &cl_resp_plane_pos);
        
        hedge_neg = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        i_classify_hedge_respect_to_plane(hedge_neg, A, B, C, D, &vertex_neg, &dist_to_plane_neg, &cl_resp_plane_neg);
        
        if (i_DEBUG == CIERTO)
        {
            double x_pos, y_pos, z_pos, x_neg, y_neg, z_neg;

            csmvertex_get_coordenadas(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coordenadas(vertex_neg, &x_neg, &y_neg, &z_neg);
            
            fprintf(
                    stdout,
                    "Split(): Analizing edge %lu (%g, %g, %g) -> (%g, %g, %g)\n",
                    csmedge_id(edge),
                    x_pos, y_pos, z_pos, x_neg, y_neg, z_neg);
        }
        
        if ((cl_resp_plane_pos == i_POSITION_BELOW && cl_resp_plane_neg == i_POSITION_ABOVE)
                || (cl_resp_plane_pos == i_POSITION_ABOVE && cl_resp_plane_neg == i_POSITION_BELOW))
        {
            double t;
            double x_pos, y_pos, z_pos;
            double x_neg, y_neg, z_neg;
            double x_inters, y_inters, z_inters;
            struct csmhedge_t *hedge_neg_next;
            struct csmvertex_t *new_vertex;
            
            t = dist_to_plane_pos / (dist_to_plane_pos - dist_to_plane_neg);
            
            csmvertex_get_coordenadas(vertex_pos, &x_pos, &y_pos, &z_pos);
            csmvertex_get_coordenadas(vertex_neg, &x_neg, &y_neg, &z_neg);
            
            x_inters = x_pos + t * (x_neg - x_pos);
            y_inters = y_pos + t * (y_neg - y_pos);
            z_inters = z_pos + t * (z_neg - z_pos);
            
            hedge_neg_next = csmhedge_next(hedge_neg);
            assert(vertex_pos == csmhedge_vertex(hedge_neg_next));
            
            csmeuler_lmev(hedge_pos, hedge_neg_next, x_inters, y_inters, z_inters, &new_vertex, NULL, NULL, NULL);
            arr_AppendPunteroST(set_of_on_vertices, new_vertex, csmvertex_t);
            
            if (i_DEBUG == CIERTO)
                fprintf(stdout, "\tIntersection at (%g, %g, %g), New Vertex Id: %lu\n", x_inters, y_inters, z_inters, csmvertex_id(new_vertex));
        }
        else
        {
            if (cl_resp_plane_pos == i_POSITION_ON)
            {
                i_append_vertex_if_not_exists(vertex_pos, set_of_on_vertices);
                
                if (i_DEBUG == CIERTO)
                    fprintf(stdout, "\tSplit(): Already On Vertex %lu\n", csmvertex_id(vertex_pos));
            }
            
            if (cl_resp_plane_neg == i_POSITION_ON)
            {
                i_append_vertex_if_not_exists(vertex_neg, set_of_on_vertices);
                
                if (i_DEBUG == CIERTO)
                    fprintf(stdout, "\tSplit(): Already On Vertex %lu\n", csmvertex_id(vertex_neg));
            }
        }
    }
    
    csmhashtb_free_iterator(&edge_iterator, csmedge_t);
    
    return set_of_on_vertices;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(i_neighborhood_t) *, i_initial_vertex_neighborhood, (
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D))
{
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    register struct csmhedge_t *hedge_iterator, *vertex_hedge;
    unsigned long num_iters;
    
    vertex_neighborhood = arr_CreaPunteroST(0, i_neighborhood_t);
    
    vertex_hedge = csmvertex_hedge(vertex);
    hedge_iterator = vertex_hedge;
    
    num_iters = 0;
    
    do
    {
        struct csmhedge_t *hedge_next;
        enum i_position_t cl_resp_plane;
        struct i_neighborhood_t *hedge_neighborhood;
        
        assert(num_iters < 10000);
        num_iters++;
        
        hedge_next = csmhedge_next(hedge_iterator);
        i_classify_hedge_respect_to_plane(hedge_next, A, B, C, D, NULL, NULL, &cl_resp_plane);
        
        hedge_neighborhood = i_create_neighborhod(hedge_iterator, cl_resp_plane);
        arr_AppendPunteroST(vertex_neighborhood, hedge_neighborhood, i_neighborhood_t);
        
        if (csmopbas_is_convex_hedge(hedge_iterator) == FALSO)
        {
            cl_resp_plane = i_classify_hedge_bisector_respect_to_plane(hedge_iterator, A, B, C, D);
            hedge_neighborhood = i_create_neighborhod(hedge_iterator, cl_resp_plane);
        }
        
        hedge_iterator = csmhedge_next(csmopbas_mate(hedge_iterator));
    }
    while (hedge_iterator != vertex_hedge);
    
    return vertex_neighborhood;
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sector_vertex_neighborhood(
                        struct i_neighborhood_t *hedge_neighborhood,
                        struct i_neighborhood_t *next_hedge_neighborhood,
                        double A, double B, double C, double D,
                        double tolerance_coplanarity)
{
    struct csmface_t *face;
    CYBOOL same_orientation;
    
    assert_no_null(hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    face = csmopbas_face_from_hedge(hedge_neighborhood->hedge);
    
    if (csmface_is_coplanar_to_plane(face, A, B, C, D, tolerance_coplanarity, &same_orientation) == CIERTO)
    {
        if (same_orientation == CIERTO)
        {
            hedge_neighborhood->position = i_POSITION_BELOW;
            next_hedge_neighborhood->position = i_POSITION_BELOW;
        }
        else
        {
            hedge_neighborhood->position = i_POSITION_ABOVE;
            next_hedge_neighborhood->position = i_POSITION_ABOVE;
        }
    }
}
    
// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_sectors_vertex_neighborhood(
                        double A, double B, double C, double D,
                        ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    double tolerance_coplanarity;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    tolerance_coplanarity = csmtolerance_coplanarity();
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_sector_vertex_neighborhood(
                        hedge_neighborhood,
                        next_hedge_neighborhood,
                        A, B, C, D,
                        tolerance_coplanarity);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edge_vertex_neighborhood(
                        struct i_neighborhood_t *hedge_neighborhood,
                        struct i_neighborhood_t *prev_hedge_neighborhood,
                        struct i_neighborhood_t *next_hedge_neighborhood)
{
    assert_no_null(hedge_neighborhood);
    assert_no_null(prev_hedge_neighborhood);
    assert_no_null(next_hedge_neighborhood);
    
    if (hedge_neighborhood->position == i_POSITION_ON)
    {
        enum i_position_t new_position;

        if (prev_hedge_neighborhood->position == i_POSITION_ABOVE && next_hedge_neighborhood->position == i_POSITION_ABOVE)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->position == i_POSITION_ABOVE && next_hedge_neighborhood->position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_BELOW;
        }
        else if (prev_hedge_neighborhood->position == i_POSITION_BELOW && next_hedge_neighborhood->position == i_POSITION_BELOW)
        {
            new_position = i_POSITION_ABOVE;
        }
        else
        {
            assert(prev_hedge_neighborhood->position == i_POSITION_BELOW && next_hedge_neighborhood->position == i_POSITION_ABOVE);
            new_position = i_POSITION_BELOW;
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_reclassify_on_edges_vertex_neighborhood(
                        double A, double B, double C, double D,
                        ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    unsigned long i, num_sectors;
    double tolerance_coplanarity;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    tolerance_coplanarity = csmtolerance_coplanarity();
    
    for (i = 0; i < num_sectors; i++)
    {
        struct i_neighborhood_t *hedge_neighborhood;
        unsigned long prev_idx;
        struct i_neighborhood_t *prev_hedge_neighborhood;
        unsigned long next_idx;
        struct i_neighborhood_t *next_hedge_neighborhood;
        
        hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
        
        prev_idx = (num_sectors + i - 1) % num_sectors;
        prev_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, prev_idx, i_neighborhood_t);
        
        next_idx = (i + 1) % num_sectors;
        next_hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, next_idx, i_neighborhood_t);
        
        i_reclassify_on_edge_vertex_neighborhood(
                        hedge_neighborhood,
                        prev_hedge_neighborhood,
                        next_hedge_neighborhood);
    }
}

// ----------------------------------------------------------------------------------------------------

static enum i_position_t i_sector_position(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    const struct i_neighborhood_t *hedge_neighborhood;
    
    hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
    assert_no_null(hedge_neighborhood);
    
    return hedge_neighborhood->position;    
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_below_above_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_BELOW)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_ABOVE)
            return CIERTO;
        else
            return FALSO;
    }
    else
    {
        return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_could_locate_begin_sequence(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long *start_idx)
{
    CYBOOL found;
    unsigned long start_idx_loc;
    unsigned long i, num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    assert_no_null(start_idx);
    
    found = FALSO;
    start_idx_loc = ULONG_MAX;
    
    for (i = 0; i < num_sectors; i++)
    {
        if (i_is_below_above_sequence_at_index(vertex_neighborhood, i) == CIERTO)
        {
            found = CIERTO;
            start_idx_loc = i;
            break;
        }
    }
    
    *start_idx = start_idx_loc;
    
    return found;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_above_below_sequence_at_index(const ArrEstructura(i_neighborhood_t) *vertex_neighborhood, unsigned long idx)
{
    unsigned long num_sectors;
    
    num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
    assert(num_sectors > 0);
    
    if (i_sector_position(vertex_neighborhood, idx) == i_POSITION_ABOVE)
    {
        if (i_sector_position(vertex_neighborhood, (idx + 1) % num_sectors) == i_POSITION_BELOW)
            return CIERTO;
        else
            return FALSO;
    }
    else
    {
        return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex_neighborhood(const char *description, const struct csmvertex_t *vertex, ArrEstructura(i_neighborhood_t) *vertex_neighborhood)
{
    if (i_DEBUG == CIERTO)
    {
        double x, y, z;
        unsigned long i, num_sectors;
        
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        fprintf(stdout, "Split(): Vertex neighborhood [%s]: %lu (%g, %g, %g): ", description, csmvertex_id(vertex), x, y, z);
        
        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        
        for (i = 0; i < num_sectors; i++)
        {
            struct i_neighborhood_t *hedge_neighborhood;
            
            hedge_neighborhood = arr_GetPunteroST(vertex_neighborhood, i, i_neighborhood_t);
            assert_no_null(hedge_neighborhood);
            
            fprintf(stdout, "(hedge = %lu, classification: %d): ", csmhedge_id(hedge_neighborhood->hedge), hedge_neighborhood->position);
        }
        
        fprintf(stdout, "\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(
                        struct csmvertex_t *vertex,
                        double A, double B, double C, double D,
                        ArrEstructura(csmedge_t) *set_of_null_edges)
{
    ArrEstructura(i_neighborhood_t) *vertex_neighborhood;
    unsigned long start_idx;
    
    vertex_neighborhood = i_initial_vertex_neighborhood(vertex, A, B, C, D);
    i_print_debug_info_vertex_neighborhood("Initial", vertex, vertex_neighborhood);
    
    i_reclassify_on_sectors_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Sectors", vertex, vertex_neighborhood);
    
    i_reclassify_on_edges_vertex_neighborhood(A, B, C, D, vertex_neighborhood);
    i_print_debug_info_vertex_neighborhood("After Reclassify On Edges", vertex, vertex_neighborhood);
    
    if (i_could_locate_begin_sequence(vertex_neighborhood, &start_idx) == CIERTO)
    {
        unsigned long num_sectors;
        unsigned long idx;
        struct i_neighborhood_t *head_neighborhood;
        double x_split, y_split, z_split;
        unsigned long num_iters;
        CYBOOL process_next_sequence;

        num_sectors = arr_NumElemsPunteroST(vertex_neighborhood, i_neighborhood_t);
        assert(num_sectors > 0);
        
        idx = start_idx;
        
        head_neighborhood = arr_GetPunteroST(vertex_neighborhood, start_idx, i_neighborhood_t);
        assert_no_null(head_neighborhood);
        
        csmvertex_get_coordenadas(csmhedge_vertex(head_neighborhood->hedge), &x_split, &y_split, &z_split);
        
        num_iters = 0;
        process_next_sequence = CIERTO;
        
        while (process_next_sequence == CIERTO)
        {
            struct i_neighborhood_t *tail_neighborhood;
            struct csmedge_t *null_edge;
            
            assert(num_iters < 100000);
            num_iters++;
            
            while (i_is_above_below_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
            }
            
            tail_neighborhood = arr_GetPunteroST(vertex_neighborhood, idx, i_neighborhood_t);
            assert_no_null(tail_neighborhood);
            
            if (i_DEBUG == CIERTO)
            {
                fprintf(
                        stdout,
                        "\tInserting null edge at (%g, %g, %g) from hedge %lu to hedge %lu.\n",
                        x_split, y_split, z_split,
                        csmhedge_id(head_neighborhood->hedge),
                        csmhedge_id(tail_neighborhood->hedge));
            }
            
            csmeuler_lmev(head_neighborhood->hedge, tail_neighborhood->hedge, x_split, y_split, z_split, NULL, &null_edge, NULL, NULL);
            arr_AppendPunteroST(set_of_null_edges, null_edge, csmedge_t);
            
            while (i_is_below_above_sequence_at_index(vertex_neighborhood, idx) == FALSO)
            {
                assert(num_iters < 100000);
                num_iters++;
                
                idx = (idx + 1) % num_sectors;
                
                if (idx == start_idx)
                {
                    process_next_sequence = FALSO;
                    break;
                }
            }
        }
    }
    
    arr_DestruyeEstructurasST(&vertex_neighborhood, i_free_neighborhood, i_neighborhood_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static ArrEstructura(csmedge_t) *, i_insert_nulledges_to_split_solid, (
                        double A, double B, double C, double D,
                        ArrEstructura(csmvertex_t) *set_of_on_vertices))
{
    ArrEstructura(csmedge_t) *set_of_null_edges;
    unsigned long i, num_vertices;
    
    num_vertices = arr_NumElemsPunteroST(set_of_on_vertices, csmvertex_t);
    
    set_of_null_edges = arr_CreaPunteroST(0, csmedge_t);
    
    for (i = 0; i < num_vertices; i++)
    {
        struct csmvertex_t *vertex;
        
        vertex = arr_GetPunteroST(set_of_on_vertices, i, csmvertex_t);
        i_insert_nulledges_to_split_solid_at_on_vertex_neihborhood(vertex, A, B, C, D, set_of_null_edges);
    }
    
    return set_of_null_edges;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_hedges_are_neighbors(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *face_he1, *face_he2;
    
    face_he1 = csmopbas_face_from_hedge(he1);
    face_he2 = csmopbas_face_from_hedge(he2);
    
    if (face_he1 != face_he2)
    {
        return FALSO;
    }
    else
    {
        struct csmedge_t *edge_he1, *edge_he2;
        struct csmhedge_t *he1_edge_he1, *he2_edge_he1;
        struct csmhedge_t *he1_edge_he2, *he2_edge_he2;
        
        edge_he1 = csmhedge_edge(he1);
        he1_edge_he1 = csmedge_hedge_lado(edge_he1, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he1 = csmedge_hedge_lado(edge_he1, CSMEDGE_LADO_HEDGE_NEG);
        
        edge_he2 = csmhedge_edge(he2);
        he1_edge_he2 = csmedge_hedge_lado(edge_he2, CSMEDGE_LADO_HEDGE_POS);
        he2_edge_he2 = csmedge_hedge_lado(edge_he2, CSMEDGE_LADO_HEDGE_NEG);
        
        if (he1 == he1_edge_he1 && he2 == he2_edge_he2)
            return CIERTO;
        else if (he1 == he2_edge_he1 && he2 == he1_edge_he2)
            return CIERTO;
        else
            return FALSO;
    }
}

// ----------------------------------------------------------------------------------------------------

static enum comparac_t i_compare_coords(double coord1, double coord2, double tolerance)
{
    switch (csmmath_compare_doubles(coord1, coord2, tolerance))
    {
        case CSMMATH_VALUE1_LESS_THAN_VALUE2:
            
            return comparac_PRIMERO_MENOR;
            
        case CSMMATH_EQUAL_VALUES:
            
            return comparac_IGUALES;
            
        case CSMMATH_VALUE1_GREATER_THAN_VALUE2:
            
            return comparac_PRIMERO_MAYOR;
            
        default_error();
    }
}

// ----------------------------------------------------------------------------------------------------

static enum comparac_t i_compare_edges_by_coord(const struct csmedge_t *edge1, const struct csmedge_t *edge2)
{
    enum comparac_t comparacion;
    const struct csmhedge_t *he1_edge1, *he1_edge2;
    const struct csmvertex_t *vertex1, *vertex2;
    double x1, y1, z1, x2, y2, z2;
    double tolerance;
    
    he1_edge1 = csmedge_hedge_lado_const(edge1, CSMEDGE_LADO_HEDGE_POS);
    vertex1 = csmhedge_vertex_const(he1_edge1);
    csmvertex_get_coordenadas(vertex1, &x1, &y1, &z1);
    
    he1_edge2 = csmedge_hedge_lado_const(edge2, CSMEDGE_LADO_HEDGE_POS);
    vertex2 = csmhedge_vertex_const(he1_edge2);
    csmvertex_get_coordenadas(vertex2, &x2, &y2, &z2);

    tolerance = csmtolerance_equal_coords();
    
    comparacion = i_compare_coords(x1, x2, tolerance);
    
    if (comparacion == comparac_IGUALES)
    {
        comparacion = i_compare_coords(y1, y2, tolerance);
        
        if (comparacion == comparac_IGUALES)
            comparacion = i_compare_coords(z1, z2, tolerance);
    }
    
    return comparacion;
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_can_join_he(struct csmhedge_t *he, ArrEstructura(csmhedge_t) *loose_ends, struct csmhedge_t **matching_loose_end)
{
    CYBOOL can_join;
    struct csmhedge_t *matching_loose_end_loc;
    unsigned long i, no_loose_ends;
    
    assert_no_null(matching_loose_end);
    
    no_loose_ends = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    can_join = FALSO;
    matching_loose_end_loc = NULL;
    
    for (i = 0; i < no_loose_ends; i++)
    {
        struct csmhedge_t *loose_end;
        
        loose_end = arr_GetPunteroST(loose_ends, i, csmhedge_t);
        
        if (i_hedges_are_neighbors(he, loose_end) == CIERTO)
        {
            can_join = CIERTO;
            matching_loose_end_loc = loose_end;
            
            arr_BorrarEstructuraST(loose_ends, i, NULL, csmhedge_t);
            break;
        }
    }
    
    if (can_join == FALSO)
    {
        matching_loose_end_loc = NULL;
        arr_AppendPunteroST(loose_ends, he, csmhedge_t);
    }
    
    *matching_loose_end = matching_loose_end_loc;
    
    return can_join;
}

// ----------------------------------------------------------------------------------------------------

static void i_join_hedges(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *old_face, *new_face;
    struct csmhedge_t *he1_next, *he1_next_next;
    
    old_face = csmopbas_face_from_hedge(he1);
    
    if (csmhedge_loop(he1) == csmhedge_loop(he2))
    {
        struct csmhedge_t *he1_prev, *he1_prev_prev;
        
        he1_prev = csmhedge_prev(he1);
        he1_prev_prev = csmhedge_prev(he1);
        
        if (he1_prev_prev != he2)
        {
            struct csmhedge_t *he2_next;
            
            he2_next = csmhedge_next(he2);
            csmeuler_lmef(he1, he2_next, &new_face, NULL, NULL);
            
            if (i_DEBUG == CIERTO)
            {
                fprintf(stdout, "Split(): (SAME LOOP) joining edges (%lu, %lu) with LMEF, new face %lu.\n", csmhedge_id(he1), csmhedge_id(he2), csmface_id(new_face));
                csmsolid_print_debug(csmface_fsolid(new_face), CIERTO);
            }
        }
        else
        {
            if (i_DEBUG == CIERTO)
                fprintf(stdout, "Split(): (SAME LOOP) joining edges (%lu, %lu). Already connected.\n", csmhedge_id(he1), csmhedge_id(he2));
            
            new_face = NULL;
        }
    }
    else
    {
        struct csmhedge_t *he2_next;
        
        new_face = NULL;
        
        he2_next = csmhedge_next(he2);
        csmeuler_lmekr(he1, he2_next, NULL, NULL);
        
        if (i_DEBUG == CIERTO)
            fprintf(stdout, "Split(): (DIFFERENT LOOP) joining edges (%lu, %lu) with LMEKR, lmekr he1 with %lu\n", csmhedge_id(he1), csmhedge_id(he2), csmhedge_id(he2_next));
    }
    
    he1_next = csmhedge_next(he1);
    he1_next_next = csmhedge_next(he1_next);
    
    if (he1_next_next != he2)
    {
        struct csmloop_t *old_face_floops;
        struct csmface_t *second_new_face;

        if (i_DEBUG == CIERTO)
            fprintf(stdout, "Split(): joining edges (%lu, %lu) with LMEF between (%lu, %lu)\n", csmhedge_id(he1), csmhedge_id(he2), csmhedge_id(he2), csmhedge_id(he1_next));
        
        csmeuler_lmef(he2, he1_next, &second_new_face, NULL, NULL);

        if (i_DEBUG == CIERTO)
        {
            fprintf(stdout, "\tFace %lu created\n", csmface_id(second_new_face));
            csmsolid_print_debug(csmface_fsolid(second_new_face), CIERTO);
        }
        
        old_face_floops = csmface_floops(old_face);
        
        if (new_face != NULL && csmloop_next(old_face_floops) != NULL)
            csmeuler_laringmv(old_face, new_face);
    }
}

// ----------------------------------------------------------------------------------------------------

static bool i_is_same_edge_by_ptr(const struct csmedge_t *edge1, const struct csmedge_t *edge2)
{
    return edge1 == edge2;
}

// ----------------------------------------------------------------------------------------------------

static void i_cut_he(
                    struct csmhedge_t *hedge,
                    ArrEstructura(csmedge_t) *set_of_null_edges,
                    ArrEstructura(csmface_t) *set_of_null_faces,
                    unsigned long *no_null_edges_deleted)
{
    unsigned long idx;
    struct csmsolid_t *solid;
    struct csmedge_t *edge;
    struct csmhedge_t *he1_edge, *he2_edge;
    
    assert_no_null(no_null_edges_deleted);
    
    solid = csmopbas_solid_from_hedge(hedge);
    
    edge = csmhedge_edge(hedge);
    he1_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2_edge = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);

    idx = arr_BuscarEstructuraST(set_of_null_edges, csmedge_t, edge, struct csmedge_t, i_is_same_edge_by_ptr);
    assert(idx != ULONG_MAX);
    arr_BorrarEstructuraST(set_of_null_edges, idx, NULL, csmedge_t);
    (*no_null_edges_deleted)++;
    
    if (csmhedge_loop(he1_edge) == csmhedge_loop(he2_edge))
    {
        struct csmface_t *null_face;
        
        null_face = csmopbas_face_from_hedge(hedge);
        arr_AppendPunteroST(set_of_null_faces, null_face, csmface_t);
        
        if (i_DEBUG == CIERTO)
        {
            fprintf(stdout, "Split(): (CUTTING HE)  (%lu, %lu) with LKEMR\n", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
            csmsolid_print_debug(csmopbas_solid_from_hedge(hedge), CIERTO);
        }
        
        csmeuler_lkemr(&he1_edge, &he2_edge, NULL, NULL);
        
        if (i_DEBUG == CIERTO)
            csmsolid_print_debug(solid, CIERTO);
    }
    else
    {
        if (i_DEBUG == CIERTO)
            fprintf(stdout, "Split(): (CUTTING HE)  (%lu, %lu) with LKEF\n", csmhedge_id(he1_edge), csmhedge_id(he2_edge));
        
        csmeuler_lkef(&he1_edge, &he2_edge);
        
        if (i_DEBUG == CIERTO)
            csmsolid_print_debug(solid, CIERTO);
    }
}

// ----------------------------------------------------------------------------------------------------

static CYBOOL i_is_loose_end(struct csmhedge_t *hedge, ArrEstructura(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    no_loose_end = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        if (arr_GetPunteroST(loose_ends, i, csmhedge_t) == hedge)
            return CIERTO;
    }
    
    return FALSO;
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_loose_ends(const ArrEstructura(csmhedge_t) *loose_ends)
{
    unsigned long i, no_loose_end;
    
    fprintf(stdout, "Split(): Loose ends [");
    
    no_loose_end = arr_NumElemsPunteroST(loose_ends, csmhedge_t);
    
    for (i = 0; i < no_loose_end; i++)
    {
        const struct csmhedge_t *hedge;
        
        hedge = arr_GetPunteroConstST(loose_ends, i, csmhedge_t);
        assert_no_null(hedge);
        
        if (i > 0)
            fprintf(stdout, ", ");
        
        fprintf(stdout, "%lu", csmhedge_id(hedge));
    }
    
    fprintf(stdout, "]\n");
}

// ----------------------------------------------------------------------------------------------------

static void i_print_set_of_null_edges(const ArrEstructura(csmedge_t) *set_of_null_edges)
{
    unsigned long i, num_null_edges;
    
    num_null_edges = arr_NumElemsPunteroST(set_of_null_edges, csmedge_t);
    
    fprintf(stdout, "Set of null edges:\n");
    
    for (i = 0; i < num_null_edges; i++)
    {
        const struct csmedge_t *edge;
        const struct csmhedge_t *he1, *he2;
        const struct csmvertex_t *vertex1;
        double x, y, z;
        
        edge = arr_GetPunteroST(set_of_null_edges, i, csmedge_t);
        assert_no_null(edge);
        
        he1 = csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_POS);
        he2 = csmedge_hedge_lado_const(edge, CSMEDGE_LADO_HEDGE_NEG);
        
        vertex1 = csmhedge_vertex_const(he1);
        csmvertex_get_coordenadas(vertex1, &x, &y, &z);
        
        
        fprintf(
                stdout,
                "\t[%lu] (%5.3g, %5.3g, %5.3g)\t[he1, loop, face] = (%lu, %lu, %lu)\t[he2, loop, face] = (%lu, %lu, %lu)) \n",
                csmedge_id(edge), x, y, z,
                csmhedge_id(he1), csmloop_id(csmhedge_loop((struct csmhedge_t *)he1)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he1)),
                csmhedge_id(he2), csmloop_id(csmhedge_loop((struct csmhedge_t *)he2)), csmface_id(csmopbas_face_from_hedge((struct csmhedge_t *)he2)));
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_join_null_edges(ArrEstructura(csmedge_t) *set_of_null_edges, ArrEstructura(csmface_t) **set_of_null_faces)
{
    ArrEstructura(csmface_t) *set_of_null_faces_loc;
    ArrEstructura(csmhedge_t) *loose_ends;
    unsigned long i, no_null_edges;
    unsigned long no_null_edges_deleted;
    
    arr_QSortPunteroST(set_of_null_edges, i_compare_edges_by_coord, csmedge_t);
    no_null_edges = arr_NumElemsPunteroST(set_of_null_edges, csmedge_t);
    assert(no_null_edges > 0);
    assert_no_null(set_of_null_faces);
    
    set_of_null_faces_loc = arr_CreaPunteroST(0, csmface_t);
    loose_ends = arr_CreaPunteroST(0, csmhedge_t);
    
    no_null_edges_deleted = 0;
    
    for (i = 0; i < no_null_edges; i++)
    {
        unsigned long idx;
        struct csmedge_t *next_edge;
        struct csmhedge_t *he1_next_edge, *he2_next_edge;
        struct csmhedge_t *matching_loose_end_he1, *matching_loose_end_he2;
        
        if (i_DEBUG == CIERTO)
            i_print_set_of_null_edges(set_of_null_edges);
        
        idx = i - no_null_edges_deleted;
        next_edge = arr_GetPunteroST(set_of_null_edges, idx, csmedge_t);
        
        he1_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_LADO_HEDGE_POS);
        he2_next_edge = csmedge_hedge_lado(next_edge, CSMEDGE_LADO_HEDGE_NEG);
     
        if (i_can_join_he(he1_next_edge, loose_ends, &matching_loose_end_he1) == CIERTO)
        {
            i_join_hedges(matching_loose_end_he1, he1_next_edge);
            
            if (i_is_loose_end(csmopbas_mate(matching_loose_end_he1), loose_ends) == FALSO)
                i_cut_he(matching_loose_end_he1, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he1 = NULL;
        }

        if (i_can_join_he(he2_next_edge, loose_ends, &matching_loose_end_he2) == CIERTO)
        {
            i_join_hedges(matching_loose_end_he2, he2_next_edge);
            
            if (i_is_loose_end(csmopbas_mate(matching_loose_end_he2), loose_ends) == FALSO)
                i_cut_he(matching_loose_end_he2, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        }
        else
        {
            matching_loose_end_he2 = NULL;
        }
        
        if (matching_loose_end_he1 != NULL && matching_loose_end_he2 != NULL)
            i_cut_he(he1_next_edge, set_of_null_edges, set_of_null_faces_loc, &no_null_edges_deleted);
        
        if (i_DEBUG == CIERTO)
        {
            i_print_debug_info_loose_ends(loose_ends);
            csmsolid_print_debug(csmopbas_solid_from_hedge(he1_next_edge), CIERTO);
        }
    }
    
    *set_of_null_faces = set_of_null_faces_loc;
    
    assert(arr_NumElemsPunteroST(set_of_null_edges, csmedge_t) == 0);
    assert(arr_NumElemsPunteroST(loose_ends, csmhedge_t) == 0);
    arr_DestruyeEstructurasST(&loose_ends, NULL, csmhedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_convert_inner_loops_of_null_faces_to_faces_solid_below(ArrEstructura(csmface_t) *set_of_null_faces)
{
    unsigned long i, no_null_faces;
    
    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *null_face;
        struct csmloop_t *floops, *loop_to_move;
        struct csmface_t *new_face;
        
        null_face = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
        
        floops = csmface_floops(null_face);
        loop_to_move = floops;
        
        do
        {
            struct csmloop_t *next_loop;

            next_loop = csmloop_next(loop_to_move);
            
            if (loop_to_move != csmface_flout(null_face))
            {
                csmeuler_lmfkrh(loop_to_move, &new_face);
                arr_AppendPunteroST(set_of_null_faces, new_face, csmface_t);
            }
            
            loop_to_move = next_loop;
                
        } while (loop_to_move != NULL);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_move_face_to_solid(
                        unsigned long recursion_level,
                        struct csmface_t *face, struct csmsolid_t *face_solid,
                        struct csmsolid_t *destination_solid)
{
    assert(recursion_level < 10000);
    
    if (csmface_fsolid(face) != destination_solid)
    {
        register struct csmloop_t *loop_iterator;
    
        assert(csmface_fsolid(face) == face_solid);
        
        if (i_DEBUG == CIERTO)
        {
            fprintf(stdout, "Split(): Moving face %lu (solid %p) to solid %p\n", csmface_id(face), csmface_fsolid(face), destination_solid);
            assert(csmface_fsolid(face) == face_solid);
        }
        
        csmsolid_move_face_to_solid(face_solid, face, destination_solid);
        
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            register struct csmhedge_t *loop_ledge, *he_iterator;
            unsigned long no_iters;
            
            loop_ledge = csmloop_ledge(loop_iterator);
            he_iterator = loop_ledge;
            no_iters = 0;
            
            do
            {
                struct csmhedge_t *he_mate_iterator;
                struct csmface_t *he_mate_iterator_face;
                struct csmsolid_t *he_mate_iterator_face_solid;
                
                assert(no_iters < 10000);
                no_iters++;
                
                he_mate_iterator = csmopbas_mate(he_iterator);
                he_mate_iterator_face = csmopbas_face_from_hedge(he_mate_iterator);
                he_mate_iterator_face_solid = csmface_fsolid(he_mate_iterator_face);
                
                if (he_mate_iterator_face_solid != destination_solid)
                    i_move_face_to_solid(recursion_level + 1, he_mate_iterator_face, he_mate_iterator_face_solid, destination_solid);
                
                he_iterator = csmhedge_next(he_iterator);
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_cleanup_solid(struct csmsolid_t *origin_solid, struct csmsolid_t *destination_solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(destination_solid);

    face_iterator = csmhashtb_create_iterator(destination_solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        struct csmloop_t *loop_iterator;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        loop_iterator = csmface_floops(face);
        
        while (loop_iterator != NULL)
        {
            register struct csmhedge_t *loop_ledge, *he_iterator;
            unsigned long no_iters;
        
            loop_ledge = csmloop_ledge(loop_iterator);
            he_iterator = loop_ledge;
            no_iters = 0;
            
            do
            {
                struct csmedge_t *edge;
                struct csmvertex_t *vertex;
                
                assert(no_iters < 10000);
                no_iters++;
                
                edge = csmhedge_edge(he_iterator);
                vertex = csmhedge_vertex(he_iterator);
                
                if (edge != NULL)
                {
                    if (csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS) == he_iterator)
                        csmsolid_move_edge_to_solid(origin_solid, edge, destination_solid);
                    
                    if (csmvertex_hedge(vertex) == he_iterator)
                        csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                else
                {
                    assert(csmvertex_hedge(vertex) == he_iterator);
                    csmsolid_move_vertex_to_solid(origin_solid, vertex, destination_solid);
                }
                
                he_iterator = csmhedge_next(he_iterator);
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_finish_split(
                        ArrEstructura(csmface_t) *set_of_null_faces,
                        struct csmsolid_t *work_solid,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    unsigned long i, no_null_faces;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;

    no_null_faces = arr_NumElemsPunteroST(set_of_null_faces, csmface_t);
    assert(no_null_faces > 0);
    assert_no_null(solid_above);
    assert_no_null(solid_below);

    if (i_DEBUG == CIERTO)
        csmsolid_print_debug(work_solid, CIERTO);

    i_convert_inner_loops_of_null_faces_to_faces_solid_below(set_of_null_faces);
    assert(2 * no_null_faces == arr_NumElemsPunteroST(set_of_null_faces, csmface_t));
    
    if (i_DEBUG == CIERTO)
        csmsolid_print_debug(work_solid, CIERTO);
    
    solid_above_loc = csmsolid_crea_vacio();
    solid_below_loc = csmsolid_crea_vacio();
    
    for (i = 0; i < no_null_faces; i++)
    {
        struct csmface_t *face_to_solid_above;
        struct csmface_t *face_to_solid_below;
        
        face_to_solid_above = arr_GetPunteroST(set_of_null_faces, i, csmface_t);
        i_move_face_to_solid(0, face_to_solid_above, work_solid, solid_above_loc);
        
        face_to_solid_below = arr_GetPunteroST(set_of_null_faces, i + no_null_faces, csmface_t);
        i_move_face_to_solid(0, face_to_solid_below, work_solid, solid_below_loc);
    }
    
    i_cleanup_solid(work_solid, solid_above_loc);
    i_cleanup_solid(work_solid, solid_below_loc);

    if (i_DEBUG == CIERTO)
    {
        csmsolid_print_debug(work_solid, CIERTO);
        csmsolid_print_debug(solid_above_loc, CIERTO);
        csmsolid_print_debug(solid_below_loc, CIERTO);
    }
    
    *solid_above = solid_above_loc;
    *solid_below = solid_below_loc;
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsplit_does_plane_split_solid(
                        const struct csmsolid_t *solid,
                        double A, double B, double C, double D,
                        struct csmsolid_t **solid_above, struct csmsolid_t **solid_below)
{
    CYBOOL does_plane_split_solid;
    struct csmsolid_t *solid_above_loc, *solid_below_loc;
    struct csmsolid_t *work_solid;
    ArrEstructura(csmvertex_t) *set_of_on_vertices;
    ArrEstructura(csmedge_t) *set_of_null_edges;

    assert_no_null(solid_above);
    assert_no_null(solid_below);
    
    work_solid = csmsolid_duplicate(solid);
    csmsolid_redo_geometric_generated_data(work_solid);
    
    set_of_on_vertices = i_split_edges_by_plane(work_solid, A, B, C, D);
    set_of_null_edges = i_insert_nulledges_to_split_solid(A, B, C, D, set_of_on_vertices);
    
    if (arr_NumElemsPunteroST(set_of_null_edges, csmedge_t) == 0)
    {
        does_plane_split_solid = FALSO;
        
        solid_above_loc = NULL;
        solid_below_loc = NULL;
    }
    else
    {
        ArrEstructura(csmface_t) *set_of_null_faces;
        
        does_plane_split_solid = CIERTO;
        
        i_join_null_edges(set_of_null_edges, &set_of_null_faces);
        i_finish_split(set_of_null_faces, work_solid, &solid_above_loc, &solid_below_loc);

        assert(csmsolid_is_empty(work_solid) == CIERTO);

        arr_DestruyeEstructurasST(&set_of_null_faces, NULL, csmface_t);
    }

    *solid_above = solid_above_loc;
    *solid_below = solid_below_loc;
    
    csmsolid_free(&work_solid);
    arr_DestruyeEstructurasST(&set_of_on_vertices, NULL, csmvertex_t);
    arr_DestruyeEstructurasST(&set_of_null_edges, NULL, csmedge_t);
    
    return does_plane_split_solid;
}