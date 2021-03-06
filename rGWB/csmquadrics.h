//
//  csmquadrics.h
//  rGWB
//
//  Created by Manuel Fernández on 25/9/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmquadrics_create_torus, (
                        double R, unsigned long no_points_circle_R,
                        double r, unsigned long no_points_circle_r,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmquadrics_create_cone, (
                        double height, double radius, unsigned long no_points_circle_radius,
                        double x_base_center, double y_base_center, double z_base_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmquadrics_create_sphere, (
                        double radius,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmquadrics_create_ellipsoid, (
                        double radius_x, double radius_y, double radius_z,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));

DLL_RGWB CONSTRUCTOR(struct csmsolid_t *, csmquadrics_create_hyperbolid_one_sheet, (
                        double a, double c, double height,
                        unsigned long no_points_circle_radius_parallels_semisphere,
                        unsigned long no_points_circle_radius_meridians,
                        double x_center, double y_center, double z_center,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        unsigned long start_id_of_new_element));

#ifdef __cplusplus
}
#endif
