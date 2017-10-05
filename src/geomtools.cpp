/*
 val3dity - Copyright (c) 2011-2016, Hugo Ledoux.  All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the authors nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL HUGO LEDOUX BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*/

#include "geomtools.h"
#include "CGAL/squared_distance_3.h"
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/minkowski_sum_3.h>
#include <CGAL/OFF_to_nef_3.h>
#include <CGAL/Bbox_3.h>

namespace val3dity
{

void mark_domains(CT& ct,
  CT::Face_handle start,
  int index,
  std::list<CT::Edge>& border) 
{
  if (start->info().nesting_level != -1) {
    return;
  }
  std::list<CT::Face_handle> queue;
  queue.push_back(start);
  while (!queue.empty()) {
    CT::Face_handle fh = queue.front();
    queue.pop_front();
    if (fh->info().nesting_level == -1) {
      fh->info().nesting_level = index;
      for (int i = 0; i < 3; i++) {
        CT::Edge e(fh, i);
        CT::Face_handle n = fh->neighbor(i);
        if (n->info().nesting_level == -1) {
          if (ct.is_constrained(e)) border.push_back(e);
          else queue.push_back(n);
        }
      }
    }
  }
}

//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident 
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void mark_domains(CT& ct) {
  for (CT::All_faces_iterator it = ct.all_faces_begin(); it != ct.all_faces_end(); ++it) {
    it->info().nesting_level = -1;
  }
  std::list<CT::Edge> border;
  mark_domains(ct, ct.infinite_face(), 0, border);
  while (!border.empty()) {
    CT::Edge e = border.front();
    border.pop_front();
    CT::Face_handle n = e.first->neighbor(e.second);
    if (n->info().nesting_level == -1) {
      mark_domains(ct, n, e.first->info().nesting_level + 1, border);
    }
  }
}  

Nef_polyhedron* get_structuring_element_dodecahedron(float r)
{
  std::stringstream ss;
  ss << "OFF" << std::endl
    << "20 36 0"      << std::endl
    << "-0.57735 -0.57735 0.57735" << std::endl
    << "0.934172 0.356822 0" << std::endl
    << "0.934172 -0.356822 0" << std::endl
    << "-0.934172 0.356822 0" << std::endl
    << "-0.934172 -0.356822 0" << std::endl
    << "0 0.934172 0.356822" << std::endl
    << "0 0.934172 -0.356822" << std::endl
    << "0.356822 0 -0.934172" << std::endl
    << "-0.356822 0 -0.934172" << std::endl
    << "0 -0.934172 -0.356822" << std::endl
    << "0 -0.934172 0.356822" << std::endl
    << "0.356822 0 0.934172" << std::endl
    << "-0.356822 0 0.934172" << std::endl
    << "0.57735 0.57735 -0.57735" << std::endl
    << "0.57735 0.57735 0.57735" << std::endl
    << "-0.57735 0.57735 -0.57735" << std::endl
    << "-0.57735 0.57735 0.57735" << std::endl
    << "0.57735 -0.57735 -0.57735" << std::endl
    << "0.57735 -0.57735 0.57735" << std::endl
    << "-0.57735 -0.57735 -0.57735" << std::endl
    << "3 18 2 1" << std::endl
    << "3 11 18 1" << std::endl
    << "3 14 11 1" << std::endl
    << "3 7 13 1" << std::endl
    << "3 17 7 1" << std::endl
    << "3 2 17 1" << std::endl
    << "3 19 4 3" << std::endl
    << "3 8 19 3" << std::endl
    << "3 15 8 3" << std::endl
    << "3 12 16 3" << std::endl
    << "3 0 12 3" << std::endl
    << "3 4 0 3" << std::endl
    << "3 6 15 3" << std::endl
    << "3 5 6 3" << std::endl
    << "3 16 5 3" << std::endl
    << "3 5 14 1" << std::endl
    << "3 6 5 1" << std::endl
    << "3 13 6 1" << std::endl
    << "3 9 17 2" << std::endl
    << "3 10 9 2" << std::endl
    << "3 18 10 2" << std::endl
    << "3 10 0 4" << std::endl
    << "3 9 10 4" << std::endl
    << "3 19 9 4" << std::endl
    << "3 19 8 7" << std::endl
    << "3 9 19 7" << std::endl
    << "3 17 9 7" << std::endl
    << "3 8 15 6" << std::endl
    << "3 7 8 6" << std::endl
    << "3 13 7 6" << std::endl
    << "3 11 14 5" << std::endl
    << "3 12 11 5" << std::endl
    << "3 16 12 5" << std::endl
    << "3 12 0 10" << std::endl
    << "3 11 12 10" << std::endl
    << "3 18 11 10" << std::endl;
  Nef_polyhedron* myse = new Nef_polyhedron;
  CGAL::OFF_to_nef_3(ss, *myse);
  Transformation scale(CGAL::SCALING, r);
  myse->transform(scale);
  return myse;
}

Nef_polyhedron* get_structuring_element_cube(float r)
{
  std::stringstream ss;
  ss << "OFF"        << std::endl
     << "8 6 0"      << std::endl
     << "1 1 1"      << std::endl
     << "-1 1 1"     << std::endl
     << "1 -1 1"     << std::endl
     << "-1 -1 1"    << std::endl
     << "1 1 -1 "    << std::endl
     << "-1 1 -1"    << std::endl
     << "1 -1 -1"    << std::endl
     << "-1 -1 -1"   << std::endl
     << "4 1 3 2 0"  << std::endl
     << "4 2 6 4 0"  << std::endl
     << "4 4 5 1 0"  << std::endl
     << "4 5 4 6 7"  << std::endl
     << "4 6 2 3 7"  << std::endl
     << "4 3 1 5 7"  << std::endl;
  Nef_polyhedron* mycube = new Nef_polyhedron;
  CGAL::OFF_to_nef_3(ss, *mycube);
  Transformation scale(CGAL::SCALING, r);
  mycube->transform(scale);
  return mycube;
}


Nef_polyhedron* dilate_nef_polyhedron(Nef_polyhedron* nef, float r)
{
  Nef_polyhedron* output = new Nef_polyhedron;
  Nef_polyhedron* cube = get_structuring_element_cube(r);
  *output = CGAL::minkowski_sum_3(*nef, *cube);
  return output;
}


Nef_polyhedron* erode_nef_polyhedron(Nef_polyhedron* nef, float r)
{
  Nef_polyhedron* output = new Nef_polyhedron;
  Nef_polyhedron* se = get_structuring_element_cube(r);
  // Nef_polyhedron* se = get_structuring_element_dodecahedron(r);
  Nef_polyhedron* bbox = get_aabb(nef);
  Nef_polyhedron complement = *bbox - *nef;
  Nef_polyhedron tmp = CGAL::minkowski_sum_3(complement, *se);
  *output = *nef - tmp;
  output->regularization();
  // std::cout << "#volume " << output->number_of_volumes() << std::endl;
  // Nef_polyhedron::Vertex_const_iterator v;
  // for (v = output->vertices_begin(); v != output->vertices_end(); v++)
  //   std::cout << v->point() << std::endl;
  return output;
}


Nef_polyhedron* get_aabb(Nef_polyhedron* mynef) 
{
  double xmin =  1e12; 
  double ymin =  1e12; 
  double zmin =  1e12; 
  double xmax = -1e12;
  double ymax = -1e12;
  double zmax = -1e12;;
  Nef_polyhedron::Vertex_const_iterator v;
  for (v = mynef->vertices_begin(); v != mynef->vertices_end(); v++) 
  {
    if ( CGAL::to_double(v->point().x()) < xmin )
      xmin = CGAL::to_double(v->point().x());
    if ( CGAL::to_double(v->point().y()) < ymin )
      ymin = CGAL::to_double(v->point().y());
    if ( CGAL::to_double(v->point().z()) < zmin )
      zmin = CGAL::to_double(v->point().z());
    if ( CGAL::to_double(v->point().x()) > xmax )
      xmax = CGAL::to_double(v->point().x());
    if ( CGAL::to_double(v->point().y()) > ymax )
      ymax = CGAL::to_double(v->point().y());
    if ( CGAL::to_double(v->point().z()) > zmax )
      zmax = CGAL::to_double(v->point().z());
  }
  //-- expand the bbox by 10units
  xmin -= 10;
  ymin -= 10;
  zmin -= 10;
  xmax += 10;
  ymax += 10;
  zmax += 10;
  //-- write an OFF file and convert Nef, simplest (and fastest?) solution
  std::stringstream ss;
  ss << "OFF" << std::endl
     << "8 6 0" << std::endl
     << xmin << " " << ymin << " " << zmin << std::endl
     << xmax << " " << ymin << " " << zmin << std::endl
     << xmax << " " << ymax << " " << zmin << std::endl
     << xmin << " " << ymax << " " << zmin << std::endl
     << xmin << " " << ymin << " " << zmax << std::endl
     << xmax << " " << ymin << " " << zmax << std::endl
     << xmax << " " << ymax << " " << zmax << std::endl
     << xmin << " " << ymax << " " << zmax << std::endl
     << "4 0 3 2 1" << std::endl
     << "4 0 1 5 4"    << std::endl
     << "4 1 2 6 5"    << std::endl
     << "4 2 3 7 6"    << std::endl
     << "4 0 4 7 3"    << std::endl
     << "4 4 5 6 7"    << std::endl;
  Nef_polyhedron* nefbbox = new Nef_polyhedron;
  CGAL::OFF_to_nef_3(ss, *nefbbox);
  return nefbbox;
}


void get_best_fitted_plane(const std::vector< Point3 > &lsPts, CgalPolyhedron::Plane_3 &plane)
{
  linear_least_squares_fitting_3(lsPts.begin(), lsPts.end(), plane, CGAL::Dimension_tag<0>());  
}


bool is_face_planar_distance2plane(const std::vector<Point3> &pts, const CgalPolyhedron::Plane_3 &plane, double& value, float tolerance)
{
  if (pts.size() == 3) {
    return true;
  }
  //-- test distance to that plane for each point
  std::vector<Point3>::const_iterator it = pts.begin();
  bool isPlanar = true;
  for ( ; it != pts.end(); it++)
  {
    K::FT d2 = CGAL::squared_distance(*it, plane);
    if ( CGAL::to_double(d2) > (tolerance*tolerance) )
    {
      value = sqrt(CGAL::to_double(d2));
      isPlanar = false;
      break;
    }
  }
  return isPlanar;
}


bool cmpPoint3(Point3 &p1, Point3 &p2, double tol)
{
  if ( (p1 == p2) || (CGAL::squared_distance(p1, p2) <= (tol * tol)) )
    return true;
  else
    return false;
}


void create_cgal_polygon(const std::vector<Point3>& lsPts, const std::vector<int>& ids, const CgalPolyhedron::Plane_3 &plane, Polygon &outpgn)
{
  // int proj = projection_plane(lsPts, ids);
  std::vector<int>::const_iterator it = ids.begin();
  for ( ; it != ids.end(); it++)
  {
    Point3 p = lsPts[*it];
    outpgn.push_back(plane.to_2d(p));
  }
}


bool is_face_planar_normals(const std::vector<int*> &trs, const std::vector<Point3>& lsPts, double& value, float angleTolerance)
{
  std::vector<int*>::const_iterator ittr = trs.begin();
  int* a = *ittr;
  Vector v0 = unit_normal( lsPts[a[0]], lsPts[a[1]], lsPts[a[2]]);
  ittr++;
  bool isPlanar = true;
  for ( ; ittr != trs.end(); ittr++)
  {
    a = *ittr;
    Vector v1 = unit_normal( lsPts[a[0]], lsPts[a[1]], lsPts[a[2]] );
    Vector a = CGAL::cross_product(v0, v1);
    K::FT norm = sqrt(a.squared_length());
    double dot = CGAL::to_double((v0*v1));
    double angle = atan2(CGAL::to_double(norm), dot);
    if (angle*180/PI > angleTolerance)
    {
      // cout << "\t---angle: " << angle*180/PI << endl;
      value = angle*180/PI;
      isPlanar = false;
      break;
    }
  }
  return isPlanar;
}

} // namespace val3dity