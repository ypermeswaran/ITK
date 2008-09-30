#ifndef __itkQuadEdgeMeshDiscreteCurvatureEstimator_h
#define __itkQuadEegeMeshDiscreteCurvatureEstimator_h

#include <itkQuadEdgeMeshToQuadEdgeMeshFilter.h>
#include "itkTriangleHelper.h"

namespace itk
{
/**
 * \class QuadEdgeMeshDiscreteCurvatureEstimator
 * \brief
*/
template< class TInputMesh, class TOutputMesh >
class QuadEdgeMeshDiscreteCurvatureEstimator :
  public QuadEdgeMeshToQuadEdgeMeshFilter< TInputMesh, TOutputMesh >
{
public:
  typedef QuadEdgeMeshDiscreteCurvatureEstimator Self;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  typedef QuadEdgeMeshToQuadEdgeMeshFilter< TInputMesh, TOutputMesh > Superclass;

  typedef TInputMesh                                    InputMeshType;
  typedef typename InputMeshType::Pointer               InputMeshPointer;

  typedef TOutputMesh OutputMeshType;
  typedef typename OutputMeshType::Pointer              OutputMeshPointer;
  typedef typename OutputMeshType::PointsContainerPointer
    OutputPointsContainerPointer;
  typedef typename OutputMeshType::PointsContainerIterator
    OutputPointsContainerIterator;
  typedef typename OutputMeshType::PointType            OutputPointType;
  typedef typename OutputPointType::CoordRepType        OutputCoordType;
  typedef typename OutputMeshType::PointIdentifier      OutputPointIdentifier;
  typedef typename OutputMeshType::CellIdentifier       OutputCellIdentifier;
  typedef typename OutputMeshType::QEType               OutputQEType;
  typedef typename OutputMeshType::MeshTraits           OutputMeshTraits;
  typedef typename OutputMeshTraits::PixelType          OutputCurvatureType;

  typedef TriangleHelper< OutputPointType > TriangleType;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( QuadEdgeMeshDiscreteCurvatureEstimator,
    QuadEdgeMeshToQuadEdgeMeshFilter );

protected:
  QuadEdgeMeshDiscreteCurvatureEstimator() : Superclass() {}
  ~QuadEdgeMeshDiscreteCurvatureEstimator() {}

  virtual OutputCurvatureType EstimateCurvature( const OutputPointType& iP ) = 0;

  OutputCurvatureType ComputeMixedArea( OutputQEType* iQE1, OutputQEType* iQE2 )
  {
    OutputMeshPointer output = this->GetOutput();

    OutputPointIdentifier id[3];
    id[0] = iQE1->GetOrigin();
    id[1] = iQE1->GetDestination();
    id[2] = iQE2->GetDestination();

    OutputPointType p[3];

    for( int i = 0; i < 3; i++ )
      p[i] = output->GetPoint( id[i] );

    if( !TriangleType::IsObtuse( p[0], p[1], p[2] ) )
      {
      OutputCurvatureType sq_d01 =
        static_cast< OutputCurvatureType >(
          p[0].SquaredEuclideanDistanceTo( p[1] ) );
      OutputCurvatureType sq_d02 =
        static_cast< OutputCurvatureType >(
          p[0].SquaredEuclideanDistanceTo( p[2] ) );

      OutputCurvatureType cot_theta_210 =
        TriangleType::Cotangent( p[2], p[1], p[0] );
      OutputCurvatureType cot_theta_021 =
        TriangleType::Cotangent( p[0], p[2], p[1] );

      return 0.125 * ( sq_d02 * cot_theta_210 + sq_d01 * cot_theta_021 );
      }
    else
      {
      OutputCurvatureType area =
        static_cast< OutputCurvatureType >(
          TriangleType::ComputeArea( p[0], p[1], p[2] ) );
      if( ( p[1] - p[0] ) * ( p[2] - p[0] ) < 0. )
        return 0.5 * area;
      else
        return 0.25 * area;
      }
  }

  virtual void GenerateData()
  {
    Superclass::GenerateData();
    OutputMeshPointer output = this->GetOutput();

    OutputPointsContainerPointer points = output->GetPoints();

    for( OutputPointsContainerIterator p_it = points->Begin();
        p_it != points->End();
        p_it++ )
      {
      output->SetPointData( p_it->Index(), EstimateCurvature( p_it->Value() ) );
      }
  }

private:
  QuadEdgeMeshDiscreteCurvatureEstimator( const Self& );
  void operator = ( const Self& );
};
}
#endif
