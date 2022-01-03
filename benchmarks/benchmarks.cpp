// benchmarks.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define autoc auto const
#define autoce auto constexpr

#include <iostream>
#include <fstream>
#include <chrono>
#include <execution>
#include <span>
#include <functional>
#include <numbers>
#include <vector>
#include <random>

#include "../octree.h"
#include "OrthoTreePointDynamicGeneral.h"


using namespace std::chrono;
using namespace std;

using namespace OrthoTree;


namespace
{
  autoce N = 3;

  autoce szSeparator = "; ";
  autoce szNewLine = "\n";

  autoce rMax = 8.0;

#ifdef _DEBUG
  autoce N1M = 100000;
  autoce NR = 1;
#else
  autoce N1M = 1000000;
  autoce NR = 100;
#endif // _DEBUG

  auto boxMax = BoundingBoxND<N>{};

  using time_unit = milliseconds;

  using Adaptor = AdaptorGeneral<N, PointND<N>, BoundingBoxND<N>>;
  autoce degree_to_rad(double degree) { return degree / 180.0 * std::numbers::pi; }


  template<size_t nDim>
  static constexpr PointND<nDim> CreateBoxMax(PointND<nDim> const& pt, double size)
  {
    auto ptMax = pt;
    for (size_t iDim = 0; iDim < nDim; ++iDim)
      Adaptor::point_comp(ptMax, static_cast<dim_type>(iDim)) += size;

    return ptMax;
  }


  template<dim_type nDim>
  static BoundingBoxND<nDim> CreateSearcBox(double rBegin, double rSize)
  {
    auto box = BoundingBoxND<nDim>{};
    for (dim_type iDim = 0; iDim < nDim; ++iDim)
      box.Min[iDim] = rBegin;

    box.Max = CreateBoxMax<nDim>(box.Min, rSize);
    return box;
  }


  template<dim_type nDim, size_t nNumber>
  static vector<PointND<nDim>> CreatePoints_Diagonal()
  {
    auto aPoint = vector<PointND<nDim>>(nNumber);
    if (nNumber <= 1)
      return aPoint;

    size_t iNumber = 1;

    auto ptMax = PointND<nDim>{};
    for (dim_type iDim = 0; iDim < nDim; ++iDim)
      ptMax[iDim] = rMax;

    // Corner points
    {
      aPoint[1] = ptMax;
      ++iNumber;

      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
        aPoint[iNumber][iDim] = rMax;
    }

    autoc nNumberPre = iNumber;
    // Angle points
    {
      autoc nRemain = nNumber - iNumber;
      autoc rStep = rMax / static_cast<double>(nRemain + 2);
      for (size_t iRemain = 1; iNumber < nNumber; ++iNumber, ++iRemain)
        for (dim_type iDim = 0; iDim < nDim; ++iDim)
          aPoint[iNumber][iDim] = rMax - iRemain * rStep;

    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(next(begin(aPoint), nNumberPre), end(aPoint), g);

    autoc box = Adaptor::box_of_points(aPoint);
    assert(Adaptor::are_points_equal(box.Max, ptMax, 0.0001));
    assert(Adaptor::are_points_equal(box.Min, PointND<nDim>{}, 0.0001));

    return aPoint;
  }

  template<dim_type nDim, size_t nNumber>
  static vector<PointND<nDim>> CreatePoints_Random()
  {
    auto aPoint = vector<PointND<nDim>>(nNumber);
    if (nNumber <= 1)
      return aPoint;

    size_t iNumber = 1;

    auto ptMax = PointND<nDim>{};
    for (dim_type iDim = 0; iDim < nDim; ++iDim)
      ptMax[iDim] = rMax;


    // Corner points
    {
      aPoint[1] = ptMax;
      ++iNumber;

      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
        aPoint[iNumber][iDim] = rMax;
    }

    srand(0);
    {
      for (; iNumber < nNumber; ++iNumber)
        for (dim_type iDim = 0; iDim < nDim; ++iDim)
          aPoint[iNumber][iDim] = (rand() % 100) * (rMax / 100.0);

    }

    autoc box = Adaptor::box_of_points(aPoint);
    assert(Adaptor::are_points_equal(box.Max, ptMax, 0.0001));
    assert(Adaptor::are_points_equal(box.Min, PointND<nDim>{}, 0.0001));

    return aPoint;
  }

  template<dim_type nDim, size_t nNumber>
  static vector<PointND<nDim>> CreatePoints_CylindricalSemiRandom()
  {
    auto aPoint = vector<PointND<nDim>>(nNumber);
    if (nNumber <= 1)
      return aPoint;

    size_t iNumber = 1;

    auto ptMax = PointND<nDim>{};
    for (dim_type iDim = 0; iDim < nDim; ++iDim)
      ptMax[iDim] = rMax;

    // Corner points
    {
      aPoint[1] = ptMax;
      ++iNumber;

      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
        aPoint[iNumber][iDim] = rMax;
    }

    srand(0);
    {
      for (; iNumber < nNumber; ++iNumber)
      {
        autoc rAngle = degree_to_rad(static_cast<double>(rand() % 360));
        autoc rRadius = rMax / 4.0 + (rand() % 100) * 0.01;
        aPoint[iNumber][0] = cos(rAngle) * rRadius + rMax / 2.0;
        aPoint[iNumber][1] = sin(rAngle) * rRadius + rMax / 2.0;

        for (dim_type iDim = 2; iDim < nDim; ++iDim)
          aPoint[iNumber][iDim] = (rand() % 100) * rMax / 100.0;
      }
    }

    autoc box = Adaptor::box_of_points(aPoint);
    assert(Adaptor::are_points_equal(box.Max, ptMax, 0.0001));
    assert(Adaptor::are_points_equal(box.Min, PointND<nDim>{}, 0.0001));

    return aPoint;
  }



  template<dim_type nDim, size_t nNumber>
  static vector<BoundingBoxND<nDim>> CreateBoxes_Diagonal()
  {
    if (nNumber == 0)
      return {};

    autoce rMax = 8.0;
    autoce rUnit = 1.0;
    auto aBox = vector<BoundingBoxND<nDim>>(nNumber);
    aBox[0].Max = CreateBoxMax(PointND<nDim>(), rMax);
    if (nNumber == 1)
      return aBox;

    size_t iNumber = 1;

    // Corner points
    {
      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
      {
        aBox[iNumber].Min[iDim] = rMax - rUnit;
        aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);
      }
      if (iNumber == nNumber)
        return aBox;


      for (dim_type iDim = 0; iDim < nDim; ++iDim)
        aBox[iNumber].Min[iDim] = rMax - rUnit;

      aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);

      ++iNumber;
    }

    // Angle points
    {
      autoc nRemain = nNumber - iNumber;
      autoc rStep = (rMax - rUnit) / (nRemain + 2);
      for (size_t iRemain = 1; iNumber < nNumber; ++iNumber, ++iRemain)
      {
        for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim)
          aBox[iNumber].Min[iDim] = rMax - rUnit - iRemain * rStep;

        aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rStep);
      }
    }

    return aBox;
  }

  template<dim_type nDim, size_t nNumber>
  static vector<BoundingBoxND<nDim>> CreateBoxes_Random()
  {
    if (nNumber == 0)
      return {};

    autoce rMax = 8.0;
    autoce rUnit = 1.0;
    auto aBox = vector<BoundingBoxND<nDim>>(nNumber);
    aBox[0].Max = CreateBoxMax(PointND<nDim>(), rMax);
    if (nNumber == 1)
      return aBox;

    size_t iNumber = 1;

    // Corner points
    {
      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
      {
        aBox[iNumber].Min[iDim] = rMax - rUnit;
        aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);
      }
      if (iNumber == nNumber)
        return aBox;


      for (dim_type iDim = 0; iDim < nDim; ++iDim)
        aBox[iNumber].Min[iDim] = rMax - rUnit;

      aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);

      ++iNumber;
    }

    srand(0);

    {
      for (size_t iRemain = 1; iNumber < nNumber; ++iNumber, ++iRemain)
      {
        autoc iNumberBox = nNumber - iNumber - 1;
        for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim)
          aBox[iNumberBox].Min[iDim] = (rand() % 100) * ((rMax - 1.0) / 100.0);

        aBox[iNumberBox].Max = CreateBoxMax(aBox[iNumberBox].Min, (rand() % 100) * (1.0 * rUnit / 100.0));
      }
    }

    return aBox;
  }

  template<dim_type nDim, size_t nNumber>
  static vector<BoundingBoxND<nDim>> CreateBoxes_CylindricalSemiRandom()
  {
    if (nNumber == 0)
      return {};

    autoce rUnit = 1.0;
    auto aBox = vector<BoundingBoxND<nDim>>(nNumber);
    aBox[0].Max = CreateBoxMax(PointND<nDim>(), rMax);
    if (nNumber == 1)
      return aBox;

    size_t iNumber = 1;

    // Corner points
    {
      for (dim_type iDim = 0; iDim < nDim && iNumber < nNumber; ++iDim, ++iNumber)
      {
        aBox[iNumber].Min[iDim] = rMax - rUnit;
        aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);
      }
      if (iNumber == nNumber)
        return aBox;


      for (dim_type iDim = 0; iDim < nDim; ++iDim)
        aBox[iNumber].Min[iDim] = rMax - rUnit;

      aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rUnit);

      ++iNumber;
    }

    srand(0);

    {
      for (; iNumber < nNumber; ++iNumber)
      {
        autoc rAngle = degree_to_rad(static_cast<double>(rand() % 180) * 2);
        autoc rRadius = 3.0 + (rand() % 100) * 0.01;
        autoc rSize = 0.0005 + static_cast<double>(rand() % 1000) * 0.0005;
        aBox[iNumber].Min[0] = cos(rAngle) * rRadius + rMax / 2.0 - rSize / 2.0;
        aBox[iNumber].Min[1] = sin(rAngle) * rRadius + rMax / 2.0 - rSize / 2.0;

        for (dim_type iDim = 2; iDim < nDim; ++iDim)
          aBox[iNumber].Min[iDim] = (rand() % 100) * 0.01 * (rMax - 2.0 * rUnit) + rUnit - rSize / 2.0;

        aBox[iNumber].Max = CreateBoxMax(aBox[iNumber].Min, rSize);

      }
    }

    return aBox;
  }

  template <typename vector_type, typename box_type>
  vector<vector<OrthoTree::entity_id_type>> RangeSearchNaive(span<box_type const> const& vSearchBox, span<box_type const> const& vBox)
  {
    autoc n = vBox.size();
    auto vElementFound = vector<vector<OrthoTree::entity_id_type>>();
    vElementFound.reserve(n);
    for (autoc& boxSearch : vSearchBox)
    {
      auto& vElementByBox = vElementFound.emplace_back();
      vElementByBox.reserve(50);

      for (size_t i = 0; i < n; ++i)
        if (AdaptorGeneral<N, vector_type, box_type>::are_boxes_overlapped(boxSearch, vBox[i], false))
          vElementByBox.emplace_back(i);
    }
    return vElementFound;
  }

  template <typename vector_type, typename box_type, typename execution_policy_type = std::execution::unsequenced_policy>
  vector<std::pair<entity_id_type, entity_id_type>> SelfConflicthNaive(span<box_type const> const& vBox)
  {
    autoc nEntity = vBox.size();

    auto vidCheck = vector<entity_id_type>(nEntity);
    std::iota(std::begin(vidCheck), std::end(vidCheck), 0);

    auto vvidCollision = vector<vector<entity_id_type>>(vidCheck.size());
    std::transform(execution_policy_type{}, std::begin(vidCheck), std::end(vidCheck), std::begin(vvidCollision), [&](autoc idCheck) -> vector<entity_id_type>
    {
      auto sidFound = vector<entity_id_type>();
      for (size_t i = idCheck + 1; i < nEntity; ++i)
        if (AdaptorGeneral<N, vector_type, box_type>::are_boxes_overlapped(vBox[idCheck], vBox[i], false))
          sidFound.emplace_back(i);

      return sidFound;
    });

    auto vPair = vector<std::pair<entity_id_type, entity_id_type>>{};
    if (nEntity > 10)
      vPair.reserve(nEntity / 10);

    for (autoc idCheck : vidCheck)
      for (autoc idCollide : vvidCollision[idCheck])
        vPair.emplace_back(idCheck, idCollide);

    return vPair;
  }


  template <typename vector_type, typename box_type>
  vector<vector<OrthoTree::entity_id_type>> RangeSearchNaive(span<box_type const> const& vSearchBox, span<vector_type const> const& vPoint)
  {
    autoc n = vPoint.size();
    auto vElementFound = vector<vector<OrthoTree::entity_id_type>>();
    vElementFound.reserve(n);
    for (autoc& boxSearch : vSearchBox)
    {
      auto& vElementByBox = vElementFound.emplace_back();
      vElementByBox.reserve(50);

      for (size_t i = 0; i < n; ++i)
        if (AdaptorGeneral<N, vector_type, box_type>::does_box_contain_point(boxSearch, vPoint[i]))
          vElementByBox.emplace_back(i);
    }
    return vElementFound;
  }


  template<size_t nDim>
  static size_t TreePointCreate(size_t depth, std::span<PointND<nDim> const> const& aPoint, bool fPar = false)
  {
    auto box = BoundingBoxND<nDim>{};
    box.Max.fill(rMax);

    auto nt = fPar
      ? TreePointND<nDim>::template Create<std::execution::parallel_unsequenced_policy>(aPoint, depth, box)
      : TreePointND<nDim>::Create(aPoint, depth, box)
      ;

    return nt.GetNodes().size();
  }

  template<size_t nDim>
  static size_t TreePointNaiveCreate(size_t depth, std::span<PointND<nDim> const> const& aPoint, bool)
  {
    auto box = BoundingBoxND<nDim>{};
    box.Max.fill(rMax);

    auto nt = OrthoTreePointDynamicND<nDim>::Create(aPoint, depth, box, 11);
    return nt.GetNodeSize();
  }


  template<size_t nDim>
  static size_t TreeBoxCreate(size_t depth, std::span<BoundingBoxND<nDim> const> const& aBox, bool fPar = false)
  {
    auto box = BoundingBoxND<nDim>{};
    box.Max.fill(rMax);

    auto nt = fPar
      ? TreeBoxND<nDim>::template Create<std::execution::parallel_unsequenced_policy>(aBox, depth, box)
      : TreeBoxND<nDim>::Create(aBox, depth, box)
      ;

    return nt.GetNodes().size();
  }

  template<size_t nDim>
  static size_t TreeBoxDynCreate(size_t depth, std::span<BoundingBoxND<nDim> const> const& aBox, bool)
  {
    auto box = BoundingBoxND<nDim>{};
    box.Max.fill(rMax);

    auto nt = OrthoTreeBoxDynamicND<nDim>::Create(aBox, depth, box, 11);
    return nt.GetNodeSize();
  }
  

  void display_time(microseconds const& time)
  {
    if (time.count() < 10000)
      std::cout << time;
    else
      std::cout << duration_cast<milliseconds>(time);
  }

  inline double microseconds_to_double(microseconds const& time)
  {
    return static_cast<double>(time.count()) / 1000.0;
  }

  template<typename entity_type>
  struct MeasurementTask
  {
    string szDisplay;
    int nDataSize;
    int nRepeat = 10;
    size_t nDepth = 5;
    bool fParallel = false;
    span<entity_type const> sEntity;
    function<size_t(size_t, span<entity_type const>, bool)> func;

    size_t Run() const { return func(nDepth, sEntity, fParallel); }
  };


  template<size_t N, typename product_type>
  product_type GenerateGeometry(std::function<product_type(void)> fn, string const& szName, double M, ofstream& report)
  {
    std::cout << "Generate " << szName << "...";

    autoc t0 = high_resolution_clock::now();
    product_type const product = fn();
    autoc t1 = high_resolution_clock::now();
    autoc tDur = duration_cast<microseconds>(t1 - t0);

    std::cout << " Finished. ";
    display_time(tDur);
    std::cout << "\n";

    report
      << szName << szSeparator
      << 1 << szSeparator
      << string("seq") << szSeparator
      << M * N1M << szSeparator
      << microseconds_to_double(tDur) << szSeparator
      << szNewLine
      ;

    return product;
  }


  autoce aSizeNonLog = array{ 100, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000, 10000 };
  //autoce aSizeNonLog = array{ 10000 };

  autoce nSizeNonLog = aSizeNonLog.size();
  autoce aRepeatNonLog = array{ 100 * NR, 10 * NR, 10 * NR, 10 * NR, 10 * NR, 10 * NR, 1 * NR, 1 * NR, 1 * NR, 1 * NR, 1 * NR, 1 * NR, 1 * NR, 1 * NR };
  //autoce aRepeatNonLog = array{ 1 * NR };

  static_assert(nSizeNonLog == aRepeatNonLog.size());

  autoce aSizeLog = array{ 10, 50, 100, 1000, 2500, 5000, 7500, 10000, 100000, N1M, 10 * N1M, 100 * N1M };
  autoce nSizeLog = aSizeLog.size();

  autoce aRepeatLog = array{ 100000, 100000, 10000, 2000, 1000, 500, 500, 100, 100, 10, 10, 5 };
  static_assert(nSizeLog == aRepeatLog.size());


  template<size_t N>
  vector<MeasurementTask<PointND<N>>> GeneratePointTasks(size_t nDepth, string const& szName, span<PointND<N> const> const& sPoint)
  {
    auto vTask = vector<MeasurementTask<PointND<N>>>();
    for (autoc fPar : { false, true })
      for (size_t iSize = 0; iSize < nSizeLog; ++iSize)
        vTask.push_back(MeasurementTask<PointND<N>>{ szName, aSizeLog[iSize], aRepeatLog[iSize], nDepth, fPar, sPoint.subspan(0, aSizeLog[iSize]), TreePointCreate<N> });

    return vTask;
  }

  template<size_t N>
  vector<MeasurementTask<PointND<N>>> GeneratePointTasks_NonLog(size_t nDepth, string const& szName, span<PointND<N> const> const& sPoint)
  {
    auto vTask = vector<MeasurementTask<PointND<N>>>();
    for (size_t iSize = 0; iSize < nSizeNonLog; ++iSize)
      vTask.push_back(MeasurementTask<PointND<N>>{ szName, aSizeNonLog[iSize], aRepeatNonLog[iSize], nDepth, false, sPoint.subspan(0, aSizeNonLog[iSize]), TreePointCreate<N> });

    return vTask;
  }


  template<size_t N>
  vector<MeasurementTask<PointND<N>>> GeneratePointDynTasks_NonLog(size_t nDepth, string const& szName, span<PointND<N> const> const& sPoint)
  {
    auto vTask = vector<MeasurementTask<PointND<N>>>();
    for (size_t iSize = 0; iSize < nSizeLog; ++iSize)
      vTask.push_back(MeasurementTask<PointND<N>>{ szName, aSizeLog[iSize], aRepeatLog[iSize], nDepth, false, sPoint.subspan(0, aSizeLog[iSize]), TreePointNaiveCreate<N> });

    return vTask;
  }

  template<size_t N>
  vector<MeasurementTask<BoundingBoxND<N>>> GenerateBoxDynTasks_NonLog(size_t nDepth, string const& szName, span<BoundingBoxND<N> const> const& sBox)
  {
    auto vTask = vector<MeasurementTask<BoundingBoxND<N>>>();
    for (size_t iSize = 0; iSize < nSizeLog; ++iSize)
      vTask.push_back(MeasurementTask<BoundingBoxND<N>>{ szName, aSizeLog[iSize], aRepeatLog[iSize], nDepth, false, sBox.subspan(0, aSizeLog[iSize]), TreeBoxDynCreate<N> });

    return vTask;
  }


  template<size_t N>
  vector<MeasurementTask<BoundingBoxND<N>>> SearchTreeBoxTasks(size_t nDepth, string const& szName, span<BoundingBoxND<N> const> const& aBox_)
  {
    auto vTask = vector<MeasurementTask<BoundingBoxND<N>>>();
    for (autoc fPar : { false, true })
      for (size_t iSize = 0; iSize < nSizeNonLog; ++iSize)
      {
        auto nDepthActual = nDepth;
        if (nDepth == -1) // Adapt to the size
        {
          if (aSizeNonLog[iSize] <= 100)
            nDepthActual = 3;
          else if (aSizeNonLog[iSize] <= 500)
            nDepthActual = 3;
          else if (aSizeNonLog[iSize] <= 1000)
            nDepthActual = 4;
          else if (aSizeNonLog[iSize] <= 10000)
            nDepthActual = 5;
          else 
            nDepthActual = 6;
        }
        vTask.push_back(MeasurementTask<BoundingBoxND<N>>{ szName, aSizeNonLog[iSize], aRepeatNonLog[iSize], nDepthActual, fPar, aBox_.subspan(0, aSizeNonLog[iSize]), [](size_t nDepth, span<BoundingBoxND<N> const> const& aBox, bool fPar)
        {
          if (fPar)
          {
            autoc nt = TreeBoxND<N>::template Create<std::execution::parallel_unsequenced_policy>(aBox, nDepth, boxMax);
            autoc vPair = nt.template CollisionDetection<std::execution::parallel_unsequenced_policy>(aBox);
            return vPair.size();
          }
          else
          {
            autoc nt = TreeBoxND<N>::Create(aBox, nDepth, boxMax);
            autoc vPair = nt.CollisionDetection(aBox);
            return vPair.size();
            /*
            size_t nFound = 0;
            autoc n = aBox.size();
            for (size_t i = 0; i < n; ++i)
            {
              autoc vElem = nt.RangeSearch<false>(aBox[i], aBox);
              nFound += vElem.size();
              assert(vElem.size());
            }
            return nFound;
            */
          }          
        } 
        });
      }
    return vTask;
  }

  template<size_t N>
  vector<MeasurementTask<BoundingBoxND<N>>> SearchBruteForceBoxTasks(size_t nDepth, string const& szName, span<BoundingBoxND<N> const> const& aBox_)
  {
    auto vTask = vector<MeasurementTask<BoundingBoxND<N>>>();
    for (size_t iSize = 0; iSize < nSizeNonLog; ++iSize)
      vTask.push_back(MeasurementTask<BoundingBoxND<N>>
        { szName, aSizeNonLog[iSize], aRepeatNonLog[iSize], nDepth, false, aBox_.subspan(0, aSizeNonLog[iSize])
        , [](size_t nDepth, span<BoundingBoxND<N> const> const& aBox, bool fPar)
          {
            autoc vvElem = RangeSearchNaive<PointND<N>, BoundingBoxND<N>>(aBox, aBox);

            size_t nFound = 0;
            for (autoc& vElem : vvElem)
              nFound += vElem.size();

            return nFound;
          }
        }
      );

    return vTask;
  }

  template<size_t N>
  vector<MeasurementTask<BoundingBoxND<N>>> SelfConflictBruteForceBoxTasks(string const& szName, span<BoundingBoxND<N> const> const& aBox_)
  {
    auto vTask = vector<MeasurementTask<BoundingBoxND<N>>>();
    for (size_t iSize = 0; iSize < nSizeNonLog; ++iSize)
      vTask.push_back(MeasurementTask<BoundingBoxND<N>>
    { szName, aSizeNonLog[iSize], aRepeatNonLog[iSize], 0, false, aBox_.subspan(0, aSizeNonLog[iSize])
        , [](size_t nDepth, span<BoundingBoxND<N> const> const& aBox, bool fPar)
    {
      if (!fPar)
      {
        autoc vvElem = SelfConflicthNaive<PointND<N>, BoundingBoxND<N>>(aBox);
        return vvElem.size();
      }
      else
      {
        autoc vvElem = SelfConflicthNaive<PointND<N>, BoundingBoxND<N>, std::execution::parallel_unsequenced_policy>(aBox);
        return vvElem.size();
      }
    }
    }
    );

    return vTask;
  }


  template<size_t N>
  vector<MeasurementTask<BoundingBoxND<N>>> GenerateBoxTasks(size_t nDepth, string szName, span<BoundingBoxND<N> const> const& sBox)
  {
    auto vTask = vector<MeasurementTask<BoundingBoxND<N>>>();
    for (autoc fPar : { false, true })
      for (int iSize = 0; iSize < nSizeLog; ++iSize)
        vTask.push_back(MeasurementTask<BoundingBoxND<N>>{ szName, aSizeLog[iSize], aRepeatLog[iSize], nDepth, fPar, sBox.subspan(0, aSizeLog[iSize]), TreeBoxCreate<N> });

    return vTask;
  }


  tuple<microseconds, size_t> Measure(int nRepeat, function<size_t(void)> const& fn)
  {
    autoc t0 = high_resolution_clock::now();
    auto v = vector<size_t>(nRepeat);
    for (int i = 0; i < nRepeat; ++i)
      v[i] = fn();

    autoc t1 = high_resolution_clock::now();
    return { duration_cast<microseconds>((t1 - t0) / nRepeat), v[0] };
  }


  template<typename entity_type>
  void RunTasks(vector<MeasurementTask<entity_type>> const& vTask, ofstream& report)
  {
    for (autoc& task : vTask)
    {
      autoc szPar = (task.fParallel ? string("par") : string("unseq"));
      std::cout << "Create tree for: " << task.szDisplay << " " << task.nDataSize << " " << szPar << " Repeat: " << task.nRepeat << "...";
      autoc[tDur, nNode] = Measure(task.nRepeat, [&task]{ return task.Run(); });

      std::cout << " Finished. ";
      display_time(tDur);
      std::cout << szNewLine;

      report
        << task.szDisplay << szSeparator
        << task.nRepeat << szSeparator
        << szPar << szSeparator
        << task.nDataSize << szSeparator
        << microseconds_to_double(tDur) << szSeparator
        << nNode << szSeparator
        << szNewLine
        ;
    }
  }
}


int main()
{
  boxMax.Max.fill(rMax);

  ofstream report;
  report.open("report.csv");

  autoce nDepth = 5;
  {
    autoc szName = string("Diagonally placed points");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<PointND<N>>>([&] { return CreatePoints_Diagonal<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GeneratePointTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }

  {
    autoc szName = string("Random placed points");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<PointND<N>>>([&] { return CreatePoints_Random<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GeneratePointTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }

  {
    autoc szName = string("Cylindrical semi-random placed points");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<PointND<N>>>([&] { return CreatePoints_CylindricalSemiRandom<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GeneratePointTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }

  {
    autoc szName = string("Diagonally placed boxes");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<BoundingBoxND<N>>>([&] { return CreateBoxes_Diagonal<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GenerateBoxTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }

  {
    autoc szName = string("Random placed boxes");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<BoundingBoxND<N>>>([&] { return CreateBoxes_Random<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GenerateBoxTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }

  {
    autoc szName = string("Cylindrical semi-random placed boxes");
    autoc aPointDiag_100M = GenerateGeometry<N, vector<BoundingBoxND<N>>>([&] { return CreateBoxes_CylindricalSemiRandom<N, 100 * N1M>(); }, szName, 100, report);
    autoc vTask = GenerateBoxTasks<N>(nDepth, szName, aPointDiag_100M);
    RunTasks(vTask, report);
  }
  
  // Morton vs Dynamic
  {
    autoc szName = string("Cylindrical semi-random placed points Morton vs Dynamic");
    autoc aPoint = GenerateGeometry<N, vector<PointND<N>>>([&] { return CreatePoints_Random<N, 100*N1M>(); }, szName, 100, report);
    autoc aBox = GenerateGeometry<N, vector<BoundingBoxND<N>>>([&] { return CreateBoxes_Random<N, 100 * N1M>(); }, szName, 100, report);

    autoc vTaskMortonP = GeneratePointTasks<N>(nDepth, "Morton point", aPoint);
    autoc vTaskDynP = GeneratePointDynTasks_NonLog<N>(nDepth, "Dynamic point", aPoint);
    
    autoc vTaskMortonB = GenerateBoxTasks<N>(nDepth, "Morton box", aBox);
    autoc vTaskDynB = GenerateBoxDynTasks_NonLog<N>(nDepth, "Dynamic box", aBox);

    RunTasks(vTaskMortonP, report);
    RunTasks(vTaskDynP, report);
    RunTasks(vTaskMortonB, report);
    RunTasks(vTaskDynB, report);

  }
  // Search
  
  // Range search: Brute force vs Octree
  {
    autoc szName = string("Search: Cylindrical semi-random placed point NoPt/NoBox:100%");
    //autoc aPoint = GenerateGeometry<N, vector<PointND<N>>>([&] { return CreatePoints_CylindricalSemiRandom<N, N1M>(); }, szName, N1M, report);
    autoc aBox = GenerateGeometry<N, vector<BoundingBoxND<N>>>([&] { return CreateBoxes_CylindricalSemiRandom<N, static_cast<size_t>(aSizeNonLog.back())>(); }, szName, aSizeNonLog.back(), report);
    autoce rPercentage = 100.0;
    autoc vTaskBruteForce = SelfConflictBruteForceBoxTasks<N>("Box self conflict by brute force", aBox);
    autoc vTaskTree = SearchTreeBoxTasks<N>(-1, "Box self conflict by octree", aBox);

    RunTasks(vTaskBruteForce, report);
    RunTasks(vTaskTree, report);
  }

  report.close();
}
