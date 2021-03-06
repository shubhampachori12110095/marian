//=================================================================================================
/*!
//  \file blaze/math/views/column/Dense.h
//  \brief Column specialization for dense matrices
//
//  Copyright (C) 2013 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_VIEWS_COLUMN_DENSE_H_
#define _BLAZE_MATH_VIEWS_COLUMN_DENSE_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <iterator>
#include <blaze/math/Aliases.h>
#include <blaze/math/constraints/ColumnMajorMatrix.h>
#include <blaze/math/constraints/ColumnVector.h>
#include <blaze/math/constraints/Computation.h>
#include <blaze/math/constraints/DenseMatrix.h>
#include <blaze/math/constraints/DenseVector.h>
#include <blaze/math/constraints/RequiresEvaluation.h>
#include <blaze/math/constraints/RowMajorMatrix.h>
#include <blaze/math/constraints/Symmetric.h>
#include <blaze/math/constraints/TransExpr.h>
#include <blaze/math/constraints/UniTriangular.h>
#include <blaze/math/Exception.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/expressions/View.h>
#include <blaze/math/InitializerList.h>
#include <blaze/math/shims/Clear.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/SIMD.h>
#include <blaze/math/traits/ColumnTrait.h>
#include <blaze/math/traits/DerestrictTrait.h>
#include <blaze/math/typetraits/AreSIMDCombinable.h>
#include <blaze/math/typetraits/HasSIMDAdd.h>
#include <blaze/math/typetraits/HasSIMDDiv.h>
#include <blaze/math/typetraits/HasSIMDMult.h>
#include <blaze/math/typetraits/HasSIMDSub.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsLower.h>
#include <blaze/math/typetraits/IsPadded.h>
#include <blaze/math/typetraits/IsRestricted.h>
#include <blaze/math/typetraits/IsSparseVector.h>
#include <blaze/math/typetraits/IsStrictlyLower.h>
#include <blaze/math/typetraits/IsStrictlyUpper.h>
#include <blaze/math/typetraits/IsUniLower.h>
#include <blaze/math/typetraits/IsUniUpper.h>
#include <blaze/math/typetraits/IsUpper.h>
#include <blaze/math/views/column/BaseTemplate.h>
#include <blaze/system/CacheSize.h>
#include <blaze/system/Inline.h>
#include <blaze/system/Optimizations.h>
#include <blaze/system/Thresholds.h>
#include <blaze/util/Assert.h>
#include <blaze/util/constraints/Pointer.h>
#include <blaze/util/constraints/Reference.h>
#include <blaze/util/constraints/Vectorizable.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/mpl/Not.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/Template.h>
#include <blaze/util/TrueType.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsConst.h>
#include <blaze/util/typetraits/IsNumeric.h>
#include <blaze/util/typetraits/IsReference.h>
#include <blaze/util/typetraits/RemoveReference.h>


namespace blaze {

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR COLUMN-MAJOR DENSE MATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of Column for columns on column-major dense matrices.
// \ingroup views
//
// This specialization of Column adapts the class template to the requirements of column-major
// dense matrices.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
class Column<MT,true,true,SF>
   : public DenseVector< Column<MT,true,true,SF>, false >
   , private View
{
 private:
   //**Type definitions****************************************************************************
   //! Composite data type of the dense matrix expression.
   typedef If_< IsExpression<MT>, MT, MT& >  Operand;
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef Column<MT,true,true,SF>     This;           //!< Type of this Column instance.
   typedef DenseVector<This,false>     BaseType;       //!< Base type of this Column instance.
   typedef ColumnTrait_<MT>            ResultType;     //!< Result type for expression template evaluations.
   typedef TransposeType_<ResultType>  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<MT>            ElementType;    //!< Type of the column elements.
   typedef SIMDTrait_<ElementType>     SIMDType;       //!< SIMD type of the column elements.
   typedef ReturnType_<MT>             ReturnType;     //!< Return type for expression template evaluations
   typedef const Column&               CompositeType;  //!< Data type for composite expression templates.

   //! Reference to a constant column value.
   typedef ConstReference_<MT>  ConstReference;

   //! Reference to a non-constant column value.
   typedef If_< IsConst<MT>, ConstReference, Reference_<MT> >  Reference;

   //! Pointer to a constant column value.
   typedef const ElementType*  ConstPointer;

   //! Pointer to a non-constant column value.
   typedef If_< Or< IsConst<MT>, Not< HasMutableDataAccess<MT> > >, ConstPointer, ElementType* >  Pointer;

   //! Iterator over constant elements.
   typedef ConstIterator_<MT>  ConstIterator;

   //! Iterator over non-constant elements.
   typedef If_< IsConst<MT>, ConstIterator, Iterator_<MT> >  Iterator;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = MT::simdEnabled };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = MT::smpAssignable };
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline Column( MT& matrix, size_t index );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Reference      at( size_t index );
   inline ConstReference at( size_t index ) const;
   inline Pointer        data  () noexcept;
   inline ConstPointer   data  () const noexcept;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   inline Column& operator=( const ElementType& rhs );
   inline Column& operator=( initializer_list<ElementType> list );
   inline Column& operator=( const Column& rhs );

   template< typename VT > inline Column& operator= ( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator+=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator-=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator*=( const DenseVector<VT,false>&  rhs );
   template< typename VT > inline Column& operator*=( const SparseVector<VT,false>& rhs );
   template< typename VT > inline Column& operator/=( const DenseVector<VT,false>&  rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator*=( Other rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t  size() const noexcept;
                              inline size_t  capacity() const noexcept;
                              inline size_t  nonZeros() const;
                              inline void    reset();
   template< typename Other > inline Column& scale( const Other& scalar );
   //@}
   //**********************************************************************************************

 private:
   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedAddAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDAdd< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedSubAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDSub< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedMultAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDMult< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedDivAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDDiv< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**SIMD properties*****************************************************************************
   //! The number of elements packed within a single SIMD element.
   enum : size_t { SIMDSIZE = SIMDTrait<ElementType>::size };
   //**********************************************************************************************

 public:
   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other >
   inline bool canAlias( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool canAlias( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   template< typename Other >
   inline bool isAliased( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   inline bool isAligned   () const noexcept;
   inline bool canSMPAssign() const noexcept;

   BLAZE_ALWAYS_INLINE SIMDType load ( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loada( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loadu( size_t index ) const noexcept;

   BLAZE_ALWAYS_INLINE void store ( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storea( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storeu( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void stream( size_t index, const SIMDType& value ) noexcept;

   template< typename VT >
   inline DisableIf_< VectorizedAssign<VT> > assign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedAssign<VT> > assign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void assign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedAddAssign<VT> > addAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedAddAssign<VT> > addAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void addAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedSubAssign<VT> > subAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedSubAssign<VT> > subAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void subAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedMultAssign<VT> > multAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedMultAssign<VT> > multAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void multAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedDivAssign<VT> > divAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedDivAssign<VT> > divAssign( const DenseVector<VT,false>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   Operand      matrix_;  //!< The dense matrix containing the column.
   const size_t col_;     //!< The index of the column in the matrix.
   //@}
   //**********************************************************************************************

   //**Friend declarations*************************************************************************
   template< typename MT2, bool SO2, bool DF2, bool SF2 > friend class Column;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isIntact( const Column<MT2,SO2,DF2,SF2>& column ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isSame( const Column<MT2,SO2,DF2,SF2>& a, const Column<MT2,SO2,DF2,SF2>& b ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAddAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool trySubAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryMultAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryDivAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend DerestrictTrait_< Column<MT2,SO2,DF2,SF2> > derestrict( Column<MT2,SO2,DF2,SF2>& column );
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE       ( MT );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_TRANSEXPR_TYPE      ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE        ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE      ( MT );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The constructor for Column.
//
// \param matrix The matrix containing the column.
// \param index The index of the column.
// \exception std::invalid_argument Invalid column access index.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline Column<MT,true,true,SF>::Column( MT& matrix, size_t index )
   : matrix_( matrix )  // The dense matrix containing the column
   , col_   ( index  )  // The index of the column in the matrix
{
   if( matrix_.columns() <= index ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid column access index" );
   }
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::Reference Column<MT,true,true,SF>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return matrix_(index,col_);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstReference
   Column<MT,true,true,SF>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return const_cast<const MT&>( matrix_ )(index,col_);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::Reference Column<MT,true,true,SF>::at( size_t index )
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstReference
   Column<MT,true,true,SF>::at( size_t index ) const
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::Pointer Column<MT,true,true,SF>::data() noexcept
{
   return matrix_.data( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstPointer Column<MT,true,true,SF>::data() const noexcept
{
   return matrix_.data( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::Iterator Column<MT,true,true,SF>::begin()
{
   return matrix_.begin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstIterator Column<MT,true,true,SF>::begin() const
{
   return matrix_.cbegin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstIterator Column<MT,true,true,SF>::cbegin() const
{
   return matrix_.cbegin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::Iterator Column<MT,true,true,SF>::end()
{
   return matrix_.end( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstIterator Column<MT,true,true,SF>::end() const
{
   return matrix_.cend( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline typename Column<MT,true,true,SF>::ConstIterator Column<MT,true,true,SF>::cend() const
{
   return matrix_.cend( col_ );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Homogenous assignment to all column elements.
//
// \param rhs Scalar value to be assigned to all column elements.
// \return Reference to the assigned column.
//
// This function homogeneously assigns the given value to all elements of the column. Note that in
// case the underlying dense matrix is a lower/upper matrix only lower/upper and diagonal elements
// of the underlying matrix are modified.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator=( const ElementType& rhs )
{
   const size_t ibegin( ( IsLower<MT>::value )
                        ?( ( IsUniLower<MT>::value || IsStrictlyLower<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t iend  ( ( IsUpper<MT>::value )
                        ?( ( IsUniUpper<MT>::value || IsStrictlyUpper<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t i=ibegin; i<iend; ++i )
      matrix_(i,col_) = rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief List assignment to all column elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to column.
//
// This assignment operator offers the option to directly assign to all elements of the dense
// column by means of an initializer list. The column elements are assigned the values from the
// given initializer list. Missing values are reset to their default state. Note that in case
// the size of the initializer list exceeds the size of the column, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator=( initializer_list<ElementType> list )
{
   if( list.size() > size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to column" );
   }

   std::fill( std::copy( list.begin(), list.end(), begin() ), end(), ElementType() );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Copy assignment operator for Column.
//
// \param rhs Dense column to be copied.
// \return Reference to the assigned column.
// \exception std::invalid_argument Column sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two columns don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator=( const Column& rhs )
{
   if( &rhs == this ) return *this;

   if( size() != rhs.size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Column sizes do not match" );
   }

   if( !tryAssign( matrix_, rhs, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, rhs );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be assigned.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpAssign( left, tmp );
   }
   else {
      if( IsSparseVector<VT>::value )
         reset();
      smpAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator+=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAddAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpAddAssign( left, tmp );
   }
   else {
      smpAddAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator-=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !trySubAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpSubAssign( left, tmp );
   }
   else {
      smpSubAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a dense vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator*=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryMultAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpMultAssign( left, tmp );
   }
   else {
      smpMultAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a sparse vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side sparse vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side sparse vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator*=( const SparseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE  ( ResultType );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   const ResultType right( *this * (~rhs) );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, right );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense vector (\f$ \vec{a}/=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector divisor.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::operator/=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryDivAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpDivAssign( left, tmp );
   }
   else {
      smpDivAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication between a dense column and
//        a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
*/
template< typename MT       // Type of the dense matrix
        , bool SF >         // Symmetry flag
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,true,true,SF> >&
   Column<MT,true,true,SF>::operator*=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   return operator=( (*this) * rhs );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense column by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
//
// \note A division by zero is only checked by an user assert.
*/
template< typename MT       // Type of the dense matrix
        , bool SF >         // Symmetry flag
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,true,true,SF> >&
   Column<MT,true,true,SF>::operator/=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   return operator=( (*this) / rhs );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current size/dimension of the column.
//
// \return The size of the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline size_t Column<MT,true,true,SF>::size() const noexcept
{
   return matrix_.rows();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the maximum capacity of the dense column.
//
// \return The capacity of the dense column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline size_t Column<MT,true,true,SF>::capacity() const noexcept
{
   return matrix_.capacity( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the number of non-zero elements in the column.
//
// \return The number of non-zero elements in the column.
//
// Note that the number of non-zero elements is always less than or equal to the current number
// of rows of the matrix containing the column.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline size_t Column<MT,true,true,SF>::nonZeros() const
{
   return matrix_.nonZeros( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline void Column<MT,true,true,SF>::reset()
{
   matrix_.reset( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling of the column by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the column scaling.
// \return Reference to the dense column.
//
// This function scales all elements of the column by the given scalar value \a scalar. Note that
// the function cannot be used to scale a column on a lower or upper unitriangular matrix. The
// attempt to scale such a row results in a compile time error!
*/
template< typename MT       // Type of the dense matrix
        , bool SF >         // Symmetry flag
template< typename Other >  // Data type of the scalar value
inline Column<MT,true,true,SF>& Column<MT,true,true,SF>::scale( const Other& scalar )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   const size_t ibegin( ( IsLower<MT>::value )
                        ?( ( IsStrictlyLower<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t iend  ( ( IsUpper<MT>::value )
                        ?( ( IsStrictlyUpper<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t i=ibegin; i<iend; ++i ) {
      matrix_(i,col_) *= scalar;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In
// contrast to the isAliased() function this function is allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT       // Type of the dense matrix
        , bool SF >         // Symmetry flag
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,true,true,SF>::canAlias( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In
// contrast to the isAliased() function this function is allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT   // Type of the dense matrix
        , bool SF >     // Symmetry flag
template< typename MT2  // Data type of the foreign dense column
        , bool SO2      // Storage order of the foreign dense column
        , bool SF2 >    // Symmetry flag of the foreign dense column
inline bool Column<MT,true,true,SF>::canAlias( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In
// contrast to the canAlias() function this function is not allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT       // Type of the dense matrix
        , bool SF >         // Symmetry flag
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,true,true,SF>::isAliased( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In
// contrast to the canAlias() function this function is not allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT   // Type of the dense matrix
        , bool SF >     // Symmetry flag
template< typename MT2  // Data type of the foreign dense column
        , bool SO2      // Storage order of the foreign dense column
        , bool SF2 >    // Symmetry flag of the foreign dense column
inline bool Column<MT,true,true,SF>::isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( &alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is properly aligned in memory.
//
// \return \a true in case the dense column is aligned, \a false if not.
//
// This function returns whether the dense column is guaranteed to be properly aligned in memory,
// i.e. whether the beginning and the end of the dense column are guaranteed to conform to the
// alignment restrictions of the element type \a Type.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline bool Column<MT,true,true,SF>::isAligned() const noexcept
{
   return matrix_.isAligned();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can be used in SMP assignments.
//
// \return \a true in case the dense column can be used in SMP assignments, \a false if not.
//
// This function returns whether the dense column can be used in SMP assignments. In contrast
// to the \a smpAssignable member enumeration, which is based solely on compile time information,
// this function additionally provides runtime information (as for instance the current size of
// the dense column).
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
inline bool Column<MT,true,true,SF>::canSMPAssign() const noexcept
{
   return ( size() > SMP_DVECASSIGN_THRESHOLD );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs a load of a specific SIMD element of the dense column. The index must
// be smaller than the number of matrix rows. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE typename Column<MT,true,true,SF>::SIMDType
   Column<MT,true,true,SF>::load( size_t index ) const noexcept
{
   return matrix_.load( index, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs an aligned load of a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE typename Column<MT,true,true,SF>::SIMDType
   Column<MT,true,true,SF>::loada( size_t index ) const noexcept
{
   return matrix_.loada( index, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Unaligned load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs an unaligned load of a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE typename Column<MT,true,true,SF>::SIMDType
   Column<MT,true,true,SF>::loadu( size_t index ) const noexcept
{
   return matrix_.loadu( index, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs a store a specific SIMD element of the dense column. The index must
// be smaller than the number of matrix rows. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE void
   Column<MT,true,true,SF>::store( size_t index, const SIMDType& value ) noexcept
{
   matrix_.store( index, col_, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned store a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE void
   Column<MT,true,true,SF>::storea( size_t index, const SIMDType& value ) noexcept
{
   matrix_.storea( index, col_, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Unaligned store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an unaligned store a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE void
   Column<MT,true,true,SF>::storeu( size_t index, const SIMDType& value ) noexcept
{
   matrix_.storeu( index, col_, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned, non-temporal store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned, non-temporal store a specific SIMD element of the dense
// column. The index must be smaller than the number of matrix rows. This function must \b NOT
// be called explicitly! It is used internally for the performance optimized evaluation of
// expression templates. Calling this function explicitly might result in erroneous results
// and/or in compilation errors.
*/
template< typename MT  // Type of the dense matrix
        , bool SF >    // Symmetry flag
BLAZE_ALWAYS_INLINE void
   Column<MT,true,true,SF>::stream( size_t index, const SIMDType& value ) noexcept
{
   matrix_.stream( index, col_, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedAssign<VT> >
   Column<MT,true,true,SF>::assign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) = (~rhs)[i    ];
      matrix_(i+1UL,col_) = (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) = (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedAssign<VT> >
   Column<MT,true,true,SF>::assign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t rows( size() );

   const size_t ipos( ( remainder )?( rows & size_t(-SIMDSIZE) ):( rows ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( rows - ( rows % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

   size_t i( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   if( useStreaming && rows > ( cacheSize/( sizeof(ElementType) * 3UL ) ) && !(~rhs).isAliased( this ) )
   {
      for( ; i<ipos; i+=SIMDSIZE ) {
         left.stream( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; remainder && i<rows; ++i ) {
         *left = *right; ++left; ++right;
      }
   }
   else
   {
      for( ; (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL ) {
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; i<ipos; i+=SIMDSIZE ) {
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; remainder && i<rows; ++i ) {
         *left = *right; ++left; ++right;
      }
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,true,true,SF>::assign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) = element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedAddAssign<VT> >
   Column<MT,true,true,SF>::addAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) += (~rhs)[i    ];
      matrix_(i+1UL,col_) += (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) += (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedAddAssign<VT> >
   Column<MT,true,true,SF>::addAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t rows( size() );

   const size_t ipos( ( remainder )?( rows & size_t(-SIMDSIZE) ):( rows ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( rows - ( rows % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

   size_t i( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL ) {
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; i<ipos; i+=SIMDSIZE ) {
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && i<rows; ++i ) {
      *left += *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,true,true,SF>::addAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) += element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedSubAssign<VT> >
   Column<MT,true,true,SF>::subAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) -= (~rhs)[i    ];
      matrix_(i+1UL,col_) -= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) -= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedSubAssign<VT> >
   Column<MT,true,true,SF>::subAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t rows( size() );

   const size_t ipos( ( remainder )?( rows & size_t(-SIMDSIZE) ):( rows ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( rows - ( rows % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

   size_t i( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL ) {
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; i<ipos; i+=SIMDSIZE ) {
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && i<rows; ++i ) {
      *left -= *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,true,true,SF>::subAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) -= element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedMultAssign<VT> >
   Column<MT,true,true,SF>::multAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) *= (~rhs)[i    ];
      matrix_(i+1UL,col_) *= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) *= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedMultAssign<VT> >
   Column<MT,true,true,SF>::multAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t rows( size() );

   const size_t ipos( ( remainder )?( rows & size_t(-SIMDSIZE) ):( rows ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( rows - ( rows % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

   size_t i( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL ) {
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; i<ipos; i+=SIMDSIZE ) {
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && i<rows; ++i ) {
      *left *= *right; ++left, ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,true,true,SF>::multAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const ResultType tmp( serial( *this ) );

   reset();

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) = tmp[element->index()] * element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedDivAssign<VT> >
   Column<MT,true,true,SF>::divAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) /= (~rhs)[i    ];
      matrix_(i+1UL,col_) /= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) /= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the dense matrix
        , bool SF >      // Symmetry flag
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,true,true,SF>::BLAZE_TEMPLATE VectorizedDivAssign<VT> >
   Column<MT,true,true,SF>::divAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t rows( size() );

   const size_t ipos( rows & size_t(-SIMDSIZE) );
   BLAZE_INTERNAL_ASSERT( ( rows - ( rows % (SIMDSIZE) ) ) == ipos, "Invalid end calculation" );

   size_t i( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL ) {
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; i<ipos; i+=SIMDSIZE ) {
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; i<rows; ++i ) {
      *left /= *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************








//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR GENERAL ROW-MAJOR DENSE MATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of Column for general row-major dense matrices.
// \ingroup views
//
// This specialization of Column adapts the class template to the requirements of general
// row-major dense matrices.
*/
template< typename MT >  // Type of the dense matrix
class Column<MT,false,true,false>
   : public DenseVector< Column<MT,false,true,false>, false >
   , private View
{
 private:
   //**Type definitions****************************************************************************
   //! Composite data type of the dense matrix expression.
   typedef If_< IsExpression<MT>, MT, MT& >  Operand;
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef Column<MT,false,true,false>  This;           //!< Type of this Column instance.
   typedef DenseVector<This,false>      BaseType;       //!< Base type of this Column instance.
   typedef ColumnTrait_<MT>             ResultType;     //!< Result type for expression template evaluations.
   typedef TransposeType_<ResultType>   TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<MT>             ElementType;    //!< Type of the column elements.
   typedef ReturnType_<MT>              ReturnType;     //!< Return type for expression template evaluations
   typedef const Column&                CompositeType;  //!< Data type for composite expression templates.

   //! Reference to a constant column value.
   typedef ConstReference_<MT>  ConstReference;

   //! Reference to a non-constant column value.
   typedef If_< IsConst<MT>, ConstReference, Reference_<MT> >  Reference;

   //! Pointer to a constant column value.
   typedef const ElementType*  ConstPointer;

   //! Pointer to a non-constant column value.
   typedef If_< Or< IsConst<MT>, Not< HasMutableDataAccess<MT> > >, ConstPointer, ElementType* >  Pointer;
   //**********************************************************************************************

   //**ColumnIterator class definition*************************************************************
   /*!\brief Iterator over the elements of the dense column.
   */
   template< typename MatrixType >  // Type of the dense matrix
   class ColumnIterator
   {
    public:
      //**Type definitions*************************************************************************
      //! Return type for the access to the value of a dense element.
      typedef If_< IsConst<MatrixType>, ConstReference_<MatrixType>, Reference_<MatrixType> >  Reference;

      typedef std::random_access_iterator_tag  IteratorCategory;  //!< The iterator category.
      typedef RemoveReference_<Reference>      ValueType;         //!< Type of the underlying elements.
      typedef ValueType*                       PointerType;       //!< Pointer return type.
      typedef Reference                        ReferenceType;     //!< Reference return type.
      typedef ptrdiff_t                        DifferenceType;    //!< Difference between two iterators.

      // STL iterator requirements
      typedef IteratorCategory  iterator_category;  //!< The iterator category.
      typedef ValueType         value_type;         //!< Type of the underlying elements.
      typedef PointerType       pointer;            //!< Pointer return type.
      typedef ReferenceType     reference;          //!< Reference return type.
      typedef DifferenceType    difference_type;    //!< Difference between two iterators.
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Default constructor of the ColumnIterator class.
      */
      inline ColumnIterator() noexcept
         : matrix_( nullptr )  // The dense matrix containing the column.
         , row_   ( 0UL )      // The current row index.
         , column_( 0UL )      // The current column index.
      {}
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor of the ColumnIterator class.
      //
      // \param matrix The matrix containing the column.
      // \param row The row index.
      // \param column The column index.
      */
      inline ColumnIterator( MatrixType& matrix, size_t row, size_t column ) noexcept
         : matrix_( &matrix )  // The dense matrix containing the column.
         , row_   ( row     )  // The current row index.
         , column_( column  )  // The current column index.
      {}
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Conversion constructor from different ColumnIterator instances.
      //
      // \param it The column iterator to be copied.
      */
      template< typename MatrixType2 >
      inline ColumnIterator( const ColumnIterator<MatrixType2>& it ) noexcept
         : matrix_( it.matrix_ )  // The dense matrix containing the column.
         , row_   ( it.row_    )  // The current row index.
         , column_( it.column_ )  // The current column index.
      {}
      //*******************************************************************************************

      //**Addition assignment operator*************************************************************
      /*!\brief Addition assignment operator.
      //
      // \param inc The increment of the iterator.
      // \return The incremented iterator.
      */
      inline ColumnIterator& operator+=( size_t inc ) noexcept {
         row_ += inc;
         return *this;
      }
      //*******************************************************************************************

      //**Subtraction assignment operator**********************************************************
      /*!\brief Subtraction assignment operator.
      //
      // \param dec The decrement of the iterator.
      // \return The decremented iterator.
      */
      inline ColumnIterator& operator-=( size_t dec ) noexcept {
         row_ -= dec;
         return *this;
      }
      //*******************************************************************************************

      //**Prefix increment operator****************************************************************
      /*!\brief Pre-increment operator.
      //
      // \return Reference to the incremented iterator.
      */
      inline ColumnIterator& operator++() noexcept {
         ++row_;
         return *this;
      }
      //*******************************************************************************************

      //**Postfix increment operator***************************************************************
      /*!\brief Post-increment operator.
      //
      // \return The previous position of the iterator.
      */
      inline const ColumnIterator operator++( int ) noexcept {
         const ColumnIterator tmp( *this );
         ++(*this);
         return tmp;
      }
      //*******************************************************************************************

      //**Prefix decrement operator****************************************************************
      /*!\brief Pre-decrement operator.
      //
      // \return Reference to the decremented iterator.
      */
      inline ColumnIterator& operator--() noexcept {
         --row_;
         return *this;
      }
      //*******************************************************************************************

      //**Postfix decrement operator***************************************************************
      /*!\brief Post-decrement operator.
      //
      // \return The previous position of the iterator.
      */
      inline const ColumnIterator operator--( int ) noexcept {
         const ColumnIterator tmp( *this );
         --(*this);
         return tmp;
      }
      //*******************************************************************************************

      //**Subscript operator***********************************************************************
      /*!\brief Direct access to the dense column elements.
      //
      // \param index Access index.
      // \return Reference to the accessed value.
      */
      inline ReferenceType operator[]( size_t index ) const {
         return (*matrix_)(row_+index,column_);
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the dense vector element at the current iterator position.
      //
      // \return The current value of the dense element.
      */
      inline ReferenceType operator*() const {
         return (*matrix_)(row_,column_);
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the dense vector element at the current iterator position.
      //
      // \return Reference to the dense vector element at the current iterator position.
      */
      inline PointerType operator->() const {
         return &(*matrix_)(row_,column_);
      }
      //*******************************************************************************************

      //**Equality operator************************************************************************
      /*!\brief Equality comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the iterators refer to the same element, \a false if not.
      */
      template< typename MatrixType2 >
      inline bool operator==( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return ( matrix_ == rhs.matrix_ ) && ( row_ == rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Inequality operator**********************************************************************
      /*!\brief Inequality comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the iterators don't refer to the same element, \a false if they do.
      */
      template< typename MatrixType2 >
      inline bool operator!=( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return !( *this == rhs );
      }
      //*******************************************************************************************

      //**Less-than operator***********************************************************************
      /*!\brief Less-than comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the left-hand side iterator is smaller, \a false if not.
      */
      template< typename MatrixType2 >
      inline bool operator<( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return ( matrix_ == rhs.matrix_ ) && ( row_ < rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Greater-than operator********************************************************************
      /*!\brief Greater-than comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the left-hand side iterator is greater, \a false if not.
      */
      template< typename MatrixType2 >
      inline bool operator>( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return ( matrix_ == rhs.matrix_ ) && ( row_ > rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Less-or-equal-than operator**************************************************************
      /*!\brief Less-than comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the left-hand side iterator is smaller or equal, \a false if not.
      */
      template< typename MatrixType2 >
      inline bool operator<=( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return ( matrix_ == rhs.matrix_ ) && ( row_ <= rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Greater-or-equal-than operator***********************************************************
      /*!\brief Greater-than comparison between two ColumnIterator objects.
      //
      // \param rhs The right-hand side column iterator.
      // \return \a true if the left-hand side iterator is greater or equal, \a false if not.
      */
      template< typename MatrixType2 >
      inline bool operator>=( const ColumnIterator<MatrixType2>& rhs ) const noexcept {
         return ( matrix_ == rhs.matrix_ ) && ( row_ >= rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Subtraction operator*********************************************************************
      /*!\brief Calculating the number of elements between two column iterators.
      //
      // \param rhs The right-hand side column iterator.
      // \return The number of elements between the two column iterators.
      */
      inline DifferenceType operator-( const ColumnIterator& rhs ) const noexcept {
         return row_ - rhs.row_;
      }
      //*******************************************************************************************

      //**Addition operator************************************************************************
      /*!\brief Addition between a ColumnIterator and an integral value.
      //
      // \param it The iterator to be incremented.
      // \param inc The number of elements the iterator is incremented.
      // \return The incremented iterator.
      */
      friend inline const ColumnIterator operator+( const ColumnIterator& it, size_t inc ) noexcept {
         return ColumnIterator( *it.matrix_, it.row_+inc, it.column_ );
      }
      //*******************************************************************************************

      //**Addition operator************************************************************************
      /*!\brief Addition between an integral value and a ColumnIterator.
      //
      // \param inc The number of elements the iterator is incremented.
      // \param it The iterator to be incremented.
      // \return The incremented iterator.
      */
      friend inline const ColumnIterator operator+( size_t inc, const ColumnIterator& it ) noexcept {
         return ColumnIterator( *it.matrix_, it.row_+inc, it.column_ );
      }
      //*******************************************************************************************

      //**Subtraction operator*********************************************************************
      /*!\brief Subtraction between a ColumnIterator and an integral value.
      //
      // \param it The iterator to be decremented.
      // \param inc The number of elements the iterator is decremented.
      // \return The decremented iterator.
      */
      friend inline const ColumnIterator operator-( const ColumnIterator& it, size_t dec ) noexcept {
         return ColumnIterator( *it.matrix_, it.row_-dec, it.column_ );
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      MatrixType* matrix_;  //!< The dense matrix containing the column.
      size_t      row_;     //!< The current row index.
      size_t      column_;  //!< The current column index.
      //*******************************************************************************************

      //**Friend declarations**********************************************************************
      template< typename MatrixType2 > friend class ColumnIterator;
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**Type definitions****************************************************************************
   //! Iterator over constant elements.
   typedef ColumnIterator<const MT>  ConstIterator;

   //! Iterator over non-constant elements.
   typedef If_< IsConst<MT>, ConstIterator, ColumnIterator<MT> >  Iterator;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = false };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = MT::smpAssignable };
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline Column( MT& matrix, size_t index );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Reference      at( size_t index );
   inline ConstReference at( size_t index ) const;
   inline Pointer        data  () noexcept;
   inline ConstPointer   data  () const noexcept;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   inline Column& operator=( const ElementType& rhs );
   inline Column& operator=( initializer_list<ElementType> list );
   inline Column& operator=( const Column& rhs );

   template< typename VT > inline Column& operator= ( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator+=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator-=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator*=( const DenseVector<VT,false>&  rhs );
   template< typename VT > inline Column& operator*=( const SparseVector<VT,false>& rhs );
   template< typename VT > inline Column& operator/=( const DenseVector<VT,false>&  rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator*=( Other rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t  size() const noexcept;
                              inline size_t  capacity() const noexcept;
                              inline size_t  nonZeros() const;
                              inline void    reset();
   template< typename Other > inline Column& scale( const Other& scalar );
   //@}
   //**********************************************************************************************

 public:
   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other >
   inline bool canAlias ( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool canAlias ( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   template< typename Other >
   inline bool isAliased( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   inline bool isAligned   () const noexcept;
   inline bool canSMPAssign() const noexcept;

   template< typename VT > inline void assign    ( const DenseVector <VT,false>& rhs );
   template< typename VT > inline void assign    ( const SparseVector<VT,false>& rhs );
   template< typename VT > inline void addAssign ( const DenseVector <VT,false>& rhs );
   template< typename VT > inline void addAssign ( const SparseVector<VT,false>& rhs );
   template< typename VT > inline void subAssign ( const DenseVector <VT,false>& rhs );
   template< typename VT > inline void subAssign ( const SparseVector<VT,false>& rhs );
   template< typename VT > inline void multAssign( const DenseVector <VT,false>& rhs );
   template< typename VT > inline void multAssign( const SparseVector<VT,false>& rhs );
   template< typename VT > inline void divAssign ( const DenseVector <VT,false>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   Operand      matrix_;  //!< The dense matrix containing the column.
   const size_t col_;     //!< The index of the column in the matrix.
   //@}
   //**********************************************************************************************

   //**Friend declarations*************************************************************************
   template< typename MT2, bool SO2, bool DF2, bool SF2 > friend class Column;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isIntact( const Column<MT2,SO2,DF2,SF2>& column ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isSame( const Column<MT2,SO2,DF2,SF2>& a, const Column<MT2,SO2,DF2,SF2>& b ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAddAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool trySubAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryMultAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryDivAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend DerestrictTrait_< Column<MT2,SO2,DF2,SF2> > derestrict( Column<MT2,SO2,DF2,SF2>& column );
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE        ( MT );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE     ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_TRANSEXPR_TYPE       ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE         ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE       ( MT );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The constructor for Column.
//
// \param matrix The matrix containing the column.
// \param index The index of the column.
// \exception std::invalid_argument Invalid column access index.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,false>::Column( MT& matrix, size_t index )
   : matrix_( matrix )  // The dense matrix containing the column
   , col_   ( index  )  // The index of the column in the matrix
{
   if( matrix_.columns() <= index ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid column access index" );
   }
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::Reference
   Column<MT,false,true,false>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return matrix_(index,col_);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstReference
   Column<MT,false,true,false>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return const_cast<const MT&>( matrix_ )(index,col_);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::Reference
   Column<MT,false,true,false>::at( size_t index )
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstReference
   Column<MT,false,true,false>::at( size_t index ) const
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::Pointer Column<MT,false,true,false>::data() noexcept
{
   return matrix_.data() + col_;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstPointer Column<MT,false,true,false>::data() const noexcept
{
   return matrix_.data() + col_;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::Iterator Column<MT,false,true,false>::begin()
{
   return Iterator( matrix_, 0UL, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstIterator
   Column<MT,false,true,false>::begin() const
{
   return ConstIterator( matrix_, 0UL, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstIterator
   Column<MT,false,true,false>::cbegin() const
{
   return ConstIterator( matrix_, 0UL, col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::Iterator Column<MT,false,true,false>::end()
{
   return Iterator( matrix_, size(), col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstIterator
   Column<MT,false,true,false>::end() const
{
   return ConstIterator( matrix_, size(), col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,false>::ConstIterator
   Column<MT,false,true,false>::cend() const
{
   return ConstIterator( matrix_, size(), col_ );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Homogenous assignment to all column elements.
//
// \param rhs Scalar value to be assigned to all column elements.
// \return Reference to the assigned column.
//
// This function homogeneously assigns the given value to all elements of the column. Note that in
// case the underlying dense matrix is a lower/upper matrix only lower/upper and diagonal elements
// of the underlying matrix are modified.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator=( const ElementType& rhs )
{
   const size_t ibegin( ( IsLower<MT>::value )
                        ?( ( IsUniLower<MT>::value || IsStrictlyLower<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t iend  ( ( IsUpper<MT>::value )
                        ?( ( IsUniUpper<MT>::value || IsStrictlyUpper<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t i=ibegin; i<iend; ++i )
      matrix_(i,col_) = rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief List assignment to all column elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to column.
//
// This assignment operator offers the option to directly assign to all elements of the dense
// column by means of an initializer list. The column elements are assigned the values from the
// given initializer list. Missing values are reset to their default state. Note that in case
// the size of the initializer list exceeds the size of the column, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator=( initializer_list<ElementType> list )
{
   if( list.size() > size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to column" );
   }

   std::fill( std::copy( list.begin(), list.end(), begin() ), end(), ElementType() );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Copy assignment operator for Column.
//
// \param rhs Dense column to be copied.
// \return Reference to the assigned column.
// \exception std::invalid_argument Column sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two columns don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator=( const Column& rhs )
{
   if( &rhs == this ) return *this;

   if( size() != rhs.size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Column sizes do not match" );
   }

   if( !tryAssign( matrix_, rhs, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, rhs );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be assigned.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE  ( ResultType );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType tmp( right );
      smpAssign( left, tmp );
   }
   else {
      if( IsSparseVector<VT>::value )
         reset();
      smpAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator+=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAddAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpAddAssign( left, tmp );
   }
   else {
      smpAddAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator-=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !trySubAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpSubAssign( left, tmp );
   }
   else {
      smpSubAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a dense vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator*=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryMultAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpMultAssign( left, tmp );
   }
   else {
      smpMultAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a sparse vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side sparse vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator*=( const SparseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE  ( ResultType );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   const ResultType right( *this * (~rhs) );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, right );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense vector (\f$ \vec{a}/=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector divisor.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,false,true,false>&
   Column<MT,false,true,false>::operator/=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryDivAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpDivAssign( left, tmp );
   }
   else {
      smpDivAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication between a dense column and
//        a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,false,true,false> >&
   Column<MT,false,true,false>::operator*=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   return operator=( (*this) * rhs );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense column by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
//
// \note A division by zero is only checked by an user assert.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,false,true,false> >&
   Column<MT,false,true,false>::operator/=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   return operator=( (*this) / rhs );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current size/dimension of the column.
//
// \return The size of the column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,false>::size() const noexcept
{
   return matrix_.rows();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the maximum capacity of the dense column.
//
// \return The capacity of the dense column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,false>::capacity() const noexcept
{
   return matrix_.rows();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the number of non-zero elements in the column.
//
// \return The number of non-zero elements in the column.
//
// Note that the number of non-zero elements is always less than or equal to the current number
// of columns of the matrix containing the column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,false>::nonZeros() const
{
   const size_t rows( size() );
   size_t nonzeros( 0UL );

   for( size_t i=0UL; i<rows; ++i )
      if( !isDefault( matrix_(i,col_) ) )
         ++nonzeros;

   return nonzeros;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT >  // Type of the dense matrix
inline void Column<MT,false,true,false>::reset()
{
   using blaze::clear;

   const size_t ibegin( ( IsLower<MT>::value )
                        ?( ( IsUniLower<MT>::value || IsStrictlyLower<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t iend  ( ( IsUpper<MT>::value )
                        ?( ( IsUniUpper<MT>::value || IsStrictlyUpper<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t i=ibegin; i<iend; ++i )
      clear( matrix_(i,col_) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling of the column by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the column scaling.
// \return Reference to the dense column.
//
// This function scales all elements of the column by the given scalar value \a scalar. Note that
// the function cannot be used to scale a column on a lower or upper unitriangular matrix. The
// attempt to scale such a row results in a compile time error!
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the scalar value
inline Column<MT,false,true,false>& Column<MT,false,true,false>::scale( const Other& scalar )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   const size_t ibegin( ( IsLower<MT>::value )
                        ?( ( IsStrictlyLower<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t iend  ( ( IsUpper<MT>::value )
                        ?( ( IsStrictlyUpper<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t i=ibegin; i<iend; ++i ) {
      matrix_(i,col_) *= scalar;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In contrast
// to the isAliased() function this function is allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,false,true,false>::canAlias( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In contrast
// to the isAliased() function this function is allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT >  // Type of the dense matrix
template< typename MT2   // Data type of the foreign dense column
        , bool SO2       // Storage order of the foreign dense column
        , bool SF2 >     // Symmetry flag of the foreign dense column
inline bool Column<MT,false,true,false>::canAlias( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( &alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,false,true,false>::isAliased( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT >  // Type of the dense matrix
template< typename MT2   // Data type of the foreign dense column
        , bool SO2       // Storage order of the foreign dense column
        , bool SF2 >     // Symmetry flag of the foreign dense column
inline bool Column<MT,false,true,false>::isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( &alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is properly aligned in memory.
//
// \return \a true in case the dense column is aligned, \a false if not.
//
// This function returns whether the dense column is guaranteed to be properly aligned in memory,
// i.e. whether the beginning and the end of the dense column are guaranteed to conform to the
// alignment restrictions of the element type \a Type.
*/
template< typename MT >  // Type of the dense matrix
inline bool Column<MT,false,true,false>::isAligned() const noexcept
{
   return false;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can be used in SMP assignments.
//
// \return \a true in case the dense column can be used in SMP assignments, \a false if not.
//
// This function returns whether the dense column can be used in SMP assignments. In contrast
// to the \a smpAssignable member enumeration, which is based solely on compile time information,
// this function additionally provides runtime information (as for instance the current size of
// the dense column).
*/
template< typename MT >  // Type of the dense matrix
inline bool Column<MT,false,true,false>::canSMPAssign() const noexcept
{
   return ( size() > SMP_DVECASSIGN_THRESHOLD );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void Column<MT,false,true,false>::assign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) = (~rhs)[i    ];
      matrix_(i+1UL,col_) = (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) = (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,false>::assign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) = element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void Column<MT,false,true,false>::addAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) += (~rhs)[i    ];
      matrix_(i+1UL,col_) += (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) += (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,false>::addAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) += element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void Column<MT,false,true,false>::subAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) -= (~rhs)[i    ];
      matrix_(i+1UL,col_) -= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) -= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,false>::subAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) -= element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void Column<MT,false,true,false>::multAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) *= (~rhs)[i    ];
      matrix_(i+1UL,col_) *= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) *= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,false>::multAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const ResultType tmp( serial( *this ) );

   reset();

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(element->index(),col_) = tmp[element->index()] * element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void Column<MT,false,true,false>::divAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t ipos( (~rhs).size() & size_t(-2) );
   for( size_t i=0UL; i<ipos; i+=2UL ) {
      matrix_(i    ,col_) /= (~rhs)[i    ];
      matrix_(i+1UL,col_) /= (~rhs)[i+1UL];
   }
   if( ipos < (~rhs).size() )
      matrix_(ipos,col_) /= (~rhs)[ipos];
}
/*! \endcond */
//*************************************************************************************************








//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR SYMMETRIC ROW-MAJOR DENSE MATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of Column for symmetric row-major dense matrices.
// \ingroup views
//
// This specialization of Column adapts the class template to the requirements of symmetric
// row-major dense matrices.
*/
template< typename MT >  // Type of the dense matrix
class Column<MT,false,true,true>
   : public DenseVector< Column<MT,false,true,true>, false >
   , private View
{
 private:
   //**Type definitions****************************************************************************
   //! Composite data type of the dense matrix expression.
   typedef If_< IsExpression<MT>, MT, MT& >  Operand;
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef Column<MT,false,true,true>  This;           //!< Type of this Column instance.
   typedef DenseVector<This,false>     BaseType;       //!< Base type of this Column instance.
   typedef ColumnTrait_<MT>            ResultType;     //!< Result type for expression template evaluations.
   typedef TransposeType_<ResultType>  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef ElementType_<MT>            ElementType;    //!< Type of the column elements.
   typedef SIMDTrait_<ElementType>     SIMDType;       //!< SIMD type of the column elements.
   typedef ReturnType_<MT>             ReturnType;     //!< Return type for expression template evaluations
   typedef const Column&               CompositeType;  //!< Data type for composite expression templates.

   //! Reference to a constant column value.
   typedef ConstReference_<MT>  ConstReference;

   //! Reference to a non-constant column value.
   typedef If_< IsConst<MT>, ConstReference, Reference_<MT> >  Reference;

   //! Pointer to a constant column value.
   typedef const ElementType*  ConstPointer;

   //! Pointer to a non-constant column value.
   typedef If_< Or< IsConst<MT>, Not< HasMutableDataAccess<MT> > >, ConstPointer, ElementType* >  Pointer;

   //! Iterator over constant elements.
   typedef ConstIterator_<MT>  ConstIterator;

   //! Iterator over non-constant elements.
   typedef If_< IsConst<MT>, ConstIterator, Iterator_<MT> >  Iterator;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = MT::simdEnabled };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = MT::smpAssignable };
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline Column( MT& matrix, size_t index );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Reference      at( size_t index );
   inline ConstReference at( size_t index ) const;
   inline Pointer        data  () noexcept;
   inline ConstPointer   data  () const noexcept;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   inline Column& operator=( const ElementType& rhs );
   inline Column& operator=( initializer_list<ElementType> list );
   inline Column& operator=( const Column& rhs );

   template< typename VT > inline Column& operator= ( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator+=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator-=( const Vector<VT,false>& rhs );
   template< typename VT > inline Column& operator*=( const DenseVector<VT,false>&  rhs );
   template< typename VT > inline Column& operator*=( const SparseVector<VT,false>& rhs );
   template< typename VT > inline Column& operator/=( const DenseVector<VT,false>&  rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator*=( Other rhs );

   template< typename Other >
   inline EnableIf_< IsNumeric<Other>, Column >& operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t  size() const noexcept;
                              inline size_t  capacity() const noexcept;
                              inline size_t  nonZeros() const;
                              inline void    reset();
   template< typename Other > inline Column& scale( const Other& scalar );
   //@}
   //**********************************************************************************************

 private:
   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedAddAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDAdd< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedSubAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDSub< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedMultAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDMult< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT >
   struct VectorizedDivAssign {
      enum : bool { value = useOptimizedKernels &&
                            simdEnabled && VT::simdEnabled &&
                            AreSIMDCombinable< ElementType, ElementType_<VT> >::value &&
                            HasSIMDDiv< ElementType, ElementType_<VT> >::value };
   };
   //**********************************************************************************************

   //**SIMD properties*****************************************************************************
   //! The number of elements packed within a single SIMD element.
   enum : size_t { SIMDSIZE = SIMDTrait<ElementType>::size };
   //**********************************************************************************************

 public:
   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other >
   inline bool canAlias( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool canAlias( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   template< typename Other >
   inline bool isAliased( const Other* alias ) const noexcept;

   template< typename MT2, bool SO2, bool SF2 >
   inline bool isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept;

   inline bool isAligned   () const noexcept;
   inline bool canSMPAssign() const noexcept;

   BLAZE_ALWAYS_INLINE SIMDType load ( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loada( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loadu( size_t index ) const noexcept;

   BLAZE_ALWAYS_INLINE void store ( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storea( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storeu( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void stream( size_t index, const SIMDType& value ) noexcept;

   template< typename VT >
   inline DisableIf_< VectorizedAssign<VT> > assign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedAssign<VT> > assign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void assign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedAddAssign<VT> > addAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedAddAssign<VT> > addAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void addAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedSubAssign<VT> > subAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedSubAssign<VT> > subAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void subAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedMultAssign<VT> > multAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedMultAssign<VT> > multAssign( const DenseVector<VT,false>& rhs );

   template< typename VT > inline void multAssign( const SparseVector<VT,false>& rhs );

   template< typename VT >
   inline DisableIf_< VectorizedDivAssign<VT> > divAssign( const DenseVector<VT,false>& rhs );

   template< typename VT >
   inline EnableIf_< VectorizedDivAssign<VT> > divAssign( const DenseVector<VT,false>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   Operand      matrix_;  //!< The dense matrix containing the column.
   const size_t col_;     //!< The index of the column in the matrix.
   //@}
   //**********************************************************************************************

   //**Friend declarations*************************************************************************
   template< typename MT2, bool SO2, bool DF2, bool SF2 > friend class Column;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isIntact( const Column<MT2,SO2,DF2,SF2>& column ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend bool isSame( const Column<MT2,SO2,DF2,SF2>& a, const Column<MT2,SO2,DF2,SF2>& b ) noexcept;

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryAddAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool trySubAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryMultAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2, typename VT >
   friend bool tryDivAssign( const Column<MT2,SO2,DF2,SF2>& lhs, const Vector<VT,false>& rhs, size_t index );

   template< typename MT2, bool SO2, bool DF2, bool SF2 >
   friend DerestrictTrait_< Column<MT2,SO2,DF2,SF2> > derestrict( Column<MT2,SO2,DF2,SF2>& column );
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_BE_SYMMETRIC_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_TRANSEXPR_TYPE   ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE     ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE   ( MT );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The constructor for Column.
//
// \param matrix The matrix containing the column.
// \param index The index of the column.
// \exception std::invalid_argument Invalid column access index.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,true>::Column( MT& matrix, size_t index )
   : matrix_( matrix )  // The dense matrix containing the column
   , col_   ( index  )  // The index of the column in the matrix
{
   if( matrix_.columns() <= index ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid column access index" );
   }
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::Reference
   Column<MT,false,true,true>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return matrix_(col_,index);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstReference
   Column<MT,false,true,true>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid column access index" );
   return const_cast<const MT&>( matrix_ )(index,col_);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::Reference
   Column<MT,false,true,true>::at( size_t index )
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Checked access to the column elements.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid column access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstReference
   Column<MT,false,true,true>::at( size_t index ) const
{
   if( index >= size() ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
   }
   return (*this)[index];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::Pointer Column<MT,false,true,true>::data() noexcept
{
   return matrix_.data( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Low-level data access to the column elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the dense column. Note that in case
// of a row-major matrix you can NOT assume that the column elements lie adjacent to each other!
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstPointer
   Column<MT,false,true,true>::data() const noexcept
{
   return matrix_.data( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::Iterator Column<MT,false,true,true>::begin()
{
   return matrix_.begin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstIterator Column<MT,false,true,true>::begin() const
{
   return matrix_.cbegin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the column.
//
// \return Iterator to the first element of the column.
//
// This function returns an iterator to the first element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstIterator Column<MT,false,true,true>::cbegin() const
{
   return matrix_.cbegin( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::Iterator Column<MT,false,true,true>::end()
{
   return matrix_.end( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstIterator Column<MT,false,true,true>::end() const
{
   return matrix_.cend( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the column.
//
// \return Iterator just past the last element of the column.
//
// This function returns an iterator just past the last element of the column.
*/
template< typename MT >  // Type of the dense matrix
inline typename Column<MT,false,true,true>::ConstIterator Column<MT,false,true,true>::cend() const
{
   return matrix_.cend( col_ );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Homogenous assignment to all column elements.
//
// \param rhs Scalar value to be assigned to all column elements.
// \return Reference to the assigned column.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,true>& Column<MT,false,true,true>::operator=( const ElementType& rhs )
{
   const size_t jbegin( ( IsUpper<MT>::value )
                        ?( ( IsUniUpper<MT>::value || IsStrictlyUpper<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t jend  ( ( IsLower<MT>::value )
                        ?( ( IsUniLower<MT>::value || IsStrictlyLower<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t j=jbegin; j<jend; ++j )
      matrix_(col_,j) = rhs;

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief List assignment to all column elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to column.
//
// This assignment operator offers the option to directly assign to all elements of the dense
// column by means of an initializer list. The column elements are assigned the values from the
// given initializer list. Missing values are reset to their default state. Note that in case
// the size of the initializer list exceeds the size of the column, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator=( initializer_list<ElementType> list )
{
   if( list.size() > size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to column" );
   }

   std::fill( std::copy( list.begin(), list.end(), begin() ), end(), ElementType() );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Copy assignment operator for Column.
//
// \param rhs Dense column to be copied.
// \return Reference to the assigned column.
// \exception std::invalid_argument Column sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two columns don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
inline Column<MT,false,true,true>& Column<MT,false,true,true>::operator=( const Column& rhs )
{
   if( &rhs == this ) return *this;

   if( size() != rhs.size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Column sizes do not match" );
   }

   if( !tryAssign( matrix_, rhs, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, rhs );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be assigned.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpAssign( left, tmp );
   }
   else {
      if( IsSparseVector<VT>::value )
         reset();
      smpAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator+=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryAddAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpAddAssign( left, tmp );
   }
   else {
      smpAddAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown. Also, if the underlying matrix \a MT is a lower or upper triangular matrix and the
// assignment would violate its lower or upper property, respectively, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator-=( const Vector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !trySubAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpSubAssign( left, tmp );
   }
   else {
      smpSubAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a dense vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator*=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryMultAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpMultAssign( left, tmp );
   }
   else {
      smpMultAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a sparse vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side sparse vector to be multiplied with the dense column.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator*=( const SparseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE  ( ResultType );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   const ResultType right( *this * (~rhs) );

   if( !tryAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   smpAssign( left, right );

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense vector (\f$ \vec{a}/=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector divisor.
// \return Reference to the assigned column.
// \exception std::invalid_argument Vector sizes do not match.
// \exception std::invalid_argument Invalid assignment to restricted matrix.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline Column<MT,false,true,true>&
   Column<MT,false,true,true>::operator/=( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE ( ResultType_<VT> );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_<VT> );

   if( size() != (~rhs).size() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   typedef If_< IsRestricted<MT>, CompositeType_<VT>, const VT& >  Right;
   Right right( ~rhs );

   if( !tryDivAssign( matrix_, right, 0UL, col_ ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to restricted matrix" );
   }

   DerestrictTrait_<This> left( derestrict( *this ) );

   if( IsReference<Right>::value && right.canAlias( &matrix_ ) ) {
      const ResultType_<VT> tmp( right );
      smpDivAssign( left, tmp );
   }
   else {
      smpDivAssign( left, right );
   }

   BLAZE_INTERNAL_ASSERT( isIntact( matrix_ ), "Invariant violation detected" );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication between a dense column and
//        a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,false,true,true> >&
   Column<MT,false,true,true>::operator*=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   return operator=( (*this) * rhs );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a dense column by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the vector.
//
// This operator cannot be used for columns on lower or upper unitriangular matrices. The attempt
// to scale such a row results in a compilation error!
//
// \note A division by zero is only checked by an user assert.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the right-hand side scalar
inline EnableIf_< IsNumeric<Other>, Column<MT,false,true,true> >&
   Column<MT,false,true,true>::operator/=( Other rhs )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   return operator=( (*this) / rhs );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current size/dimension of the column.
//
// \return The size of the column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,true>::size() const noexcept
{
   return matrix_.rows();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the maximum capacity of the dense column.
//
// \return The capacity of the dense column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,true>::capacity() const noexcept
{
   return matrix_.capacity( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the number of non-zero elements in the column.
//
// \return The number of non-zero elements in the column.
//
// Note that the number of non-zero elements is always less than or equal to the current number
// of rows of the matrix containing the column.
*/
template< typename MT >  // Type of the dense matrix
inline size_t Column<MT,false,true,true>::nonZeros() const
{
   return matrix_.nonZeros( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT >  // Type of the dense matrix
inline void Column<MT,false,true,true>::reset()
{
   matrix_.reset( col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling of the column by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the column scaling.
// \return Reference to the dense column.
//
// This function scales all elements of the column by the given scalar value \a scalar. Note that
// the function cannot be used to scale a column on a lower or upper unitriangular matrix. The
// attempt to scale such a row results in a compile time error!
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the scalar value
inline Column<MT,false,true,true>& Column<MT,false,true,true>::scale( const Other& scalar )
{
   BLAZE_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE( MT );

   const size_t jbegin( ( IsUpper<MT>::value )
                        ?( ( IsStrictlyUpper<MT>::value )
                           ?( col_+1UL )
                           :( col_ ) )
                        :( 0UL ) );
   const size_t jend  ( ( IsLower<MT>::value )
                        ?( ( IsStrictlyLower<MT>::value )
                           ?( col_ )
                           :( col_+1UL ) )
                        :( size() ) );

   for( size_t j=jbegin; j<jend; ++j ) {
      matrix_(col_,j) *= scalar;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In
// contrast to the isAliased() function this function is allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,false,true,true>::canAlias( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can alias with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address can alias with the dense column. In
// contrast to the isAliased() function this function is allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT >  // Type of the dense matrix
template< typename MT2   // Data type of the foreign dense column
        , bool SO2       // Storage order of the foreign dense column
        , bool SF2 >     // Symmetry flag of the foreign dense column
inline bool Column<MT,false,true,true>::canAlias( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In
// contrast to the canAlias() function this function is not allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT >     // Type of the dense matrix
template< typename Other >  // Data type of the foreign expression
inline bool Column<MT,false,true,true>::isAliased( const Other* alias ) const noexcept
{
   return matrix_.isAliased( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is aliased with the given dense column \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this dense column, \a false if not.
//
// This function returns whether the given address is aliased with the dense column. In
// contrast to the canAlias() function this function is not allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename MT >  // Type of the dense matrix
template< typename MT2   // Data type of the foreign dense column
        , bool SO2       // Storage order of the foreign dense column
        , bool SF2 >     // Symmetry flag of the foreign dense column
inline bool Column<MT,false,true,true>::isAliased( const Column<MT2,SO2,true,SF2>* alias ) const noexcept
{
   return matrix_.isAliased( &alias->matrix_ ) && ( col_ == alias->col_ );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column is properly aligned in memory.
//
// \return \a true in case the dense column is aligned, \a false if not.
//
// This function returns whether the dense column is guaranteed to be properly aligned in memory,
// i.e. whether the beginning and the end of the dense column are guaranteed to conform to the
// alignment restrictions of the element type \a Type.
*/
template< typename MT >  // Type of the dense matrix
inline bool Column<MT,false,true,true>::isAligned() const noexcept
{
   return matrix_.isAligned();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the dense column can be used in SMP assignments.
//
// \return \a true in case the dense column can be used in SMP assignments, \a false if not.
//
// This function returns whether the dense column can be used in SMP assignments. In contrast
// to the \a smpAssignable member enumeration, which is based solely on compile time information,
// this function additionally provides runtime information (as for instance the current size of
// the dense column).
*/
template< typename MT >  // Type of the dense matrix
inline bool Column<MT,false,true,true>::canSMPAssign() const noexcept
{
   return ( size() > SMP_DVECASSIGN_THRESHOLD );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs a load of a specific SIMD element of the dense column. The index must
// be smaller than the number of matrix rows. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE typename Column<MT,false,true,true>::SIMDType
   Column<MT,false,true,true>::load( size_t index ) const noexcept
{
   return matrix_.load( col_, index );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs an aligned load of a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE typename Column<MT,false,true,true>::SIMDType
   Column<MT,false,true,true>::loada( size_t index ) const noexcept
{
   return matrix_.loada( col_, index );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Unaligned load of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \return The loaded SIMD element.
//
// This function performs an unaligned load of a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE typename Column<MT,false,true,true>::SIMDType
   Column<MT,false,true,true>::loadu( size_t index ) const noexcept
{
   return matrix_.loadu( col_, index );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs a store a specific SIMD element of the dense column. The index must
// be smaller than the number of matrix rows. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE void
   Column<MT,false,true,true>::store( size_t index, const SIMDType& value ) noexcept
{
   matrix_.store( col_, index, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned store a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE void
   Column<MT,false,true,true>::storea( size_t index, const SIMDType& value ) noexcept
{
   matrix_.storea( col_, index, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Unaligned store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an unaligned store a specific SIMD element of the dense column. The
// index must be smaller than the number of matrix rows. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE void
   Column<MT,false,true,true>::storeu( size_t index, const SIMDType& value ) noexcept
{
   matrix_.storeu( col_, index, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Aligned, non-temporal store of a SIMD element of the dense column.
//
// \param index Access index. The index must be smaller than the number of matrix rows.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned, non-temporal store a specific SIMD element of the dense
// column. The index must be smaller than the number of matrix rows. This function must \b NOT
// be called explicitly! It is used internally for the performance optimized evaluation of
// expression templates. Calling this function explicitly might result in erroneous results
// and/or in compilation errors.
*/
template< typename MT >  // Type of the dense matrix
BLAZE_ALWAYS_INLINE void
   Column<MT,false,true,true>::stream( size_t index, const SIMDType& value ) noexcept
{
   matrix_.stream( col_, index, value );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedAssign<VT> >
   Column<MT,false,true,true>::assign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t jpos( (~rhs).size() & size_t(-2) );
   for( size_t j=0UL; j<jpos; j+=2UL ) {
      matrix_(col_,j    ) = (~rhs)[j    ];
      matrix_(col_,j+1UL) = (~rhs)[j+1UL];
   }
   if( jpos < (~rhs).size() )
      matrix_(col_,jpos) = (~rhs)[jpos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedAssign<VT> >
   Column<MT,false,true,true>::assign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t columns( size() );

   const size_t jpos( ( remainder )?( columns & size_t(-SIMDSIZE) ):( columns ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( columns - ( columns % (SIMDSIZE) ) ) == jpos, "Invalid end calculation" );

   size_t j( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   if( useStreaming && columns > ( cacheSize/( sizeof(ElementType) * 3UL ) ) && !(~rhs).isAliased( this ) )
   {
      for( ; j<jpos; j+=SIMDSIZE ) {
         left.stream( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; remainder && j<columns; ++j ) {
         *left = *right; ++left; ++right;
      }
   }
   else
   {
      for( ; (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL ) {
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; j<jpos; j+=SIMDSIZE ) {
         left.store( right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      }
      for( ; remainder && j<columns; ++j ) {
         *left = *right; ++left; ++right;
      }
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,true>::assign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(col_,element->index()) = element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedAddAssign<VT> >
   Column<MT,false,true,true>::addAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t jpos( (~rhs).size() & size_t(-2) );
   for( size_t j=0UL; j<jpos; j+=2UL ) {
      matrix_(col_,j    ) += (~rhs)[j    ];
      matrix_(col_,j+1UL) += (~rhs)[j+1UL];
   }
   if( jpos < (~rhs).size() )
      matrix_(col_,jpos) += (~rhs)[jpos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedAddAssign<VT> >
   Column<MT,false,true,true>::addAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t columns( size() );

   const size_t jpos( ( remainder )?( columns & size_t(-SIMDSIZE) ):( columns ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( columns - ( columns % (SIMDSIZE) ) ) == jpos, "Invalid end calculation" );

   size_t j( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL ) {
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; j<jpos; j+=SIMDSIZE ) {
      left.store( left.load() + right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && j<columns; ++j ) {
      *left += *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,true>::addAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(col_,element->index()) += element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedSubAssign<VT> >
   Column<MT,false,true,true>::subAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t jpos( (~rhs).size() & size_t(-2) );
   for( size_t j=0UL; j<jpos; j+=2UL ) {
      matrix_(col_,j    ) -= (~rhs)[j    ];
      matrix_(col_,j+1UL) -= (~rhs)[j+1UL];
   }
   if( jpos < (~rhs).size() )
      matrix_(col_,jpos) -= (~rhs)[jpos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedSubAssign<VT> >
   Column<MT,false,true,true>::subAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t columns( size() );

   const size_t jpos( ( remainder )?( columns & size_t(-SIMDSIZE) ):( columns ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( columns - ( columns % (SIMDSIZE) ) ) == jpos, "Invalid end calculation" );

   size_t j( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL ) {
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; j<jpos; j+=SIMDSIZE ) {
      left.store( left.load() - right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && j<columns; ++j ) {
      *left -= *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,true>::subAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(col_,element->index()) -= element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedMultAssign<VT> >
   Column<MT,false,true,true>::multAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t jpos( (~rhs).size() & size_t(-2) );
   for( size_t j=0UL; j<jpos; j+=2UL ) {
      matrix_(col_,j    ) *= (~rhs)[j    ];
      matrix_(col_,j+1UL) *= (~rhs)[j+1UL];
   }
   if( jpos < (~rhs).size() )
      matrix_(col_,jpos) *= (~rhs)[jpos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedMultAssign<VT> >
   Column<MT,false,true,true>::multAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const bool remainder( !IsPadded<MT>::value || !IsPadded<VT>::value );
   const size_t columns( size() );

   const size_t jpos( ( remainder )?( columns & size_t(-SIMDSIZE) ):( columns ) );
   BLAZE_INTERNAL_ASSERT( !remainder || ( columns - ( columns % (SIMDSIZE) ) ) == jpos, "Invalid end calculation" );

   size_t j( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL ) {
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; j<jpos; j+=SIMDSIZE ) {
      left.store( left.load() * right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; remainder && j<columns; ++j ) {
      *left *= *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the multiplication assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void Column<MT,false,true,true>::multAssign( const SparseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const ResultType tmp( serial( *this ) );

   reset();

   for( ConstIterator_<VT> element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      matrix_(col_,element->index()) = tmp[element->index()] * element->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline DisableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedDivAssign<VT> >
   Column<MT,false,true,true>::divAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t jpos( (~rhs).size() & size_t(-2) );
   for( size_t j=0UL; j<jpos; j+=2UL ) {
      matrix_(col_,j    ) /= (~rhs)[j    ];
      matrix_(col_,j+1UL) /= (~rhs)[j+1UL];
   }
   if( jpos < (~rhs).size() )
      matrix_(col_,jpos) /= (~rhs)[jpos];
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief SIMD optimized implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the dense matrix
template< typename VT >  // Type of the right-hand side dense vector
inline EnableIf_< typename Column<MT,false,true,true>::BLAZE_TEMPLATE VectorizedDivAssign<VT> >
   Column<MT,false,true,true>::divAssign( const DenseVector<VT,false>& rhs )
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( ElementType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const size_t columns( size() );

   const size_t jpos( columns & size_t(-SIMDSIZE) );
   BLAZE_INTERNAL_ASSERT( ( columns - ( columns % (SIMDSIZE) ) ) == jpos, "Invalid end calculation" );

   size_t j( 0UL );
   Iterator left( begin() );
   ConstIterator_<VT> right( (~rhs).begin() );

   for( ; (j+SIMDSIZE*3UL) < jpos; j+=SIMDSIZE*4UL ) {
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; j<jpos; j+=SIMDSIZE ) {
      left.store( left.load() / right.load() ); left += SIMDSIZE; right += SIMDSIZE;
   }
   for( ; j<columns; ++j ) {
      *left /= *right; ++left; ++right;
   }
}
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
