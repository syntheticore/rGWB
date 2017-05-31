// Basic Mathematic Functions...

#include "csmfwddecl.hxx"

enum csmmath_double_relation_t csmmath_compare_doubles(double value1, double value2, double epsilon);

enum csmmath_dropped_coord_t csmmath_dropped_coord(double x, double y, double z);

void csmmath_select_not_dropped_coords(
                        double x, double y, double z,
                        enum csmmath_dropped_coord_t dropped_coord,
                        double *x_not_dropped, double *y_not_dropped);

CYBOOL csmmath_equal_coords(
                        double x1, double y1, double z1,
                        double x2, double y2, double z2,
                        double epsilon);

// Vectors...

double csmmath_length_vector3D(double x, double y, double z);

double csmmath_distance_3D(double x1, double y1, double z1, double x2, double y2, double z2);

void csmmath_make_unit_vector3D(double *Ux, double *Uy, double *Uz);

void csmmath_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz);

void csmmath_unit_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz);

double csmmath_dot_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz);

void csmmath_cross_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz, double *Wx, double *Wy, double *Wz);

void csmmath_move_point(
						double x, double y, double z,
						double Ux, double Uy, double Uz, double desp,
                        double *x_desp, double *y_desp, double *z_desp);

CYBOOL csmmath_is_null_vector(double Ux, double Uy, double Uz, double tolerance);


// Lines...

double csmmath_distance_from_point_to_line3D(
						double x, double y, double z,
                        double Xo_recta, double Yo_recta, double Zo_recta, double Ux_recta, double Uy_recta, double Uz_recta);

void csmmath_nearer_points_between_two_lines3D(
                        double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
                        double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
                        double *x_mas_cercano_recta1, double *y_mas_cercano_recta1, double *z_mas_cercano_recta1,
                        double *x_mas_cercano_recta2, double *y_mas_cercano_recta2, double *z_mas_cercano_recta2);

CYBOOL csmmath_is_point_in_segment3D(
						double x, double y, double z,
						double x1, double y1, double z1, double x2, double y2, double z2,
						double precision, 
                        double *posicion_relativa_opc);

CYBOOL csmmath_exists_intersection_between_two_lines3D(
						double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
						double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
						double precision, 
                        double *x_corte, double *y_corte, double *z_corte);

CYBOOL csmmath_exists_intersection_between_two_segments3D(
						double x1_seg1, double y1_seg1, double z1_seg1, double x2_seg1, double y2_seg1, double z2_seg1, 
						double x1_seg2, double y1_seg2, double z1_seg2, double x2_seg2, double y2_seg2, double z2_seg2, 
						double precision, 
						double *x_corte_opc, double *y_corte_opc, double *z_corte_opc,
						double *posicion_relativa1_opc, 
                        double *posicion_relativa2_opc);

// Planes...

double csmmath_signed_distance_point_to_plane(double x, double y, double z, double A, double B, double C, double D);

void csmmath_implicit_plane_equation(
						double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz,double Vx, double Vy, double Vz,
                        double *A, double *B, double *C, double *D);

void csmmath_plane_axis_from_implicit_plane_equation(
						double A, double B, double C, double D,
						double *Xo, double *Yo, double *Zo, 
                        double *Ux, double *Uy, double *Uz, double *Vx, double *Vy, double *Vz);
