#include <CGAL/Cartesian.h>
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>

#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <cmath>

//	IN ORDER TO USE, EXTRACT json_hpp.7z OR download it form
//	https://github.com/nlohmann/json

#include "json.hpp"
using json = nlohmann::json;

typedef CGAL::Quotient<CGAL::MP_Float> Number_type;
typedef CGAL::Cartesian<Number_type> Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;
typedef Traits_2::Point_2 Point_2;
typedef Traits_2::X_monotone_curve_2 Segment_2;
typedef CGAL::Arrangement_2<Traits_2> Arrangement_2;


void print_ccb (Arrangement_2::Ccb_halfedge_const_circulator circ, std::ofstream& file)
{
  Arrangement_2::Ccb_halfedge_const_circulator curr = circ;
  float x, y;
  do {
    Arrangement_2::Halfedge_const_handle he = curr;
    x = CGAL::to_double(curr->target()->point().x());
    y = CGAL::to_double(curr->target()->point().y());
    file << x << " " << y << "  ";

  } while (++curr != circ);
  file << std::endl;
}

void print_arrangement (const Arrangement_2& arr, std::ofstream& file)
{
  file << arr.number_of_faces() << std::endl;
  Arrangement_2::Face_const_iterator fit;
  for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit)
  {
    if(!fit->is_unbounded())
    {
      print_ccb (fit->outer_ccb(), file);
    }
  }
}

json get_node_coords(json& graph, std::string id)
{
  for(auto& it : graph["nodes"]){ if( it["id"] == id){ return it["coords"]; } }
  return json("not found");
}

void append_edge_info(json& graph)
{
  for(auto& it: graph["edges"])
  {
    it["cord_from"] = get_node_coords(graph, it["from"]);
    it["cord_to"] = get_node_coords(graph, it["to"]);
  }
}

int find_node_index(json& graph, std::string id)
{
  int i = 0;
  for(auto& it : graph["nodes"])
  {
    if(it["id"] == id){ return i; }
    i++;
  }
  return -1;
}

//extern "C" {
int divide(int CIRCLE_RESOLUTION, float DISASTER_R, char* ipath, char* opath)
{
  std::ifstream file(ipath);
  std::ofstream fileo;
  fileo.open(opath, std::ios_base::app);

  json graph;
  file >> graph;

  append_edge_info(graph);

  int node_count = graph["nodes"].size();

  Point_2 circles[node_count][CIRCLE_RESOLUTION];

  for(int j = 0; j < node_count; j++)
  {
    float x = (float)graph["nodes"][j]["coords"][0];
    float y = (float)graph["nodes"][j]["coords"][1];
    for(int i = 0; i < CIRCLE_RESOLUTION; i++)
    {
      float a = i *( 360 / CIRCLE_RESOLUTION)*  M_PI / 180;
      circles[j][i] = Point_2( x + DISASTER_R * cosf( a ), y + DISASTER_R * sinf( a ));
    }
  }

  int edge_count = graph["edges"].size();
  int diff = CIRCLE_RESOLUTION / 4;

  Point_2 lines[edge_count][2][2];   //edges, Upper-Lower, Start-End,

  for(int i = 0; i < edge_count; i++)
  {
    float x1 = graph["edges"][i]["cord_from"][0];
    float y1 = graph["edges"][i]["cord_from"][1];
    float x2 = graph["edges"][i]["cord_to"][0];
    float y2 = graph["edges"][i]["cord_to"][1];

    float theta = atan2f( (y2-y1), (x2-x1) );
    if( theta < 0 ){ theta = 2 * M_PI + theta; }

    int cross_index = static_cast<int>(roundf( theta * 180 / M_PI / 360 * CIRCLE_RESOLUTION ));

    int upperindex = (cross_index + diff) % CIRCLE_RESOLUTION;
    int lowerindex = (cross_index - diff) % CIRCLE_RESOLUTION;
    if( lowerindex < 0 ){ lowerindex += CIRCLE_RESOLUTION; }

    int start_node_index = find_node_index(graph, graph["edges"][i]["from"]);
    int end_node_index = find_node_index(graph, graph["edges"][i]["to"]);

    lines[i][0][0] = circles[start_node_index][upperindex]; //upper start
    lines[i][0][1] = circles[end_node_index][upperindex]; //upper end
    lines[i][1][0] = circles[start_node_index][lowerindex]; //lower start
    lines[i][1][1] = circles[end_node_index][lowerindex]; //lower end
  }

  Arrangement_2 arr;

  for(int i = 0; i < node_count; i++)
  {
    int ci = 0;
    Segment_2 cv[CIRCLE_RESOLUTION];
    Point_2 p0 = circles[i][0];
    Point_2 p1 = p0;
    for(int j = 1; j < CIRCLE_RESOLUTION; j++)
    {
      Point_2 p2 = circles[i][j];
      cv[ci++] = Segment_2( p1, p2 );
      p1 = p2;
    }
    cv[ci] = Segment_2( p0, p1 );
    CGAL::insert (arr, &cv[0], &cv[CIRCLE_RESOLUTION]);
  }

  for( int i = 0; i < edge_count; i++) // every edge
  {
    for(int j = 0; j < 2; j++)  //upper - lower
    {
      Point_2 ps = lines[i][j][0];
      Point_2 pe = lines[i][j][1];
      Segment_2 s(ps, pe);
      CGAL::insert(arr, s);
    }
  }
  Arrangement_2 arr2;
  print_arrangement(arr, fileo);
  fileo.close();
  return 0;
}

int main(int argc, char **argv)
{
  return divide(atoi(argv[1]), atof(argv[2]), argv[3], argv[4]);
}



