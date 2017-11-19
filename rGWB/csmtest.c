// Test básicos...

#include "csmtest.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmhedge.inl"
#include "csmsolid.inl"
#include "csmeuler_mvfs.inl"
#include "csmeuler_kvfs.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkev.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lmekr.inl"

#include "csmArrPoint2D.h"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmhashtb.inl"
#include "csmexplode.h"
#include "csmglue.h"
#include "csmface.inl"
#include "csmmath.inl"
#include "csmmath.tli"
#include "csmsetop.h"
#include "csmsolid.h"
#include "csmsolid_debug.inl"
#include "csmsweep.h"
#include "csmsplit.h"
#include "csmquadrics.h"
#include "csmassert.inl"
#include "csmstring.inl"
#include "csmshape2d.h"
#include "csmbasicshape2d.h"

#include "csmdebug.inl"
#include "csmviewer.inl"
#include "csmmath.tli"
#include "csmmaterial.h"
#include "csmtolerance.inl"

#include "csmarrayc.inl"
#include "csmtest_array.inl"

// ------------------------------------------------------------------------------------------

static void i_assign_flat_material_to_solid(float r, float g, float b, struct csmsolid_t *solid)
{
    struct csmmaterial_t *material;
    
    material = csmmaterial_new_flat_material(r, g, b, 1.);
    csmsolid_set_visualization_material(solid, &material);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_destruye_solido_vacio(void)
{
    struct csmsolid_t *solido;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, NULL);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_basico_solido_una_arista(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *hedge;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hedge);
    
    csmeuler_lmev(hedge, hedge, 1., 0., 0., NULL, NULL, &he1, &he2);
    
    csmeuler_lkev(&he1, &he2, &he1, NULL, &he2, NULL);
    assert(he1 == he2);

    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_lamina(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *initial_hedge, *hedge_from_vertex1, *hedge_from_vertex2, *hedge_from_vertex3;
    struct csmface_t *initial_face, *new_face;
    struct csmhedge_t *he_pos, *he_neg;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &initial_hedge);
    initial_face = csmopbas_face_from_hedge(initial_hedge);
    
    csmeuler_lmev_strut_edge(initial_hedge, 1., 0., 1., &hedge_from_vertex1);
    csmeuler_lmev_strut_edge(hedge_from_vertex1, 1., 1., 1., &hedge_from_vertex2);
    csmeuler_lmev_strut_edge(hedge_from_vertex2, 0., 1., 0., &hedge_from_vertex3);
    csmeuler_lmef(initial_hedge, hedge_from_vertex3, &new_face, &he_pos, &he_neg);
    
    {
        struct csmtolerance_t *tolerance;
        
        tolerance = csmtolerance_new();
        
        csmsolid_redo_geometric_generated_data(solido);
        
        assert(initial_face != new_face);

        assert(csmface_contains_point(initial_face, 0.5, 0.5, 0.5, tolerance, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(initial_face, 1., 1., 1., tolerance, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(initial_face, 5., 5., 0., tolerance, NULL, NULL, NULL, NULL) == CSMFALSE);
        
        assert(csmface_contains_point(new_face, 0.5, 0.5, 0.5, tolerance, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(new_face, 1., 1., 1., tolerance, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(new_face, 5., 5., 0., tolerance, NULL, NULL, NULL, NULL) == CSMFALSE);
        
        csmtolerance_free(&tolerance);
    }
    
    csmeuler_lkef(&he_pos, &he_neg);
    assert(he_pos != NULL);
    assert(he_neg != NULL);
    assert(he_pos == hedge_from_vertex3);
    
    he1 = he_pos;
    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex2);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex1);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == he2);
    assert(he1 == initial_hedge);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_lamina_con_hueco(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *initial_hedge, *hedge_from_vertex1, *hedge_from_vertex2, *hedge_from_vertex3;
    struct csmhedge_t *he_pos, *he_neg;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &initial_hedge);
    
    csmeuler_lmev_strut_edge(initial_hedge, 10., 0., 0., &hedge_from_vertex1);
    csmeuler_lmev_strut_edge(hedge_from_vertex1, 10., 1.0, 0., &hedge_from_vertex2);
    csmeuler_lmev_strut_edge(hedge_from_vertex2, 0., 10., 0., &hedge_from_vertex3);
    csmeuler_lmef(initial_hedge, hedge_from_vertex3, NULL, &he_pos, &he_neg);
    
    // Hueco de un sólo vértice...
    {
        struct csmhedge_t *he_from_vertex, *he_to_vertex;
        struct csmhedge_t *he1_hole_pos_next, *he2_hole_pos_next;
        struct csmhedge_t *hedge_lado_neg, *hedge_lado_pos;
        
        csmeuler_lmev(he_pos, he_pos, 1., 1., 0., NULL, NULL, &he_from_vertex, &he_to_vertex);
        csmeuler_lkemr(&he_to_vertex, &he_from_vertex, &he1_hole_pos_next, &he2_hole_pos_next);
        
        csmeuler_lmekr(he1_hole_pos_next, he2_hole_pos_next, &hedge_lado_neg, &hedge_lado_pos);
        csmeuler_lkev(&hedge_lado_neg, &hedge_lado_pos, &hedge_lado_neg, NULL, NULL, &hedge_lado_pos);
        assert(hedge_lado_neg != NULL);
        assert(hedge_lado_pos != NULL);
        assert(hedge_lado_pos == he_pos);
        assert(hedge_lado_neg == hedge_from_vertex1);
    }
    
    csmeuler_lkef(&he_pos, &he_neg);
    assert(he_pos != NULL);
    assert(he_neg != NULL);
    assert(he_pos == hedge_from_vertex3);
    
    he1 = he_pos;
    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex2);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex1);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == he2);
    assert(he1 == initial_hedge);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_hexaedro(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hei, *he1, *he2, *he3, *he4;
    struct csmhedge_t *he1_top, *he2_top, *he3_top, *he4_top;
    struct csmhedge_t *he1_top_next, *he1_top_next_next, *he1_top_next_next_next;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hei);
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    // Cara inferior...
    {
        csmeuler_lmev_strut_edge(hei, 10.,  0., 0., &he1);
        csmeuler_lmev_strut_edge(he1, 10., 10., 0., &he2);
        csmeuler_lmev_strut_edge(he2,  0., 10., 0., &he3);
        csmeuler_lmef(hei, he3, NULL, &he4, NULL);
        assert(csmopbas_mate(he1) == hei);
    }

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    // Aristas verticales...
    {
        csmeuler_lmev_strut_edge(he1, 10.,  0., 10., &he1_top);
        csmeuler_lmev_strut_edge(he2, 10., 10., 10., &he2_top);
        csmeuler_lmev_strut_edge(he3,  0., 10., 10., &he3_top);
        csmeuler_lmev_strut_edge(he4,  0.,  0., 10., &he4_top);
        
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he2_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he3_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he4_top));
    }

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    csmeuler_lmef(he4_top, he3_top, NULL, NULL, NULL);
    csmeuler_lmef(he3_top, he2_top, NULL, NULL, NULL);
    csmeuler_lmef(he2_top, he1_top, NULL, NULL, NULL);

    //csmeuler_lmef(he1_top, he4_top, &id_nuevo_elemento, NULL, NULL, NULL);
    
    he1_top_next = csmhedge_next(he1_top);
    assert(he1_top_next == he1);
    
    he1_top_next_next = csmhedge_next(he1_top_next);
    he1_top_next_next_next = csmhedge_next(he1_top_next_next);
    assert(csmhedge_vertex(he1_top_next_next_next) == csmhedge_vertex(he4_top));

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    csmeuler_lmef(he1_top, he1_top_next_next_next, NULL, NULL, NULL);
    
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_hexaedro_y_copia(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hei, *he1, *he2, *he3, *he4;
    struct csmhedge_t *he1_top, *he2_top, *he3_top, *he4_top;
    struct csmhedge_t *he1_top_next, *he1_top_next_next, *he1_top_next_next_next;
    struct csmsolid_t *copia_solido;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hei);
    
    // Cara inferior...
    {
        csmeuler_lmev_strut_edge(hei, 10.,  0., 0., &he1);
        csmeuler_lmev_strut_edge(he1, 10., 10., 0., &he2);
        csmeuler_lmev_strut_edge(he2,  0., 10., 0., &he3);
        csmeuler_lmef(hei, he3, NULL, &he4, NULL);
        assert(csmopbas_mate(he1) == hei);
    }

    // Aristas verticales...
    {
        csmeuler_lmev_strut_edge(he1, 10.,  0., 10., &he1_top);
        csmeuler_lmev_strut_edge(he2, 10., 10., 10., &he2_top);
        csmeuler_lmev_strut_edge(he3,  0., 10., 10., &he3_top);
        csmeuler_lmev_strut_edge(he4,  0.,  0., 10., &he4_top);
        
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he2_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he3_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he4_top));
    }

    csmeuler_lmef(he4_top, he3_top, NULL, NULL, NULL);
    csmeuler_lmef(he3_top, he2_top, NULL, NULL, NULL);
    csmeuler_lmef(he2_top, he1_top, NULL, NULL, NULL);

    //csmeuler_lmef(he1_top, he4_top, &id_nuevo_elemento, NULL, NULL, NULL);
    
    he1_top_next = csmhedge_next(he1_top);
    assert(he1_top_next == he1);
    
    he1_top_next_next = csmhedge_next(he1_top_next);
    he1_top_next_next_next = csmhedge_next(he1_top_next_next);
    assert(csmhedge_vertex(he1_top_next_next_next) == csmhedge_vertex(he4_top));

    csmeuler_lmef(he1_top, he1_top_next_next_next, NULL, NULL, NULL);
    
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    copia_solido = csmsolid_duplicate(solido);
    csmsolid_debug_print_debug(copia_solido, CSMTRUE);
    
    csmsolid_free(&solido);
    csmsolid_free(&copia_solido);
}

// ------------------------------------------------------------------------------------------

struct i_item_tabla_t
{
    unsigned long id;
    double a;
};

static void i_test_tabla_hash(void)
{
    struct csmhashtb(i_item_tabla_t) *tabla_hash;
    struct i_item_tabla_t item1, item2, item3;
    struct i_item_tabla_t *item;
    
    item1.id = 3;
    item1.a = 9.;

    item2.id = 5;
    item2.a = 19.;

    item3.id = 8;
    item3.a = 25.;
    
    tabla_hash = csmhashtb_create_empty(i_item_tabla_t);
    
    csmhashtb_add_item(tabla_hash, 3, &item1, i_item_tabla_t);
    csmhashtb_add_item(tabla_hash, 5, &item2, i_item_tabla_t);
    csmhashtb_add_item(tabla_hash, 1000, &item3, i_item_tabla_t);
    
    item = csmhashtb_ptr_for_id(tabla_hash, 5, i_item_tabla_t);
    assert_no_null(item);
    assert(item->id == 5);
    
    csmhashtb_remove_item(tabla_hash, 5, i_item_tabla_t);
    assert(csmhashtb_contains_id(tabla_hash, i_item_tabla_t, 5, NULL) == CSMFALSE);

    csmhashtb_add_item(tabla_hash, 5, &item2, i_item_tabla_t);
    
    {
        struct csmhashtb_iterator(i_item_tabla_t) *it;
        
        it = csmhashtb_create_iterator(tabla_hash, i_item_tabla_t);
        
        while (csmhashtb_has_next(it, i_item_tabla_t) == CSMTRUE)
        {
            unsigned long id;
            struct i_item_tabla_t *item_tabla;
            
            csmhashtb_next_pair(it, &id, &item_tabla, i_item_tabla_t);
            fprintf(stdout, "%lu %lu %lf\n", id, item_tabla->id, item_tabla->a);
        }
        
        csmhashtb_free_iterator(&it, i_item_tabla_t);
    }
    
    csmhashtb_free(&tabla_hash, i_item_tabla_t, NULL);
}

// ------------------------------------------------------------------------------------------

static void i_test_solid_from_shape2D(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid;
    
    shape2d = csmbasicshape2d_rectangular_shape(0.3, 0.3);
    
    solid = csmsweep_create_solid_from_shape(
                                shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                                shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_solid_from_shape2D_with_hole(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid;
    
    shape2d = csmbasicshape2d_rectangular_hollow_shape(0.6, 0.6, 0.3, 0.3);
    
    solid = csmsweep_create_solid_from_shape(
                                shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                                shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos_por_loopglue(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2;
    CSMBOOL could_merge_solids;
    
    shape2d = csmbasicshape2d_rectangular_shape(0.3, 0.3);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);

    solid2 = csmsweep_create_solid_from_shape(shape2d, 0.3, 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0.3, 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid2, CSMTRUE);

    csmglue_join_solid2_to_solid1_given_equal_faces(solid1, 46, &solid2, 36, &could_merge_solids);
    assert(could_merge_solids == CSMTRUE);

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

#include "csmstring.inl"

static void i_set_output_debug_file(const char *file_name)
{
    char *file_path;
    
    file_path = copiafor_codigo1("/Users/manueru/%s", file_name);
    csmdebug_set_ouput_file(file_path);
    
    csmstring_free(&file_path);
}

// ------------------------------------------------------------------------------------------

static void i_show_split_results(
                        double A, double B, double C, double D,
                        CSMBOOL show_plane,
                        struct csmsolid_t *solid_above, struct csmsolid_t *solid_below)
{
    double desp;
    
    if (solid_above != NULL)
    {
        csmsolid_set_name(solid_above, "Above");
        csmsolid_debug_print_debug(solid_above, CSMTRUE);
    }
    
    if (solid_below != NULL)
    {
        csmsolid_set_name(solid_below, "Below");
        csmsolid_debug_print_debug(solid_below, CSMTRUE);
    }
    
    csmdebug_close_output_file();
    
    desp = 0.1;
    
    if (solid_above != NULL)
        csmsolid_move(solid_above, desp * A, desp * D, desp * C);
    
    if (solid_below != NULL)
        csmsolid_move(solid_below, -desp * A, -desp * D, -desp * C);

    if (solid_above != NULL || solid_below != NULL )
    {
        if (show_plane == CSMTRUE)
            csmdebug_set_plane(A, B, C, D);
        else
            csmdebug_clear_plane();
        
        csmdebug_set_viewer_results(solid_above, solid_below);
        csmdebug_show_viewer();
    }
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_por_plano_medio(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split1.txt");
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_set_name(solid1, "Solid to split");
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_medio(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_hueco_plano_medio.txt");

    shape2d = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_medio2(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_hueco_plano_medio2.txt");
    
    shape2d = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(
                        shape2d,
                        0., 0., 0.,
                        1., 0., 0., 0., 0., 1.,
                        shape2d,
                        0., 1., 0.,
                        1., 0., 0., 0., 0., 1.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 0., 1., 0., 0., 0., 1., 0.5, &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);

    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_superior(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_rectangular_hueco_por_plano_superior.txt");

    shape2d = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.5, 0.5);
    shape2d = csmbasicshape2d_circular_hollow_shape(1., 0.367, 32);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., 1., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    csmsolid_free(&solid_above);

    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., -1., &A, &B, &C, &D);

    splitted = csmsplit_does_plane_split_solid(solid_below, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_superior2(void)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    shape2d = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 1. - 1.5e-3, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid_above, CSMTRUE);
    csmsolid_debug_print_debug(solid_below, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos1(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_solidos1.txt");
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos2(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_solidos2.txt");
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos1(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.5, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos2(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    circular_shape2d = csmbasicshape2d_circular_shape(0.25, 4);
    //circular_shape2d = csmbasicshape2d_rectangular_shape_con_pto_inflexion(0.25, 0.25);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.15, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.15, -0.01, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmshape2d_t *, i_rectangular_shape2d_with_inflection_point, (double ancho_x, double ancho_y))
{
    struct csmshape2d_t *shape2d;
    csmArrPoint2D *points;
    
    assert(ancho_x > 0.);
    assert(ancho_y > 0.);
    
    shape2d = csmshape2d_new();
    
    points = csmArrPoint2D_new(6);
    csmArrPoint2D_set(points, 0, -.5 * ancho_x, -.5 * ancho_y);
    csmArrPoint2D_set(points, 1,  .0, -.5 * ancho_y);
    csmArrPoint2D_set(points, 2,  .5 * ancho_x, -.5 * ancho_y);
    
    csmArrPoint2D_set(points, 3,  .5 * ancho_x,  .5 * ancho_y);
    csmArrPoint2D_set(points, 4,  .0,  .5 * ancho_y);
    csmArrPoint2D_set(points, 5, -.5 * ancho_x,  .5 * ancho_y);
    
    csmshape2d_append_new_polygon_with_points(shape2d, &points);
    
    return shape2d;
}
// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos5(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    //circular_shape2d = csmbasicshape2d_circular_shape(0.25, 4);
    circular_shape2d = i_rectangular_shape2d_with_inflection_point(0.5, 0.5);
    shape2d = csmbasicshape2d_rectangular_shape(2., 2.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 1.0, -0.15, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 1.0, -0.15, 0.01, 1., 0., 0., 0., 1., 0., 0);
    //solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., circular_shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 2., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 2., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos7(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *c_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    c_shape2d = csmbasicshape2d_C_shape(1., 0.9);
    //c_shape2d = csmbasicshape2d_L_shape(1., 1.);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(c_shape2d, 1. / 3., 0., 0.75, 1., 0., 0., 0., 1., 0., c_shape2d, 1. / 3., 0., 0.01, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmshape2d_free(&c_shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos6(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    struct csmshape2d_t *shape3d;
    struct csmsolid_t *solid3, *solid_res2;
    
    circular_shape2d = csmbasicshape2d_circular_shape(0.25, 4);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.15, 1., 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.15, 0.5, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 0.5, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    shape3d = csmbasicshape2d_rectangular_shape(0.75, 0.75);
    solid3 = csmsweep_create_solid_from_shape_debug(shape3d, 1., 0., 1, 1., 0., 0., 0., 1., 0., shape3d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 2000);
    
    solid_res2 = csmsetop_union_A_and_B(solid3, solid_res);
    csmsolid_debug_print_debug(solid_res2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
    csmsolid_free(&solid3);
    csmsolid_free(&solid_res2);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos3(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("inters_solidos3.txt");

    circular_shape2d = csmbasicshape2d_circular_shape(0.25, 4);
    //circular_shape2d = csmbasicshape2d_rectangular_shape_con_pto_inflexion(0.25, 0.25);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    //solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.16, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.16, 0.0, 1., 0., 1., 0., 1., 0., 0);
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.16, 0.75, 1., 0., 1., 0., 1., 0., circular_shape2d, 0.5, -0.16, 0.01, 1., 0., 1., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    csmdebug_print_debug_info("******* Solid 1 intersect solid 2 [begin]");
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    csmdebug_print_debug_info("******* Solid 1 intersect solid 2 [end]");

    csmdebug_print_debug_info("******* Solid 2 intersect solid 1 [begin]");
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    csmdebug_print_debug_info("******* Solid 2 intersect solid 2 [end]");
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos4(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.25, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0.25, -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_resta_solidos1(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.25, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0.25, -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_resta_solidos2(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *rshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    rshape2d = csmbasicshape2d_rectangular_shape(0.25, 0.25);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(rshape2d, 0.5, -0.25, 0.75, 1., 0., 0., 0., 1., 0., rshape2d, 0.5, -0.25, 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&rshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos1(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = csmbasicshape2d_circular_shape(0.5, 5);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    solid_res = csmsetop_union_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos2(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = csmbasicshape2d_circular_shape(0.5, 16);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_union_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos3(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = csmbasicshape2d_circular_shape(0.5, 16);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro1(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_cilindro1.txt");

    cshape2d = csmbasicshape2d_circular_shape(0.55, 4);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    shape2d2 = csmbasicshape2d_rectangular_shape(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        shape2d2, 1.,  2., 1.5, 0., 0., 1., 1., 0., 0.,
                        shape2d2, 1., -2., .5, 0., 0., 1., 1., 0., 0.,
                        1000);
    
    {
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
    }

    {
        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");
    }
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmshape2d_free(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro2(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro1.txt");

    cshape2d = csmbasicshape2d_circular_shape(0.55, 4);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    shape2d2 = csmbasicshape2d_rectangular_shape(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2.,   1., -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 1., -1., 0., 0., 0., 0., 1.,
                        1000);
     /*
      solid2 = csmsweep_create_solid_from_shape_debug(
                        shape2d2, 1.,  2., 1.5, 0., 0., 1., 1., 0., 0.,
                        shape2d2, 1., -2., .5, 0., 0., 1., 1., 0., 0.,
                        1000);
      */
    
    {
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
    }

    {
        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");
    }
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmshape2d_free(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro5(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro5.txt");

    cshape2d = csmbasicshape2d_circular_shape(0.25, 4);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    shape2d2 = csmbasicshape2d_rectangular_shape(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, .75,  0.25,   1., -1., 0., 0., 0., 0., 1.,
                        cshape2d, .75, -2.,     1., -1., 0., 0., 0., 0., 1.,
                        1000);
    {
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
    }

    {
        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");
    }
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmshape2d_free(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro6(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro6.txt");

    cshape2d = csmbasicshape2d_circular_shape(0.50, 4);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    shape2d2 = csmbasicshape2d_rectangular_shape(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 1., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 1., -2., 1., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    {
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
    }

    {
        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");
    }
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmshape2d_free(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro7(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        csmArrPoint2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = csmArrPoint2D_new(0);
        csmArrPoint2D_append(points, -0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0., 0.5 * ay);
        
        cshape2d = csmshape2d_new();
        csmshape2d_append_new_polygon_with_points(cshape2d, &points);
    }
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 0.75, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 0.75, -1., 0., 0., 0., 0., 1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro3(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    unsigned long no_sides_circle;
    
    i_set_output_debug_file("inters_cilindro2.txt");

    no_sides_circle = 32;
    
    cshape2d = csmbasicshape2d_circular_shape(0.40, no_sides_circle);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  1., 1., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 1., -1., 1., 0., 0., 1., 1., 0., 0.,
                        0);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    {
        struct csmsolid_t *solid3;
        struct csmsolid_t *solid_res2;
        
        cshape2d = csmbasicshape2d_circular_shape(0.40, no_sides_circle);
        
        solid3 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,  2., 0., .25, 0., 1., 0., 0., 0., 1.,
                        cshape2d, -1., 0., .25, 0., 1., 0., 0., 0., 1.,
                        0);
        
        solid_res2 = csmsetop_difference_A_minus_B(solid_res, solid3);
        
        //csmsolid_move(solid3, 0., -.4, 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);

        //csmsolid_move(solid3, 0., 1., 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);
        
        {
            CSMBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 1.;
            B = C = 0.;
            D = -1.;
            
            does_split = csmsplit_does_plane_split_solid(solid_res2, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CSMTRUE);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
        csmsolid_free(&solid_res2);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro4(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid3, *solid_res;
    unsigned long no_points_circle;
    
    i_set_output_debug_file("inters_cilindro2.txt");
    csmdebug_set_enabled_by_code(CSMFALSE);

    no_points_circle = 32;
    
    //cshape2d = csmbasicshape2d_rectangular_shape(0.80, 0.80);
    cshape2d = csmbasicshape2d_circular_shape(0.40, no_points_circle);
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2.,     0.5, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2.,   0.5, -1., 0., 0., 0., 0., 1.,
                        //cshape2d, 1., -0.25,   0.5, -1., 0., 0., 0., 0., 1.,
                        5000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    
    {
        struct csmsolid_t *solid3, *solid4;
        
        solid3 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,  1., 0., 1.05, 1., 0., 0., 0., 1., 0.,
                        cshape2d,  1., 0., 0.0,  1., 0., 0., 0., 1., 0.,
                        10000);
        
        solid_res = csmsetop_difference_A_minus_B(solid_res, solid3);
        
        
        csmdebug_set_enabled_by_code(CSMTRUE);
        
        //cshape2d = csmbasicshape2d_rectangular_shape(0.75, 0.75);

        /*
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,   2., 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,  -2., 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
         
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,     0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);

        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
         
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,     0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);

         */
        
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
        
        solid_res = csmsetop_difference_A_minus_B(solid_res, solid4);
        
        {
            struct csmsolid_t *solid5;
            
            cshape2d = csmbasicshape2d_circular_shape(0.35, no_points_circle); // Revisar resta con num_puntos_circulo = 8
            
            solid5 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
            
            csmsolid_move(solid1, -0.5, 0., 0.);
            solid_res = csmsetop_difference_A_minus_B(solid1, solid_res);
            i_assign_flat_material_to_solid(0.5, 0.5, 0.5, solid_res);
            
            i_assign_flat_material_to_solid(1., 0., 0., solid5);
            solid_res = csmsetop_difference_A_minus_B(solid_res, solid5);
            //solid_res = csmsetop_intersection_A_and_B(solid_res, solid5);
            //solid_res = csmsetop_intersection_A_and_B(solid_res5, solid_res);
        }
        
        {
            CSMBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 0.;
            B = 0;
            C = 1.;
            D = -0.55; // Falla split a -0.75
            
            does_split = csmsplit_does_plane_split_solid(solid_res, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CSMTRUE);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro8(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        csmArrPoint2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = csmArrPoint2D_new(0);
        csmArrPoint2D_append(points, -0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0.01 * ax, 0.5 * ay);
        csmArrPoint2D_append(points,  -0.01 * ax, 0.5 * ay);
        
        cshape2d = csmshape2d_new();
        csmshape2d_append_new_polygon_with_points(cshape2d, &points);
    }
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 0.75, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 0.75, -1., 0., 0., 0., 0., 1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro9(struct csmviewer_t *viewer)
{
    struct csmshape2d_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        csmArrPoint2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = csmArrPoint2D_new(0);
        csmArrPoint2D_append(points, -0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0.5 * ax, -0.5 * ay);
        csmArrPoint2D_append(points,  0., 0.5 * ay);
        
        cshape2d = csmshape2d_new();
        csmshape2d_append_new_polygon_with_points(cshape2d, &points);
    }
    
    shape2d = csmbasicshape2d_rectangular_shape(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 1.25, 1., 0., 0., 0., 0., -1.,
                        cshape2d, 1., -2., 1.25, 1., 0., 0., 0., 0., -1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    csmshape2d_free(&cshape2d);
    csmshape2d_free(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_ring_part, (
                        double Zo,
                        double height,
                        double outer_radius, double inner_radius, unsigned long no_points_circle))
{
    struct csmsolid_t *ring_part;
    struct csmshape2d_t *ring_shape;
    
    ring_shape = csmbasicshape2d_circular_hollow_shape(outer_radius, inner_radius, no_points_circle);
    
    ring_part = csmsweep_create_solid_from_shape_debug(
                        ring_shape,
                        0., 0., Zo + height, 1., 0., 0., 0., 1., 0.,
                        ring_shape,
                        0., 0., Zo, 1., 0., 0., 0., 1., 0.,
                        1000);
    
    csmshape2d_free(&ring_shape);
    
    return ring_part;
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part1(void)
{
    struct csmsolid_t *basic_part;
    struct csmsolid_t *main_part;
    double ax, ay, main_part_length;
    struct csmshape2d_t *main_part_shape;
    
    i_set_output_debug_file("mechanical1.txt");
    
    ax = 0.1;
    ay = 0.1;
    main_part_length = 1.;
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    
    //main_part_shape = csmbasicshape2d_rectangular_shape(ax, ay);
    main_part_shape = csmbasicshape2d_circular_shape(ax, 32);
    
    basic_part = csmsweep_create_solid_from_shape_debug(
                        main_part_shape,
                        0., 0., main_part_length, 1., 0., 0., 0., 1., 0.,
                        main_part_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    main_part = csmsolid_duplicate(basic_part);
    
    {
        unsigned long i, no_divisions;
        double ring_part_height;
        struct csmsolid_t *ring_part, *ring_part2;
        double incr_z;
        unsigned long no_operations;
        
        no_divisions = 20;
        ring_part_height = main_part_length / no_divisions;
        ring_part = i_make_ring_part(0., ring_part_height, /*1.2*/1.5 * ax, 0.4 * ax, 16);
        ring_part2 = i_make_ring_part(0., ring_part_height, 1.2 * ax, 0.4 * ax, 4);
        
        incr_z = ring_part_height;
        no_operations = 0;
        
        for (i = 0; i < no_divisions - 1; i++)
        {
            if (i % 2 == 1)
            {
                struct csmsolid_t *main_part_loc;
            
                if (no_operations % 2 == 0)
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part);
                else
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part2);
                
                csmsolid_free(&main_part);
                main_part = main_part_loc;
                
                no_operations++;
                //break;
            }
            
            csmsolid_move(ring_part, 0., 0., incr_z);
            csmsolid_move(ring_part2, 0., 0., incr_z);
        }
     
        csmsolid_free(&ring_part);
        csmsolid_free(&ring_part2);
     }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    //csmdebug_set_enabled_by_code(CSMFALSE);
   
    
    {
        struct csmshape2d_t *diff_shape;
        struct csmsolid_t *basic_part2, *main_part_loc;
        
        //diff_shape = csmbasicshape2d_rectangular_shape(0.6 * ax, 0.6 * ax);
        diff_shape = csmbasicshape2d_circular_shape(0.8 * ax, 16);
        
        basic_part2 = csmsweep_create_solid_from_shape_debug(
                        diff_shape,
                        0., 0., /*0.55*/ 0.5 * main_part_length, 1., 0., 0., 0., 1., 0.,
                        diff_shape,
                        0., 0., 0.05, 1., 0., 0., 0., 1., 0.,
                        1000);

        csmsolid_move(basic_part2, 0.25 * ax, 0., 0./*0.05 * main_part_length*/);
        
        main_part_loc = csmsetop_difference_A_minus_B(main_part, basic_part2);
        csmsolid_free(&main_part);
        main_part = main_part_loc;
        
        csmshape2d_free(&diff_shape);
        csmsolid_free(&basic_part2);
    }

    csmdebug_set_enabled_by_code(CSMTRUE);

    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        //csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0., 0., 1., 0., 0., 0., 1., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
    }
    
    csmshape2d_free(&main_part_shape);
    csmsolid_free(&main_part);
    csmsolid_free(&basic_part);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part1_redux(void)
{
    struct csmsolid_t *basic_part;
    struct csmsolid_t *main_part;
    double ax, ay, main_part_length;
    struct csmshape2d_t *main_part_shape;
    
    i_set_output_debug_file("mechanical1_redux.txt");
    
    ax = 0.1;
    ay = 0.1;
    main_part_length = 1.;
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    
    main_part_shape = csmbasicshape2d_rectangular_shape(ax, ay);
    //main_part_shape = csmbasicshape2d_circular_shape(ax, 32);
    
    basic_part = csmsweep_create_solid_from_shape_debug(
                        main_part_shape,
                        0., 0., main_part_length, 1., 0., 0., 0., 1., 0.,
                        main_part_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    main_part = csmsolid_duplicate(basic_part);
    
    {
        unsigned long i, no_divisions;
        double ring_part_height;
        struct csmsolid_t *ring_part, *ring_part2;
        double incr_z;
        unsigned long no_operations;
        
        no_divisions = 20;
        ring_part_height = main_part_length / no_divisions;
        ring_part = i_make_ring_part(0., ring_part_height, /*1.2*/1.5 * ax, 0.4 * ax, 16);
        ring_part2 = i_make_ring_part(0., ring_part_height, 1.2 * ax, 0.4 * ax, 4);
        
        incr_z = ring_part_height;
        no_operations = 0;
        
        for (i = 0; i < no_divisions - 1; i++)
        {
            if (i % 2 == 1)
            {
                struct csmsolid_t *main_part_loc;
            
                if (no_operations % 2 == 0)
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part);
                else
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part2);
                
                csmsolid_free(&main_part);
                main_part = main_part_loc;
                
                no_operations++;
                //break;
            }
            
            csmsolid_move(ring_part, 0., 0., incr_z);
            csmsolid_move(ring_part2, 0., 0., incr_z);
        }
     
        csmsolid_free(&ring_part);
        csmsolid_free(&ring_part2);
     }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    //csmdebug_set_enabled_by_code(CSMFALSE);
   
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0.5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        main_part = solid_above;
        
        csmmath_implicit_plane_equation(0., 0., 0.56, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);
        
        main_part = solid_below;
        
        csmdebug_clear_plane();
    }
    
    {
        struct csmshape2d_t *diff_shape;
        struct csmsolid_t *basic_part2, *main_part_loc;
        
        //diff_shape = csmbasicshape2d_rectangular_shape(0.6 * ax, 0.6 * ax);
        diff_shape = csmbasicshape2d_circular_shape(0.6 * ax, 6);
        
        basic_part2 = csmsweep_create_solid_from_shape_debug(
                        diff_shape,
                        0., 0., 0.55 /*0.5 * main_part_length*/, 1., 0., 0., 0., 1., 0.,
                        diff_shape,
                        0., 0., 0.5, 1., 0., 0., 0., 1., 0.,
                        1000);

        csmsolid_move(basic_part2, 0.25 * ax, 0., 0./*0.05 * main_part_length*/);
        
        main_part_loc = csmsetop_difference_A_minus_B(main_part, basic_part2);
        csmsolid_free(&main_part);
        main_part = main_part_loc;
        
        csmshape2d_free(&diff_shape);
        csmsolid_free(&basic_part2);
    }

    csmdebug_set_enabled_by_code(CSMTRUE);

    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0., 0., 1., 0., 0., 0., 1., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
    }
    
    csmshape2d_free(&main_part_shape);
    csmsolid_free(&main_part);
    csmsolid_free(&basic_part);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part2(void)
{
    double ax, ay, head_length, bolt_length, thread_length;
    struct csmshape2d_t *head_shape, *body_shape, *thread_shape;
    struct csmsolid_t *head_solid, *body_solid, *thread_solid, *bolt_solid;
    
    i_set_output_debug_file("mechanical2.txt");
    
    ax = 0.025;
    ay = 0.1;
    head_length = 0.01;
    bolt_length = 0.08;
    thread_length = 0.0025;
    
    //main_part_shape = csmbasicshape2d_rectangular_shape(ax, ay);
    head_shape = csmbasicshape2d_circular_shape(ax, 6);
    body_shape = csmbasicshape2d_circular_shape(0.5 * ax, 32);
    thread_shape = csmbasicshape2d_circular_shape(0.6 * ax, 32);
    
    head_solid = csmsweep_create_solid_from_shape_debug(
                        head_shape,
                        0., 0., head_length, 1., 0., 0., 0., 1., 0.,
                        head_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);

    body_solid = csmsweep_create_solid_from_shape_debug(
                        body_shape,
                        0., 0., bolt_length, 1., 0., 0., 0., 1., 0.,
                        body_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    thread_solid = csmsweep_create_solid_from_shape_debug(
                        thread_shape,
                        0., 0., thread_length, 1., 0., 0., 0., 1., 0.,
                        thread_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    csmsolid_move(head_solid, 0., 0., bolt_length);
    
    bolt_solid = csmsetop_union_A_and_B(head_solid, body_solid);
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    {
        unsigned long i, num_divisions;
        
        num_divisions = bolt_length / thread_length;
        
        for (i = 0; i < num_divisions; i++)
        {
            if (i % 2 == 0)
                bolt_solid = csmsetop_union_A_and_B(bolt_solid, thread_solid);
            
            csmsolid_move(thread_solid, 0., 0., thread_length);
        }
    }
    csmdebug_set_enabled_by_code(CSMTRUE);
    
    csmdebug_set_viewer_results(bolt_solid, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmsolid_t *bolt_solid2, *bolt_solid3;
        struct csmshape2d_t *block_shape;
        struct csmsolid_t *block_solid;
        
        bolt_solid2 = csmsolid_duplicate(bolt_solid);
        csmsolid_rotate(bolt_solid2, 0.6 * CSMMATH_PI, 0., 0., 0.5 * (bolt_length + head_length), 0., 1., 0.);
        csmsolid_move(bolt_solid2, 0.1 - 0.2 * (bolt_length + head_length) + head_length, 0., 0.);
        
        bolt_solid3 = csmsolid_duplicate(bolt_solid);
        csmsolid_rotate(bolt_solid3, -0.5 * CSMMATH_PI, 0., 0., 0.5 * (bolt_length + head_length), 0., 1., 0.);
        csmsolid_move(bolt_solid3, -0.1 + 0.5 * (bolt_length + head_length) - head_length, 0., 0.);

        csmdebug_set_viewer_results(bolt_solid2, bolt_solid3);
        csmdebug_show_viewer();
        
        csmdebug_set_enabled_by_code(CSMFALSE);
        {
            block_shape = csmbasicshape2d_rectangular_shape(0.2, 0.2);
        
            block_solid = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0., bolt_length + head_length, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);

            csmdebug_set_viewer_results(bolt_solid2, block_solid);
            csmdebug_show_viewer();
            
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid);
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid2);
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid3);
        }
        csmdebug_set_enabled_by_code(CSMTRUE);
        
        csmdebug_set_viewer_results(block_solid, NULL);
        csmdebug_show_viewer();
        
        {
            double A, B, C, D;
            struct csmsolid_t *solid_above, *solid_below;
            CSMBOOL splitted;

            csmdebug_set_enabled_by_code(CSMFALSE);
            
            csmmath_implicit_plane_equation(0., 0., 0., 0., 0., 1., 1., 0., 0., &A, &B, &C, &D);
        
            splitted = csmsplit_does_plane_split_solid(block_solid, A, B, C, D, &solid_above, &solid_below);
            assert(splitted == CSMTRUE);

            csmdebug_set_enabled_by_code(CSMTRUE);
            
            i_show_split_results(A, B, C, D, CSMFALSE, solid_above, NULL);
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_test_toroide(void)
{
    struct csmsolid_t *toroide;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    toroide = csmquadrics_create_torus(
                        3.,  16,
                        .75, 8,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, toroide);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_cono(void)
{
    struct csmsolid_t *cono;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    cono = csmquadrics_create_cone(
                        5., 1., 32,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 0., 1.,
                        1);

    csmdebug_set_viewer_results(cono, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, cono);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere(void)
{
    struct csmsolid_t *cono;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    cono = csmquadrics_create_sphere(
                        2.,
                        8,
                        16,
                        1.2, 1., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    csmdebug_set_viewer_results(cono, NULL);
    //csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, cono);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere2(void)
{
    struct csmsolid_t *sphere, *torus, *torus2, *cone;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    i_assign_flat_material_to_solid(1., 0., 0., sphere);

    torus = csmquadrics_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);
    i_assign_flat_material_to_solid(0., 0., 1., torus);

    torus2 = csmquadrics_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    i_assign_flat_material_to_solid(0., 1., 1., torus2);
    
    cone = csmquadrics_create_cone(
                        5., 2., 32,
                        0., 0., 0.,
                        -1., 0., 0.,
                        0., 0., 1.,
                        1);
    i_assign_flat_material_to_solid(1., 1., 1., cone);
    
    csmdebug_set_viewer_results(sphere, torus);
    //csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(sphere, torus);
    res = csmsetop_difference_A_minus_B(res, torus2);
    res = csmsetop_difference_A_minus_B(res, cone);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere3(void)
{
    struct csmsolid_t *sphere, *torus, *torus2;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    torus = csmquadrics_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);

    torus2 = csmquadrics_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    csmdebug_set_viewer_results(sphere, torus);
    //csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_union_A_and_B(sphere, torus);
    res = csmsetop_difference_A_minus_B(res, torus2);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere4(void)
{
    struct csmsolid_t *sphere1, *sphere2, *sphere3, *sphere4;
    struct csmsolid_t *res;
    struct csmsolid_t *block;
    struct csmsolid_t *torus;
    struct csmsolid_t *cylinder;
    
    //toroide = csmquadrics_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere1 = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere2 = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        2.8, 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere3 = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        2.8, 2.8, 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere4 = csmquadrics_create_sphere(
                        2.8,
                        8,
                        16,
                        0.0, 2.8, 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    i_assign_flat_material_to_solid(1., 0., 0., sphere1);
    i_assign_flat_material_to_solid(1., 0., 0., sphere2);
    i_assign_flat_material_to_solid(1., 0., 0., sphere3);
    i_assign_flat_material_to_solid(1., 0., 0., sphere4);
    
    res = csmsetop_union_A_and_B(sphere1, sphere2);
    res = csmsetop_union_A_and_B(res, sphere3);
    res = csmsetop_union_A_and_B(res, sphere4);

    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *block_shape;
        
        block_shape = csmbasicshape2d_rectangular_shape(2.9, 2.9);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        1.4, 1.4,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        1.4, 1.4, -2.8, 1., 0., 0., 0., 1., 0.,
                        10000);
        
        i_assign_flat_material_to_solid(0., 1., 0., block);
    }
    
    res = csmsetop_difference_A_minus_B(block, res);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    i_assign_flat_material_to_solid(1., 0., 0., res);
    
    res = csmsetop_difference_A_minus_B(block, res);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    torus = csmquadrics_create_torus(
                        1.5,  16,
                        1., 16,
                        1.4, 1.4, -.6,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    i_assign_flat_material_to_solid(1., 1., 1., torus);
    
    res = csmsetop_difference_A_minus_B(res, torus);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *cylinder_shape;
        
        cylinder_shape = csmbasicshape2d_circular_shape(0.3, 32);
        
        cylinder = csmsweep_create_solid_from_shape_debug(
                        cylinder_shape,
                        1.5, 1.4, 0.0, 0., 1., 0., 0., 0., 1.,
                        cylinder_shape,
                        -1.5, 1.4, 0.0, 0., 1., 0., 0., 0., 1.,
                        10000);
        
        i_assign_flat_material_to_solid(1., 0., 1., cylinder);
    }
    
    res = csmsetop_difference_A_minus_B(res, cylinder);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmshape2d_t *cylinder_shape;
        
        cylinder_shape = csmbasicshape2d_circular_shape(0.6, 32);
        
        cylinder = csmsweep_create_solid_from_shape_debug(
                        cylinder_shape,
                        2.5, 1.4, -1.8, 0., 1., 0., 0., 0., 1.,
                        cylinder_shape,
                        -2.5, 1.4, -1.8, 0., 1., 0., 0., 0., 1.,
                        10000);
        
        i_assign_flat_material_to_solid(1., 0., 1., cylinder);
    }
    
    res = csmsetop_difference_A_minus_B(res, cylinder);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_split_solid_ang_get_division, (
	                    const struct csmsolid_t *solid,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        CSMBOOL get_top))
{
    CSMBOOL split;
    double A, B, C, D;
    struct csmsolid_t *above, *below;
    
    csmmath_implicit_plane_equation(
                        Xo, Yo, Zo,
                        Ux, Uy, Uz, Vx, Vy, Vz,
                        &A, &B, &C, &D);
    
    split = csmsplit_does_plane_split_solid(solid, A, B, C, D, &above, &below);
    assert(split == CSMTRUE);
    
    if (get_top == CSMTRUE)
    {
        csmsolid_free(&below);
        return above;
    }
    else
    {
        csmsolid_free(&above);
        return below;
    }
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_hollow_semisphere_at_origin, (double radius, double thickness, unsigned long no_points_circle))
{
    struct csmsolid_t *hollow_sphere;
    struct csmsolid_t *solid_sphere, *solid_above_sphere, *inner_sphere;
    CSMBOOL split;
    
    assert(radius > 0.);
    assert(thickness > 0.);
    assert(thickness < radius);
    
    solid_sphere = csmquadrics_create_sphere(radius, no_points_circle, no_points_circle, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    solid_above_sphere = i_split_solid_ang_get_division(solid_sphere, 0., 0., 0., 1., 0., 0., 0., 1., 0., CSMTRUE);
    csmsolid_free(&solid_sphere);
    
    inner_sphere = csmquadrics_create_sphere(radius - thickness, no_points_circle, no_points_circle, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    hollow_sphere = csmsetop_difference_A_minus_B(solid_above_sphere, inner_sphere);
    csmsolid_free(&inner_sphere);
    
    i_assign_flat_material_to_solid(.5, .5, .5, hollow_sphere);
    
    return hollow_sphere;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_hollow_cylinder, (double radius, double thickness, double height, unsigned long no_points_circle))
{
    struct csmsolid_t *hollow_cylinder;
    struct csmshape2d_t *shape;
    
    assert(radius > 0.);
    assert(thickness > 0.);
    assert(thickness < radius);
    assert(height > 0.);
    
    shape = csmbasicshape2d_circular_hollow_shape(radius, radius - thickness, no_points_circle);
    
    hollow_cylinder = csmsweep_create_solid_from_shape_debug(
	                    shape, 0., 0.,  0.5 * height, 1., 0., 0., 0., 1., 0.,
                        shape, 0., 0., -0.5 * height, 1., 0., 0., 0., 1., 0.,
                        0);
    
    i_assign_flat_material_to_solid(.5, .5, .5, hollow_cylinder);
    
    csmshape2d_free(&shape);
    
    return hollow_cylinder;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_horizontal_hole_cylinder, (double radius, double x1, double x2, double z, unsigned long no_points_circle))
{
    struct csmsolid_t *hollow_cylinder;
    struct csmshape2d_t *shape;
    
    assert(radius > 0.);
    
    shape = csmbasicshape2d_circular_shape(radius, no_points_circle);
    
    hollow_cylinder = csmsweep_create_solid_from_shape_debug(
                        shape, x2, 0., z, 0., 1., 0., 0., 0., 1.,
	                    shape, x1, 0., z, 0., 1., 0., 0., 0., 1.,
                        0);
    
    i_assign_flat_material_to_solid(.5, .5, .5, hollow_cylinder);
    
    csmshape2d_free(&shape);
    
    return hollow_cylinder;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_horizontal_hollow_hole_cylinder, (double radius, double thickness, double x1, double x2, double z, unsigned long no_points_circle))
{
    struct csmsolid_t *hollow_cylinder;
    struct csmshape2d_t *shape;
    
    assert(radius > 0.);
    
    shape = csmbasicshape2d_circular_hollow_shape(radius, radius - thickness, no_points_circle);
    
    hollow_cylinder = csmsweep_create_solid_from_shape_debug(
                        shape, x2, 0., z, 0., 1., 0., 0., 0., 1.,
	                    shape, x1, 0., z, 0., 1., 0., 0., 0., 1.,
                        0);
    
    i_assign_flat_material_to_solid(1., .3, .3, hollow_cylinder);
    
    csmshape2d_free(&shape);
    
    return hollow_cylinder;
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical5(void)
{
    struct csmsolid_t *solid;
    struct csmsolid_t *solid_aux;
    double radius, thickness, body_height;
    unsigned long no_points_circle;
    struct csmsolid_t *top_part, *hollow_cylinder;
    double hole_radius;
    struct csmsolid_t *hole_cylinder;
    double outer_hole_radius, hollow_hole_cylinder_thickness;
    struct csmsolid_t *hollow_hole_cylinder_left, *hollow_hole_cylinder_right;
    struct csmsolid_t *torus;
    
    radius = 0.025;
    thickness = 0.0025;
    no_points_circle = 32;

    body_height = 0.1;
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    
    top_part = i_make_hollow_semisphere_at_origin(radius, thickness, no_points_circle);
    csmsolid_move(top_part, 0., 0., 0.5 * body_height);
    
    hollow_cylinder = i_make_hollow_cylinder(radius, thickness, body_height, no_points_circle);

    hole_radius = 0.01;
    hole_cylinder = i_make_horizontal_hole_cylinder(hole_radius, -radius, radius, 0., no_points_circle);
    
    solid = csmsetop_union_A_and_B(top_part, hollow_cylinder);
    
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hole_cylinder);
        csmsolid_free(&solid_aux);
    }

    csmsolid_move(hole_cylinder, 0., 0., 0.045);
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hole_cylinder);
        csmsolid_free(&solid_aux);
    }
    
    hollow_hole_cylinder_thickness = 0.0015;
    outer_hole_radius = hole_radius + hollow_hole_cylinder_thickness;
    hollow_hole_cylinder_left = i_make_horizontal_hollow_hole_cylinder(outer_hole_radius, hollow_hole_cylinder_thickness, -1.5 * radius, -radius + 3. * thickness, 0., no_points_circle);
    {
        solid_aux = hollow_hole_cylinder_left;
        hollow_hole_cylinder_left = csmsetop_difference_A_minus_B(hollow_hole_cylinder_left, solid);
        csmsolid_free(&solid_aux);
        
        csmsolid_move(hollow_hole_cylinder_left, 0.5 * thickness, 0., 0.);
        i_assign_flat_material_to_solid(1., .3, .3, hollow_hole_cylinder_left);
    }
    
    hollow_hole_cylinder_right = i_make_horizontal_hollow_hole_cylinder(outer_hole_radius, hollow_hole_cylinder_thickness, radius - 3. * thickness, 1.25 * radius, 0., no_points_circle);
    {
        solid_aux = hollow_hole_cylinder_right;
        hollow_hole_cylinder_right = csmsetop_difference_A_minus_B(hollow_hole_cylinder_right, solid);
        csmsolid_free(&solid_aux);
        
        csmsolid_move(hollow_hole_cylinder_right, -0.5 * thickness, 0., 0.);
        i_assign_flat_material_to_solid(1., .3, .3, hollow_hole_cylinder_right);
    }

    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_left);
        csmsolid_free(&solid_aux);
        
        csmsolid_move(hollow_hole_cylinder_left, 0., 0., 0.045);
        
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_left);
        csmsolid_free(&solid_aux);
    }

    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_right);
        csmsolid_free(&solid_aux);
        
        csmsolid_move(hollow_hole_cylinder_right, 0., 0., 0.045);
        
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_right);
        csmsolid_free(&solid_aux);
    }
    
    csmsolid_rotate(hole_cylinder, .5 * CSMMATH_PI, 0., 0., 0.045, 0., 0., 1.);
    csmsolid_rotate(hollow_hole_cylinder_left, .5 * CSMMATH_PI, 0., 0., 0.045, 0., 0., 1.);
    csmsolid_rotate(hollow_hole_cylinder_right, .5 * CSMMATH_PI, 0., 0., 0.045, 0., 0., 1.);

    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hole_cylinder);
        csmsolid_free(&solid_aux);
    }
    
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_left);
        csmsolid_free(&solid_aux);
    }
    
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_right);
        csmsolid_free(&solid_aux);
    }
    
    csmsolid_move(hole_cylinder, 0., 0., -0.045);
    csmsolid_move(hollow_hole_cylinder_left, 0., 0., -0.045);
    csmsolid_move(hollow_hole_cylinder_right, 0., 0., -0.045);

    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hole_cylinder);
        csmsolid_free(&solid_aux);
    }
    
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_left);
        csmsolid_free(&solid_aux);
    }
    
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, hollow_hole_cylinder_right);
        csmsolid_free(&solid_aux);
    }
    
    torus = csmquadrics_create_torus(radius + 0.5 * thickness, no_points_circle, thickness, no_points_circle, 0., 0., -0.02, 1., 0., 0., 0., 1., 0., 0);
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, torus);
        csmsolid_free(&solid_aux);
    }

    csmsolid_move(torus, 0., 0., -0.01);
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, torus);
        csmsolid_free(&solid_aux);
    }

    csmsolid_move(torus, 0., 0., -0.01);
    {
        solid_aux = solid;
        solid = csmsetop_difference_A_minus_B(solid_aux, torus);
        csmsolid_free(&solid_aux);
    }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    
    csmsolid_set_draw_only_border_edges(solid, CSMFALSE);
    csmdebug_set_viewer_results(solid, NULL);
    csmdebug_show_viewer();
    
    csmsolid_free(&top_part);
    csmsolid_free(&hollow_cylinder);
    csmsolid_free(&hole_cylinder);
    csmsolid_free(&hollow_hole_cylinder_left);
    csmsolid_free(&hollow_hole_cylinder_right);
    csmsolid_free(&torus);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical6bis(void)
{
    unsigned long no_points_circle;
    struct csmshape2d_t *shape1, *shape2_redux, *shape2;
    struct csmsolid_t *s1, *s2, *s3;
    
    i_set_output_debug_file("test_mechanical7_redux.txt");
    
    no_points_circle = 4;
    
    shape1 = csmbasicshape2d_circular_hollow_shape(1., 0.6, no_points_circle);
    shape2 = csmbasicshape2d_circular_hollow_shape(1., 0.6, no_points_circle);
    shape2_redux = csmbasicshape2d_circular_hollow_shape(0.75, 0.6, no_points_circle);
    
    s1 = csmsweep_create_solid_from_shape_debug(
                        shape1, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                        shape1, 0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);

    s2 = csmsweep_create_solid_from_shape_debug(
                        shape2_redux, 0., 0., 2., 1., 0., 0., 0., 1., 0.,
                        shape2, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                        1000);
    
    csmdebug_print_debug_info("***MAIN_JOIN_OPERATION");
    s3 = csmsetop_union_A_and_B(s1, s2);
    
    csmsolid_set_draw_only_border_edges(s3, CSMFALSE);
    csmdebug_set_viewer_results(s3, NULL);
    csmdebug_show_viewer();
    
    csmshape2d_free(&shape1);
    csmshape2d_free(&shape2_redux);
    csmshape2d_free(&shape2);
    csmsolid_free(&s1);
    csmsolid_free(&s2);
    csmsolid_free(&s3);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical6(void)
{
    struct csmshape2d_t *shape1, *shape2, *shape3;
    struct csmsolid_t *s1, *s1_aux, *s2, *s3;
    
    i_set_output_debug_file("test_mechanical6.txt");
    
    //shape = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.5, 0.5);
    shape1 = csmbasicshape2d_circular_shape(1., 16);
    shape3 = csmbasicshape2d_circular_shape(.6, 16);
    shape2 = csmbasicshape2d_circular_hollow_shape(1., 0.6, 16);
    
    s1 = csmsweep_create_solid_from_shape_debug(
	                    shape1, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                        shape1, 0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    s1_aux = csmsweep_create_solid_from_shape_debug(
                        shape3, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                        shape3, 0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    s1 = csmsetop_difference_A_minus_B(s1, s1_aux);

    s2 = csmsweep_create_solid_from_shape_debug(
                        shape2, 0., 0., 2., 1., 0., 0., 0., 1., 0.,
                        shape2, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                        1000);
    
    csmdebug_print_debug_info("***MAIN_JOIN_OPERATION");
    s3 = csmsetop_union_A_and_B(s1, s2);
    
    csmsolid_set_draw_only_border_edges(s3, CSMFALSE);
    csmdebug_set_viewer_results(s3, NULL);
    csmdebug_show_viewer();
    
    csmshape2d_free(&shape1);
    csmshape2d_free(&shape2);
    csmsolid_free(&s1);
    csmsolid_free(&s2);
    csmsolid_free(&s3);
}

// ------------------------------------------------------------------------------------------

static void i_test7_edge_spheres(
                        double radius, double thick,
                        unsigned long no_points_circle,
                        struct csmsolid_t **edge_left, struct csmsolid_t **edge_rigth)
{
    struct csmsolid_t *sphere, *inner_sphere, *half_sphere_top, *half_sphere_bottom;
    struct csmsolid_t *solid_aux;
    CSMBOOL splitted;
    
    assert(radius > 0.);
    assert(thick > 0.);
    assert(thick < radius);
    
    sphere = csmquadrics_create_sphere(radius, no_points_circle, no_points_circle, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    inner_sphere = csmquadrics_create_sphere(radius - thick, no_points_circle, no_points_circle, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    
    splitted = csmsplit_does_plane_split_solid(sphere, 0., 0., 1., 0., &half_sphere_top, &half_sphere_bottom);
    assert(splitted == CSMTRUE);
    csmsolid_free(&half_sphere_top);
    csmsolid_free(&sphere);
    
    solid_aux = half_sphere_bottom;
    half_sphere_bottom = csmsetop_difference_A_minus_B(half_sphere_bottom, inner_sphere);
    csmsolid_free(&solid_aux);
    csmsolid_free(&inner_sphere);
    
    splitted = csmsplit_does_plane_split_solid(half_sphere_bottom, -1., 0., 0., 0., edge_left, edge_rigth);
    assert(splitted == CSMTRUE);
}

// ------------------------------------------------------------------------------------------

static struct csmsolid_t *i_create_horizontal_hollow_cylinder(
                    double radius, double thick,
                    double length,
                    unsigned long no_points_circle)
{
    struct csmsolid_t *cylinder;
    struct csmshape2d_t *cylinder_shape;
    
    cylinder_shape = csmbasicshape2d_circular_hollow_shape(radius, radius - thick, no_points_circle);
    
    cylinder = csmsweep_create_solid_from_shape_debug(
                    cylinder_shape,
                     0.5 * length, 0., 0., 0., 1., 0., 0., 0., 1.,
                    cylinder_shape,
                    -0.5 * length, 0., 0., 0., 1., 0., 0., 0., 1.,
                    0);
    
    csmshape2d_free(&cylinder_shape);
    
    return cylinder;
}

// ------------------------------------------------------------------------------------------

static struct csmsolid_t *i_create_planar_block(double ax, double ay, double az)
{
    struct csmsolid_t *solid;
    struct csmshape2d_t *shape;
    
    shape = csmbasicshape2d_rectangular_shape(ax, ay);
    
    solid = csmsweep_create_solid_from_shape_debug(
                        shape, 0., 0.,  0.5 * az, 1., 0., 0., 0., 1., 0.,
                        shape, 0., 0., -0.5 * az, 1., 0., 0., 0., 1., 0.,
                        0);
    
    csmshape2d_free(&shape);
    
    return solid;
}

// ------------------------------------------------------------------------------------------

static void i_test7_cylinder(
                        double radius, double thick,
                        double length,
                        unsigned long no_points_circle,
                        struct csmsolid_t **cylinder)
{
    struct csmsolid_t *cyl, *cyl_top;
    CSMBOOL splitted;
    
    assert(radius > 0.);
    assert(thick > 0.);
    assert(thick < radius);
    
    cyl = i_create_horizontal_hollow_cylinder(radius, thick, length, no_points_circle);
    
    splitted = csmsplit_does_plane_split_solid(cyl, 0., 0., 1., 0., &cyl_top, cylinder);
    assert(splitted == CSMTRUE);
    csmsolid_free(&cyl_top);
    csmsolid_free(&cyl);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanichal7(void)
{
    struct csmsolid_t *solid;
    struct csmsolid_t *solid_aux;
    double radius, thick, length;
    unsigned long no_points_circle;
    struct csmsolid_t *hsphere_left, *hsphere_rigth;
    struct csmsolid_t *body_cylinder;
    struct csmsolid_t *top_block;
    
    radius = 0.2;
    thick = 0.01;
    length = 1.;
    no_points_circle = 32;
    
    i_set_output_debug_file("mechanical7.txt");

    csmdebug_set_enabled_by_code(CSMFALSE);
    
    i_test7_edge_spheres(radius, thick, no_points_circle, &hsphere_left, &hsphere_rigth);
    
    csmsolid_move(hsphere_left,  -(0.5 * length), 0., 0.);
    csmsolid_move(hsphere_rigth,  (0.5 * length), 0., 0.);
    
    i_test7_cylinder(radius, thick, length, no_points_circle, &body_cylinder);
    
    solid = csmsetop_union_A_and_B(hsphere_left, body_cylinder);
    
    solid_aux = solid;
    solid = csmsetop_union_A_and_B(solid, hsphere_rigth);
    csmsolid_free(&solid_aux);
    
    csmsolid_scale(solid, 1., 1.5, 1.);
    i_assign_flat_material_to_solid(0.95, 0.95, 0.95, solid);
    
    top_block = i_create_planar_block(length + 3. * radius, 4. * radius, thick);
    i_assign_flat_material_to_solid(0.95, 0.95, 0.95, top_block);
    csmsolid_move(top_block, 0., 0., -0.5 * thick);

    {
        struct csmsolid_t *difference;
        csmArrayStruct(csmsolid_t) *shells;
        struct csmsolid_t *top_block_hole, *top_block_reduced;
        
        difference = csmsetop_difference_A_minus_B(top_block, solid);
        shells = csmexplode_explode_shells(difference);

        top_block_hole = csmsolid_duplicate(csmarrayc_get_st(shells, 1, csmsolid_t));
        
        csmdebug_set_viewer_results(top_block_hole, NULL);
        csmdebug_show_viewer();
        
        //csmsolid_free(&top_block);
        //top_block = csmsolid_duplicate(top_block_hole);
        //csmsolid_scale(top_block, 1.25, 1.25, 1.);
        
        top_block_reduced = csmsolid_duplicate(top_block);
        csmsolid_scale(top_block_reduced, 0.95, 0.95, 1.);
        csmsolid_move(top_block_reduced, 0., 0., -0.5 * thick);

        solid_aux = top_block;
        top_block = csmsetop_difference_A_minus_B(top_block, top_block_reduced);
        csmsolid_free(&solid_aux);

        solid_aux = top_block;
        top_block = csmsetop_difference_A_minus_B(top_block, top_block_hole);
        csmsolid_free(&solid_aux);
        
        csmsolid_free(&difference);
        csmsolid_free(&top_block_reduced);
        csmsolid_free(&top_block_hole);
        csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
    }
    
    solid_aux = solid;
    solid = csmsetop_union_A_and_B(solid, top_block);
    csmsolid_free(&solid_aux);
    csmsolid_free(&top_block);
    
    {
        double outer_radius, inner_radius;
        struct csmshape2d_t *shape;
        struct csmsolid_t *cylinder;

        {
            outer_radius = 0.01;
            shape = csmbasicshape2d_circular_shape(outer_radius, no_points_circle);
            cylinder = csmsweep_create_solid_from_shape(shape, 0., 0., 0.1, 1., 0., 0., 0., 1., 0., shape, 0., 0., -0.20, 1., 0., 0., 0., 1., 0.);
            i_assign_flat_material_to_solid(0.9, 0.9, 0.9, cylinder);

            solid_aux = solid;
            solid = csmsetop_difference_A_minus_B(solid, cylinder);
            csmsolid_free(&solid_aux);
        
            csmsolid_free(&cylinder);
        }
        
        outer_radius = 0.02;
        inner_radius = 0.0175;
        shape = csmbasicshape2d_circular_hollow_shape(outer_radius, inner_radius, no_points_circle);
        cylinder = csmsweep_create_solid_from_shape(shape, 0., 0., 0.1, 1., 0., 0., 0., 1., 0., shape, 0., 0., -0.20, 1., 0., 0., 0., 1., 0.);
        i_assign_flat_material_to_solid(0.95, 0.95, 0.95, cylinder);
        
        {
            struct csmsolid_t *cylinder_aux, *cylinder_cutted;
            csmArrayStruct(csmsolid_t) *shells;
            
            cylinder_aux = csmsolid_duplicate(cylinder);
            csmsolid_move(cylinder_aux, .5 * length, -radius, 0.);
        
            cylinder_cutted = csmsetop_difference_A_minus_B(cylinder_aux, solid);
            shells = csmexplode_explode_shells(cylinder_cutted);
        
            solid_aux = cylinder_cutted;
            cylinder_cutted = csmsolid_duplicate(csmarrayc_get_st(shells, 0, csmsolid_t));
            csmsolid_free(&solid_aux);
            csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
        
            solid_aux = solid;
            solid = csmsetop_union_A_and_B(solid, cylinder_cutted);
            csmsolid_free(&solid_aux);
            csmsolid_free(&cylinder_cutted);
            csmsolid_free(&cylinder_aux);
        }

        {
            struct csmsolid_t *cylinder_aux, *cylinder_cutted;
            csmArrayStruct(csmsolid_t) *shells;
            
            cylinder_aux = csmsolid_duplicate(cylinder);
            csmsolid_move(cylinder_aux, .5 * length, radius, 0.);
        
            cylinder_cutted = csmsetop_difference_A_minus_B(cylinder_aux, solid);
            shells = csmexplode_explode_shells(cylinder_cutted);
        
            solid_aux = cylinder_cutted;
            cylinder_cutted = csmsolid_duplicate(csmarrayc_get_st(shells, 0, csmsolid_t));
            csmsolid_free(&solid_aux);
            csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
        
            solid_aux = solid;
            solid = csmsetop_union_A_and_B(solid, cylinder_cutted);
            csmsolid_free(&solid_aux);
            csmsolid_free(&cylinder_cutted);
            csmsolid_free(&cylinder_aux);
        }

        {
            struct csmsolid_t *cylinder_aux, *cylinder_cutted;
            csmArrayStruct(csmsolid_t) *shells;
            
            cylinder_aux = csmsolid_duplicate(cylinder);
            csmsolid_move(cylinder_aux, -.5 * length, radius, 0.);
        
            cylinder_cutted = csmsetop_difference_A_minus_B(cylinder_aux, solid);
            shells = csmexplode_explode_shells(cylinder_cutted);
        
            solid_aux = cylinder_cutted;
            cylinder_cutted = csmsolid_duplicate(csmarrayc_get_st(shells, 0, csmsolid_t));
            csmsolid_free(&solid_aux);
            csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
        
            solid_aux = solid;
            solid = csmsetop_union_A_and_B(solid, cylinder_cutted);
            csmsolid_free(&solid_aux);
            csmsolid_free(&cylinder_cutted);
            csmsolid_free(&cylinder_aux);
        }

        {
            struct csmsolid_t *cylinder_aux, *cylinder_cutted;
            csmArrayStruct(csmsolid_t) *shells;
            
            cylinder_aux = csmsolid_duplicate(cylinder);
            csmsolid_move(cylinder_aux, -.5 * length, -radius, 0.);
        
            cylinder_cutted = csmsetop_difference_A_minus_B(cylinder_aux, solid);
            shells = csmexplode_explode_shells(cylinder_cutted);
        
            solid_aux = cylinder_cutted;
            cylinder_cutted = csmsolid_duplicate(csmarrayc_get_st(shells, 0, csmsolid_t));
            csmsolid_free(&solid_aux);
            csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
        
            solid_aux = solid;
            solid = csmsetop_union_A_and_B(solid, cylinder_cutted);
            csmsolid_free(&solid_aux);
            csmsolid_free(&cylinder_cutted);
            csmsolid_free(&cylinder_aux);
        }
        
        csmshape2d_free(&shape);
        csmsolid_free(&cylinder);
    }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    
    csmsolid_set_draw_only_border_edges(solid, CSMFALSE);
    csmdebug_set_viewer_results(solid, NULL);
    csmdebug_show_viewer();
    
    csmsolid_free(&hsphere_left);
    csmsolid_free(&hsphere_rigth);
    csmsolid_free(&body_cylinder);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanichal7_simplified(void)
{
    struct csmshape2d_t *shape1, *shape2;
    struct csmsolid_t *solid1, *solid2;
    struct csmsolid_t *solid;
    
    i_set_output_debug_file("mechanical7_simplified.txt");
    
    shape1 = csmbasicshape2d_rectangular_shape(1.1, 1.1);
    shape2 = csmbasicshape2d_rectangular_hollow_shape(1., 1., 0.9, 0.9);
    
    solid1 = csmsweep_create_solid_from_shape_debug(shape1, 0., 0., 0., 1., 0., 0., 0., 1., 0., shape1, 0., 0., -0.01, 1., 0., 0., 0., 1., 0., 0);
    i_assign_flat_material_to_solid(0., 1., 1., solid1);
    
    solid2 = csmsweep_create_solid_from_shape_debug(shape2, 0., 0., 0., 1., 0., 0., 0., 1., 0., shape2, 0., 0., -0.1, 1., 0., 0., 0., 1., 0., 0);
    i_assign_flat_material_to_solid(1., 1., 1., solid2);

    solid = csmsetop_intersection_A_and_B(solid1, solid2);
    csmdebug_set_viewer_results(solid, NULL);
    csmdebug_show_viewer();
    
 {
        struct csmsolid_t *difference;
        csmArrayStruct(csmsolid_t) *shells;
        struct csmsolid_t *solid_aux;
     
        difference = csmsetop_difference_A_minus_B(solid1, solid2);
        shells = csmexplode_explode_shells(difference);

        solid_aux = solid;
        solid = csmsolid_duplicate(csmarrayc_get_st(shells, 0, csmsolid_t));
        csmsolid_free(&solid_aux);
     
        csmdebug_set_viewer_results(solid, NULL);
        csmdebug_show_viewer();
     
        csmsolid_free(&difference);
        csmarrayc_free_st(&shells, csmsolid_t, csmsolid_free);
    }
    
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid);
    csmshape2d_free(&shape1);
    csmshape2d_free(&shape2);
}

// ------------------------------------------------------------------------------------------

void csmtest_test(void)
{
    struct csmviewer_t *viewer;
    
    viewer = csmviewer_new();
    csmdebug_set_viewer(viewer, csmviewer_show, csmviewer_set_parameters, csmviewer_set_results);
    
    //csmtest_array_test1();
    //csmtest_array_test2();
    
    /*
    i_test_crea_destruye_solido_vacio();
    i_test_basico_solido_una_arista();
    i_test_crea_lamina();
    i_test_crea_lamina_con_hueco();
    i_test_crea_hexaedro();
    i_test_crea_hexaedro_y_copia();
    i_test_tabla_hash();
    i_test_solid_from_shape2D();
    i_test_solid_from_shape2D_with_hole();
    i_test_union_solidos_por_loopglue();
    */

    /*
    i_test_divide_solido_rectangular_hueco_por_plano_medio();
    i_test_divide_solido_rectangular_hueco_por_plano_medio2();
    i_test_divide_solido_rectangular_hueco_por_plano_superior();
    i_test_divide_solido_rectangular_hueco_por_plano_superior2();

    i_test_union_solidos1(viewer);
    i_test_union_solidos2(viewer);
    i_test_union_solidos6(viewer);  // --> Pendiente eliminar caras dentro de caras
    
    i_test_interseccion_solidos1(viewer);
    i_test_interseccion_solidos2(viewer);
    i_test_interseccion_solidos3(viewer);
    i_test_interseccion_solidos4(viewer);
    i_test_interseccion_solidos5(viewer);
    i_test_interseccion_solidos7(viewer);
    i_test_resta_solidos1(viewer);
    i_test_resta_solidos2(viewer);
    i_test_multiple_solidos1(viewer);
    i_test_multiple_solidos2(viewer);
    i_test_multiple_solidos3(viewer);
    
    i_test_cilindro1(viewer);
    i_test_cilindro2(viewer);
    i_test_cilindro3(viewer);
    i_test_cilindro4(viewer); // --> Revisar la orientación de las caras del hueco, falla split a 0,75. Assert de puntos repetidos al realizar la diferencia, arista nula no borrada?
    i_test_cilindro5(viewer); // -- Intersecciones non-manifold.
    i_test_cilindro6(viewer); // --> Intersecciones non-manifold.
    i_test_cilindro7(viewer); // --> Intersecciones non-manifold.
    i_test_cilindro8(viewer); // --> Intersecciones non-manifold.
    
    //i_test_cilindro9(viewer); // --> Intersecciones non-manifold.
                              // --> Detectar situación de error y gestionarla correctamente, la unión no tiene sentido porque no se puede realizar a través de una cara
                              // --> No manipular las intersecciones non-manifold, parece que el caso out-on-out se gestiona correctamente.

    i_test_mechanical_part1_redux();
    i_test_mechanical_part1();
    i_test_mechanical_part2();
    
    i_test_toroide();
    i_test_cono();
    i_test_sphere();

    i_test_sphere2();
    i_test_sphere3();
    
    i_test_sphere4();
    
    i_test_mechanical5();
    i_test_mechanical6bis();
    i_test_mechanical6();

    i_test_mechanichal7_simplified();
     */
    i_test_mechanichal7();
    
    csmviewer_free(&viewer);
}












