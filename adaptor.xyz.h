#pragma once
/*
MIT License

Copyright (c) 2021 Attila Csik�s

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "octree.h"


namespace BasicTypesXYZ // Replaceble with simirarly build geometry types
{
  using float_t = float;

  struct Point2D
  {
    float_t x;
    float_t y;
  };

  struct Point3D
  {
    float_t x;
    float_t y;
    float_t z;
  };

  struct BoundingBox2D
  {
    Point2D Min;
    Point2D Max;
  };

  struct BoundingBox3D
  {
    Point3D Min;
    Point3D Max;
  };
} // namespace


namespace OrthoTree
{
  namespace XYAdaptor2D
  {
    using xy_geometry_type = BasicTypesXYZ::float_t;
    using XYPoint2D = BasicTypesXYZ::Point2D;
    using XYBoundingBox2D = BasicTypesXYZ::BoundingBox2D;

    struct XYAdaptorBasics
    {
      static constexpr xy_geometry_type& point_comp(XYPoint2D& pt, dim_type dimensionID)
      {
        switch (dimensionID)
        {
        case 0: return pt.x;
        case 1: return pt.y;
        default: std::terminate();
        }
      }

      static constexpr xy_geometry_type point_comp_c(XYPoint2D const& pt, dim_type dimensionID)
      {
        switch (dimensionID)
        {
        case 0: return pt.x;
        case 1: return pt.y;
        default: std::terminate();
        }
      }

      static constexpr XYPoint2D& box_min(XYBoundingBox2D& box) { return box.Min; }
      static constexpr XYPoint2D& box_max(XYBoundingBox2D& box) { return box.Max; }
      static constexpr XYPoint2D const& box_min_c(XYBoundingBox2D const& box) { return box.Min; }
      static constexpr XYPoint2D const& box_max_c(XYBoundingBox2D const& box) { return box.Max; }
    };

    using XYAdaptorGeneral = AdaptorGeneralBase<2, XYPoint2D, XYBoundingBox2D, XYAdaptorBasics, xy_geometry_type>;
  } // namespace XYAdaptor2D


  namespace XYZAdaptor3D
  {
    using xyz_geometry_type = BasicTypesXYZ::float_t;
    using XYZPoint3D = BasicTypesXYZ::Point3D;
    using XYZBoundingBox3D = BasicTypesXYZ::BoundingBox3D;

    struct XYZAdaptorBasics
    {
      static constexpr xyz_geometry_type& point_comp(XYZPoint3D& pt, dim_type dimensionID)
      {
        switch (dimensionID)
        {
        case 0: return pt.x;
        case 1: return pt.y;
        case 2: return pt.z;
        default: std::terminate();
        }
      }

      static constexpr xyz_geometry_type point_comp_c(XYZPoint3D const& pt, dim_type dimensionID)
      {
        switch (dimensionID)
        {
        case 0: return pt.x;
        case 1: return pt.y;
        case 2: return pt.z;
        default: std::terminate();
        }
      }

      static constexpr XYZPoint3D& box_min(XYZBoundingBox3D& box) { return box.Min; }
      static constexpr XYZPoint3D& box_max(XYZBoundingBox3D& box) { return box.Max; }
      static constexpr XYZPoint3D const& box_min_c(XYZBoundingBox3D const& box) { return box.Min; }
      static constexpr XYZPoint3D const& box_max_c(XYZBoundingBox3D const& box) { return box.Max; }
    };

    using XYZAdaptorGeneral = AdaptorGeneralBase<3, XYZPoint3D, XYZBoundingBox3D, XYZAdaptorBasics, xyz_geometry_type>;


  } // namespace XYAdaptor2D
}

namespace XYZ
{
  using namespace OrthoTree;
  using namespace OrthoTree::XYAdaptor2D;
  using namespace OrthoTree::XYZAdaptor3D;

  using QuadtreePoint = OrthoTreePoint<2, XYPoint2D, XYBoundingBox2D, XYAdaptorGeneral, xy_geometry_type>;

  template<uint32_t t_AdditionalDepthOfSplitStrategy = 2>
  using QuadtreeBox = OrthoTreeBoundingBox<2, XYPoint2D, XYBoundingBox2D, XYAdaptorGeneral, xy_geometry_type, t_AdditionalDepthOfSplitStrategy>;

  using QuadtreePointC = OrthoTreeContainerPoint<QuadtreePoint, XYPoint2D>;

  template<uint32_t t_AdditionalDepthOfSplitStrategy = 2>
  using QuadtreeBoxC = OrthoTreeContainerBox<QuadtreeBox<t_AdditionalDepthOfSplitStrategy>, XYBoundingBox2D>;

  
  using OctreePoint = OrthoTreePoint<3, XYZPoint3D, XYZBoundingBox3D, XYZAdaptorGeneral, xyz_geometry_type>;
  
  template<uint32_t t_AdditionalDepthOfSplitStrategy = 2>
  using OctreeBox =
    OrthoTreeBoundingBox<3, XYZPoint3D, XYZBoundingBox3D, XYZAdaptorGeneral, xyz_geometry_type, t_AdditionalDepthOfSplitStrategy>;
  

  using OcreePointC = OrthoTreeContainerPoint<OctreePoint, XYZPoint3D>;

  template<uint32_t t_AdditionalDepthOfSplitStrategy = 2>
  using OctreeBoxC = OrthoTreeContainerBox<QuadtreeBox<t_AdditionalDepthOfSplitStrategy>, XYZBoundingBox3D>;
}