// Face...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmface_t *, csmface_new, (struct csmsolid_t *solid, unsigned long *id_new_element));

CONSTRUCTOR(struct csmface_t *, csmface_duplicate, (
                        struct csmface_t *face,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_new_element,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new));

CONSTRUCTOR(struct csmface_t *, csmface_new_from_writeable_face, (
                        const struct csmwriteablesolid_face_t *w_face,
                        struct csmsolid_t *solid,
                        struct csmhashtb(csmvertex_t) *svertexs,
                        struct csmhashtb(csmhedge_t) *created_shedges));

void csmface_free(struct csmface_t **face);

unsigned long csmface_id(const struct csmface_t *face);

void csmface_reassign_id(struct csmface_t *face, unsigned long *id_new_element, unsigned long *new_id_opc);


const struct csmmaterial_t *csmface_get_visualization_material(const struct csmface_t *face);

void csmface_set_visualization_material(struct csmface_t *face, const struct csmmaterial_t *visz_material_opt);


const struct csmsurface_t *csmface_get_surface_eq(const struct csmface_t *face);

void csmface_set_surface_eq(struct csmface_t *face, const struct csmsurface_t *surface_eq);


void csmface_copy_attributes_from_face1(const struct csmface_t *face1, struct csmface_t *face2);


// Geometry...

void csmface_redo_geometric_generated_data(struct csmface_t *face);
void csmface_mark_geometric_generated_data_needs_update(struct csmface_t *face);

CSMBOOL csmface_should_analyze_intersections_between_faces(const struct csmface_t *face1, const struct csmface_t *face2);

CSMBOOL csmface_should_analyze_intersections_with_segment(
	                    const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2);

CSMBOOL csmface_contains_vertex(
                        const struct csmface_t *face,
                        const struct csmvertex_t *vertex,
                        const struct csmtolerance_t *tolerances,
                        enum csmmath_containment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc);

CSMBOOL csmface_contains_point(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances,
                        enum csmmath_containment_point_loop_t *type_of_containment_opc,
                        struct csmvertex_t **hit_vertex_opc,
                        struct csmhedge_t **hit_hedge_opc, double *t_relative_to_hit_hedge_opc);

CONSTRUCTOR(const csmArrayStruct(csmloop_t) *, csmface_get_inner_loops_with_area, (const struct csmface_t *face));

CSMBOOL csmface_is_point_interior_to_face_optimized_laringmv(
                        const struct csmface_t *face, const csmArrayStruct(csmloop_t) *face_inner_loops_with_area,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances);

CSMBOOL csmface_is_point_interior_to_face(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances);

CSMBOOL csmface_is_vertex_used_by_hedge_on_face(const struct csmface_t *face, const struct csmvertex_t *vertex);

enum csmcompare_t csmface_classify_vertex_relative_to_face(const struct csmface_t *face, const struct csmvertex_t *vertex);

CSMBOOL csmface_exists_intersection_between_line_and_face_plane(
                        const struct csmface_t *face,
                        double x1, double y1, double z1, double x2, double y2, double z2,
                        double *x_inters_opc, double *y_inters_opc, double *z_inters_opc, double *t_inters_opc);

CSMBOOL csmface_is_loop_contained_in_face(
                        struct csmface_t *face,
                        struct csmloop_t *loop,
                        const struct csmtolerance_t *tolerances);

CSMBOOL csmface_is_point_interior_to_face_outer_loop(
                        const struct csmface_t *face,
                        double x, double y, double z,
                        const struct csmtolerance_t *tolerances);

CSMBOOL csmface_is_loop_contained_in_face_outer_loop(
                        struct csmface_t *face,
                        struct csmloop_t *loop,
                        const struct csmtolerance_t *tolerances);

double csmface_tolerace(const struct csmface_t *face);

CSMBOOL csmface_is_coplanar_to_plane(
                        const struct csmface_t *face,
                        double A, double B, double C, double D,
                        const struct csmtolerance_t *tolerances,
                        CSMBOOL *same_orientation);

CSMBOOL csmface_are_coplanar_faces(struct csmface_t *face1, const struct csmface_t *face2, const struct csmtolerance_t *tolerances, CSMBOOL *same_sense_opc);

CSMBOOL csmface_are_coplanar_faces_at_common_base_vertex(struct csmface_t *face1, const struct csmface_t *face2, const struct csmtolerance_t *tolerances);

CSMBOOL csmface_faces_define_border_edge(const struct csmface_t *face1, const struct csmface_t *face2);

CSMBOOL csmface_is_oriented_in_direction(const struct csmface_t *face, double Wx, double Wy, double Wz);

void csmface_face_equation(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D);

void csmface_face_baricenter(const struct csmface_t *face, double *x, double *y, double *z);

void csmface_face_equation_info(
                        const struct csmface_t *face,
                        double *A, double *B, double *C, double *D);

void csmface_maximize_bbox(const struct csmface_t *face, struct csmbbox_t *bbox);

double csmface_loop_area_in_face(const struct csmface_t *face, const struct csmloop_t *loop);

void csmface_reorient_loops_in_face(struct csmface_t *face, const struct csmtolerance_t *tolerances);


// Topology...

struct csmsolid_t *csmface_fsolid(struct csmface_t *face);
void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid);

struct csmsolid_t *csmface_fsolid_aux(struct csmface_t *face);
void csmface_set_fsolid_aux(struct csmface_t *face, struct csmsolid_t *solid);

struct csmloop_t *csmface_flout(struct csmface_t *face);
void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop);

struct csmloop_t *csmface_floops(struct csmface_t *face);
void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop);

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop);

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop);

CSMBOOL csmface_has_loops(const struct csmface_t *face);

CSMBOOL csmface_has_holes(const struct csmface_t *face);

void csmface_revert(struct csmface_t *face);


// Algorithm marks...

void csmface_clear_algorithm_mask(struct csmface_t *face);

CSMBOOL csmface_is_setop_null_face(const struct csmface_t *face);
void csmface_mark_setop_null_face(struct csmface_t *face);

CSMBOOL csmface_setop_has_been_modified(const struct csmface_t *face);

CSMBOOL csmface_setop_has_shell_id(const struct csmface_t *face);
unsigned long csmface_setop_shell_id(const struct csmface_t *face);
void csmface_set_setop_shell_id(struct csmface_t *face, unsigned long shell_id);

void csmface_simplifyop_mark_skip_face(struct csmface_t *face);
CSMBOOL csmface_simplifyop_skip_face(const struct csmface_t *face);

// Debug...

void csmface_print_info_debug(struct csmface_t *face, CSMBOOL assert_si_no_es_integro, unsigned long *num_holes_opc);


// Visualization...

#ifdef RGWB_STANDALONE_DISTRIBUTABLE

void csmface_draw_solid(
                    struct csmface_t *face,
                    CSMBOOL draw_solid_face,
                    CSMBOOL draw_face_normal,
                    const struct bsmaterial_t *face_material,
                    const struct bsmaterial_t *normal_material,
                    struct bsgraphics2_t *graphics);

void csmface_draw_normal(struct csmface_t *face, struct bsgraphics2_t *graphics);

void csmface_draw_edges(
                    struct csmface_t *face,
                    const struct bsmaterial_t *outer_loop,
                    const struct bsmaterial_t *hole_loop,
                    const struct bsmaterial_t *inner_non_hole_loop,
                    struct bsgraphics2_t *graphics);


#else

void csmface_append_datos_mesh(
                    struct csmface_t *face,
                    ArrPunto3D *puntos, ArrPunto3D *normales, ArrBool *es_borde,
                    ArrEnum(cplan_tipo_primitiva_t) *tipo_primitivas, ArrPuntero(ArrULong) *inds_caras);

void csmface_append_cara_solido(
                    struct csmface_t *face,
                    CSMBOOL only_faces_towards_direction, double Wx, double Wy, double Wz, double tolerance_rad, 
                    ArrArrPuntero(ArrPunto3D) *caras_solido, ArrEstructura(ejes2d_t) *ejes_caras_solido);

#endif
