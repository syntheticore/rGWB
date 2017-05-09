//
//  csmmath.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmath.inl"
#include "csmmath.tli"

#include "cyassert.h"
#include "defmath.tlh"
#include <math.h>

// ------------------------------------------------------------------------------------------

enum csmmath_double_relation_t csmmath_compare_doubles(double value1, double value2, double epsilon)
{
    double diff;
    
    diff = fabs(value1 - value2);
    
    if (diff < epsilon)
        return CSMMATH_EQUAL_VALUES;
    else if (value1 < value2)
        return CSMMATH_VALUE1_LESS_THAN_VALUE2;
    else
        return CSMMATH_VALUE1_GREATER_THAN_VALUE2;
}

// ------------------------------------------------------------------------------------------

enum csmmath_dropped_coord_t csmmath_dropped_coord(double x, double y, double z)
{
    double x_abs, y_abs, z_abs;
    
    x_abs = fabs(x);
    y_abs = fabs(y);
    z_abs = fabs(z);
    
    if (x_abs >= y_abs && x_abs >= z_abs)
        return CSMMATH_DROPPED_COORD_X;
    else if (y_abs >= x_abs && y_abs >= z_abs)
        return CSMMATH_DROPPED_COORD_Y;
    else
        return CSMMATH_DROPPED_COORD_Z;
}

// ------------------------------------------------------------------------------------------

void csmmath_select_not_dropped_coords(
                        double x, double y, double z,
                        enum csmmath_dropped_coord_t dropped_coord,
                        double *x_not_dropped, double *y_not_dropped)
{
    assert_no_null(x_not_dropped);
    assert_no_null(y_not_dropped);
    
    switch (dropped_coord)
    {
        case CSMMATH_DROPPED_COORD_X:
            
            *x_not_dropped = y;
            *y_not_dropped = z;
            break;
            
        case CSMMATH_DROPPED_COORD_Y:
            
            *x_not_dropped = x;
            *y_not_dropped = z;
            break;
            
        case CSMMATH_DROPPED_COORD_Z:
            
            *x_not_dropped = x;
            *y_not_dropped = y;
            break;
            
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

CYBOOL csmmath_equal_coords(
                        double x1, double y1, double z1,
                        double x2, double y2, double z2,
                        double epsilon)
{
    double diff;
    
    diff = CUAD(x1 - x2) + CUAD(y1 - y2) + CUAD(z1 - z2);

    if (csmmath_compare_doubles(diff, 0.0, CUAD(epsilon)) == CSMMATH_EQUAL_VALUES)
        return CIERTO;
    else
        return FALSO;
}

// ------------------------------------------------------------------------------------------

double csmmath_length_vector3D(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

// ------------------------------------------------------------------------------------------

double csmmath_distance_3D(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return csmmath_length_vector3D(x2 - x1, y2 - y1, z2 - z1);
}

// ------------------------------------------------------------------------------------------

void csmmath_make_unit_vector3D(double *Ux, double *Uy, double *Uz)
{
    double length;
    
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);
    
    length = csmmath_length_vector3D(*Ux, *Uy, *Uz);
    assert(length > 0.);
    
    *Ux /= length;
    *Uy /= length;
    *Uz /= length;
}

//-------------------------------------------------------------------------------------------

void csmmath_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz)
{
	assert_no_null(Ux);
	assert_no_null(Uy);
	assert_no_null(Uz);
    
	*Ux = x2 - x1;
	*Uy = y2 - y1;
	*Uz = z2 - z1;
}

// ------------------------------------------------------------------------------------------

void csmmath_unit_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz)
{
    csmmath_vector_between_two_3D_points(x1, y1, z1, x2, y2, z2, Ux, Uy, Uz);
    csmmath_make_unit_vector3D(Ux, Uy, Uz);
}

// ------------------------------------------------------------------------------------------

double csmmath_dot_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz)
{
    return Ux * Vx + Uy * Vy + Uz * Vz;
}

//-------------------------------------------------------------------------------------------

void csmmath_cross_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz, double *Wx, double *Wy, double *Wz)
{
	assert_no_null(Wx);
	assert_no_null(Wy);
	assert_no_null(Wz);

	*Wx = (Uy * Vz - Vy * Uz);
	*Wy = -(Ux * Vz - Vx * Uz);
	*Wz = (Ux * Vy - Vx * Uy);
}

//-------------------------------------------------------------------------------------------

void csmmath_move_point(
						double x, double y, double z,
						double Ux, double Uy, double Uz, double desp,
						double *x_desp, double *y_desp, double *z_desp)
{
	assert_no_null(x_desp);
	assert_no_null(y_desp);
	assert_no_null(z_desp);

	*x_desp = x + Ux * desp;
	*y_desp = y + Uy * desp;
	*z_desp = z + Uz * desp;
}

//-------------------------------------------------------------------------------------------

double csmmath_distance_from_point_to_line3D(
						double x, double y, double z,
						double Xo_recta, double Yo_recta, double Zo_recta, double Ux_recta, double Uy_recta, double Uz_recta)
{
	double Ux_recta_punto, Uy_recta_punto, Uz_recta_punto;
    double Wx, Wy, Wz;
    
    csmmath_vector_between_two_3D_points(Xo_recta, Yo_recta, Zo_recta, x, y, z, &Ux_recta_punto, &Uy_recta_punto, &Uz_recta_punto);
    csmmath_cross_product3D(Ux_recta_punto, Uy_recta_punto, Uz_recta_punto, Ux_recta, Uy_recta, Uz_recta, &Wx, &Wy, &Wz);
    
    return csmmath_length_vector3D(Wx, Wy, Wz);
}

//-------------------------------------------------------------------------------------------

static void i_valida_es_vector_unitario_3D(double Ux, double Uy, double Uz)
{
	double modulo;

	modulo = csmmath_length_vector3D(Ux, Uy, Uz);
	assert(modulo - 1. < 1.e-6);
}

//-------------------------------------------------------------------------------------------

static void i_point_on_line3D(
							double Xo_recta, double Yo_recta, double Zo_recta, double Ux_recta, double Uy_recta, double Uz_recta,
							double posicion,
							double *x, double *y, double *z)
{
	assert_no_null(x);
	assert_no_null(y);
	assert_no_null(z);

	*x = Xo_recta + posicion * Ux_recta;
	*y = Yo_recta + posicion * Uy_recta;
	*z = Zo_recta + posicion * Uz_recta;
}

//-------------------------------------------------------------------------------------------

static CYBOOL i_son_rectas_paralelas_segun_determinante(double determinante)
{
	if (fabs(determinante) < 1.e-6)
		return CIERTO;
	else
		return FALSO;
}

//-------------------------------------------------------------------------------------------

void csmmath_nearer_points_between_two_lines3D(
							double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
							double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
							double *x_mas_cercano_recta1, double *y_mas_cercano_recta1, double *z_mas_cercano_recta1,
							double *x_mas_cercano_recta2, double *y_mas_cercano_recta2, double *z_mas_cercano_recta2)
{
	double determinante;
	double prod_escalar_vectores_directores;

	i_valida_es_vector_unitario_3D(Ux_recta1, Uy_recta1, Uz_recta1);
	i_valida_es_vector_unitario_3D(Ux_recta2, Uy_recta2, Uz_recta2);
	assert_no_null(x_mas_cercano_recta1);
	assert_no_null(y_mas_cercano_recta1);
	assert_no_null(z_mas_cercano_recta1);
	assert_no_null(x_mas_cercano_recta2);
	assert_no_null(y_mas_cercano_recta2);
	assert_no_null(z_mas_cercano_recta2);

	prod_escalar_vectores_directores = csmmath_dot_product3D(Ux_recta1, Uy_recta1, Uz_recta1, Ux_recta2, Uy_recta2, Uz_recta2);
	determinante = CUAD(prod_escalar_vectores_directores) - 1.;

	if (i_son_rectas_paralelas_segun_determinante(determinante) == CIERTO)
	{
		*x_mas_cercano_recta1 = Xo_recta1;
		*y_mas_cercano_recta1 = Yo_recta1;
		*z_mas_cercano_recta1 = Zo_recta1;

		*x_mas_cercano_recta2 = Xo_recta2;
		*y_mas_cercano_recta2 = Yo_recta2;
		*z_mas_cercano_recta2 = Zo_recta2;
	}
	else
	{
		double Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas;
		double inv_determinante;
		double prod_escalar1, prod_escalar2;
		double t1, t2;

		Ux_entre_rectas =  Xo_recta2 - Xo_recta1;
		Uy_entre_rectas =  Yo_recta2 - Yo_recta1;
		Uz_entre_rectas =  Zo_recta2 - Zo_recta1;

		inv_determinante = 1. / determinante;
		prod_escalar1 = csmmath_dot_product3D(Ux_recta1, Uy_recta1, Uz_recta1, Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas);
		prod_escalar2 = csmmath_dot_product3D(Ux_recta2, Uy_recta2, Uz_recta2, Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas);

		t1 = inv_determinante * (-prod_escalar1 + prod_escalar_vectores_directores * prod_escalar2);
		t2 = inv_determinante * (-prod_escalar_vectores_directores * prod_escalar1 + prod_escalar2);

		i_point_on_line3D(Xo_recta1, Yo_recta1, Zo_recta1, Ux_recta1, Uy_recta1, Uz_recta1, t1, x_mas_cercano_recta1, y_mas_cercano_recta1, z_mas_cercano_recta1);
		i_point_on_line3D(Xo_recta2, Yo_recta2, Zo_recta2, Ux_recta2, Uy_recta2, Uz_recta2, t2, x_mas_cercano_recta2, y_mas_cercano_recta2, z_mas_cercano_recta2);
	}
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_is_point_in_segment3D(
						double x, double y, double z,
						double x1, double y1, double z1, double x2, double y2, double z2,
						double precision, 
						double *t_opc)
{
    CYBOOL is_point_on_segment;
    CYBOOL is_point_on_line;
    double t_loc;
    double Ux_seg, Uy_seg, Uz_seg;
    double squared_dot_product_seg;

	assert(precision > 0.);
	
	Ux_seg = x2 - x1;
	Uy_seg = y2 - y1;
	Uz_seg = z2 - z1;
    
    squared_dot_product_seg = csmmath_dot_product3D(Ux_seg, Uy_seg, Uz_seg, Ux_seg, Uy_seg, Uz_seg);
    
    if (squared_dot_product_seg < CUAD(precision))
    {
        is_point_on_line = csmmath_equal_coords(x, y, z, x1, y1, z1, precision);
        t_loc = 0.;
    }
    else
    {
        double Ux_to_point, Uy_to_point, Uz_to_point;
        double t_prime;
        double x_proj_seg, y_proj_seg, z_proj_seg;
        
        Ux_to_point = x - x1;
        Uy_to_point = y - y1;
        Uz_to_point = z - z1;
        
        t_prime = csmmath_dot_product3D(Ux_seg, Uy_seg, Uz_seg, Ux_to_point, Uy_to_point, Uz_to_point) / squared_dot_product_seg;
        
        x_proj_seg = x1 + t_prime * Ux_seg;
        y_proj_seg = y1 + t_prime * Uy_seg;
        z_proj_seg = z1 + t_prime * Uz_seg;
        
        is_point_on_line = csmmath_equal_coords(x, y, z, x_proj_seg, y_proj_seg, z_proj_seg, precision);
        t_loc = t_prime;
    }
    
    if (is_point_on_line == CIERTO && t_loc >= -precision && t_loc <= 1. + precision)
        is_point_on_segment = CIERTO;
    else
        is_point_on_segment = FALSO;
    
    ASIGNA_OPC(t_opc, t_loc);
    
    return is_point_on_segment;
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_exists_intersection_between_two_lines3D(
						double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
						double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
						double precision, 
						double *x_corte, double *y_corte, double *z_corte)
{
	double x_mas_cercano_recta1, y_mas_cercano_recta1, z_mas_cercano_recta1;
	double x_mas_cercano_recta2, y_mas_cercano_recta2, z_mas_cercano_recta2;
	double squared_distance;
	
	assert(precision > 0.);
	assert_no_null(x_corte);
	assert_no_null(y_corte);
	assert_no_null(z_corte);

	csmmath_nearer_points_between_two_lines3D(
                        Xo_recta1, Yo_recta1, Zo_recta1, Ux_recta1, Uy_recta1, Uz_recta1,
                        Xo_recta2, Yo_recta2, Zo_recta2, Ux_recta2, Uy_recta2, Uz_recta2,
                        &x_mas_cercano_recta1, &y_mas_cercano_recta1, &z_mas_cercano_recta1,
                        &x_mas_cercano_recta2, &y_mas_cercano_recta2, &z_mas_cercano_recta2);
			
    squared_distance = CUAD(x_mas_cercano_recta2 - x_mas_cercano_recta1) + CUAD(y_mas_cercano_recta2 - y_mas_cercano_recta1) + CUAD(z_mas_cercano_recta2 - z_mas_cercano_recta1);
							
	if (squared_distance <= CUAD(precision))
	{	
		*x_corte = x_mas_cercano_recta1;
		*y_corte = y_mas_cercano_recta1;
		*z_corte = z_mas_cercano_recta1;
	
		return CIERTO;
	}
	else
	{
		return FALSO;
	}
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_exists_intersection_between_two_segments3D(
						double x1_seg1, double y1_seg1, double z1_seg1, double x2_seg1, double y2_seg1, double z2_seg1, 
						double x1_seg2, double y1_seg2, double z1_seg2, double x2_seg2, double y2_seg2, double z2_seg2, 
						double precision, 
						double *x_corte_opc, double *y_corte_opc, double *z_corte_opc,
						double *posicion_relativa1_opc, 
						double *posicion_relativa2_opc)
{
	CYBOOL hay_interseccion;
	double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
	double x_corte_loc, y_corte_loc, z_corte_loc;
	double posicion_relativa1_loc, posicion_relativa2_loc;
	
	csmmath_unit_vector_between_two_3D_points(x1_seg1, y1_seg1, z1_seg1, x2_seg1, y2_seg1, z2_seg1, &Ux1, &Uy1, &Uz1);
	csmmath_unit_vector_between_two_3D_points(x1_seg2, y1_seg2, z1_seg2, x2_seg2, y2_seg2, z2_seg2, &Ux2, &Uy2, &Uz2);
	
	if (csmmath_exists_intersection_between_two_lines3D(
						x1_seg1, y1_seg1, z1_seg1, Ux1, Uy1, Uz1, 
						x1_seg2, y1_seg2, z1_seg2, Ux2, Uy2, Uz2,
						precision, 
						&x_corte_loc, &y_corte_loc, &z_corte_loc) == FALSO)
	{	
		hay_interseccion = FALSO;
		
		x_corte_loc = 0.;
		y_corte_loc = 0.;
		z_corte_loc = 0.;
		
		posicion_relativa1_loc = 0.;
		posicion_relativa2_loc = 0.;
	}
	else
	{
		if (csmmath_is_point_in_segment3D(
						x_corte_loc, y_corte_loc, z_corte_loc,
						x1_seg1, y1_seg1, z1_seg1, x2_seg1, y2_seg1, z2_seg1, 
						precision, 
						&posicion_relativa1_loc) == FALSO)
		{	
			hay_interseccion = FALSO;

			x_corte_loc = 0.;
			y_corte_loc = 0.;
			z_corte_loc = 0.;

			posicion_relativa1_loc = 0.;
			posicion_relativa2_loc = 0.;
		}
		else if (csmmath_is_point_in_segment3D(
						x_corte_loc, y_corte_loc, z_corte_loc,
						x1_seg2, y1_seg2, z1_seg2, x2_seg2, y2_seg2, z2_seg2, 
						precision, 
						&posicion_relativa2_loc) == FALSO)
		{
			hay_interseccion = FALSO;

			x_corte_loc = 0.;
			y_corte_loc = 0.;
			z_corte_loc = 0.;

			posicion_relativa1_loc = 0.;
			posicion_relativa2_loc = 0.;
		}
		else
		{	
			hay_interseccion = FALSO;
		}
	}
	
	ASIGNA_OPC(x_corte_opc, x_corte_loc);
	ASIGNA_OPC(y_corte_opc, y_corte_loc);
	ASIGNA_OPC(z_corte_opc, z_corte_loc);
	ASIGNA_OPC(posicion_relativa1_opc, posicion_relativa1_loc);
	ASIGNA_OPC(posicion_relativa2_opc, posicion_relativa2_loc);

	return hay_interseccion;
}

// ------------------------------------------------------------------------------------------

double csmmath_signed_distance_point_to_plane(double x, double y, double z, double A, double B, double C, double D)
{
	return A * x + B * y + C * z + D;
}


//-------------------------------------------------------------------------------------------

void csmmath_implicit_plane_equation(
						double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz,double Vx, double Vy, double Vz,
						double *A, double *B, double *C, double *D)
{
	assert_no_null(A);
	assert_no_null(B);
	assert_no_null(C);
	assert_no_null(D);

	csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, A, B, C);
	csmmath_make_unit_vector3D(A, B, C);
	
	*D = -csmmath_dot_product3D(*A, *B, *C, Xo, Yo, Zo);
}

//-------------------------------------------------------------------------------------------

static void i_anula_valores_despreciables(double *valor)
{
	assert_no_null(valor);
	
	if (ABS(*valor) < 1.e-20)
		*valor = 0.;
}

//-------------------------------------------------------------------------------------------

void csmmath_plane_axis_from_implicit_plane_equation(
						double A, double B, double C, double D,
						double *Xo, double *Yo, double *Zo, 
						double *Ux, double *Uy, double *Uz, double *Vx, double *Vy, double *Vz)
{
	double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
	
	assert_no_null(Ux);
	assert_no_null(Uy);
	assert_no_null(Uz);
	assert_no_null(Vx);
	assert_no_null(Vy);
	assert_no_null(Vz);
	
	if (ABS(A) < 1.e-6 && ABS(B) < 1.e-6 && ABS(C) > 1.e-6)
	{	
		Ux1 = 1.;
		Uy1 = 0.;
		Uz1 = 0.;
	}
	else
	{
		csmmath_cross_product3D(0., 0., 1., A, B, C, &Ux1, &Uy1, &Uz1);
		i_anula_valores_despreciables(&Ux1);
		i_anula_valores_despreciables(&Uy1);
		i_anula_valores_despreciables(&Uz1);		
		csmmath_make_unit_vector3D(&Ux1, &Uy1, &Uz1);
	}
		
	csmmath_cross_product3D(A, B, C, Ux1, Uy1, Uz1, &Ux2, &Uy2, &Uz2);
	i_anula_valores_despreciables(&Ux2);
	i_anula_valores_despreciables(&Uy2);
	i_anula_valores_despreciables(&Uz2);		
	csmmath_make_unit_vector3D(&Ux2, &Uy2, &Uz2);
	
	csmmath_move_point(0., 0., 0., A, B, C, -D, Xo, Yo, Zo);
	
	*Ux = Ux1;
	*Uy = Uy1;
	*Uz = Uz1;
	
	*Vx = Ux2;
	*Vy = Uy2;
	*Vz = Uz2;
}
