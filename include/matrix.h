/*
 * Copyright (C) 2019 Dieter J Kybelksties
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * @date: 2019-04-30
 * @author: Dieter J Kybelksties
 */

#if !defined(NS_UTIL_MATRIX_H_INCLUDED)
#define NS_UTIL_MATRIX_H_INCLUDED

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <utility>
#include <initializer_list>
#include "stringutil.h"

namespace util
{

    struct matrix_interface
    {
        /**
         * Retrieve the horizontal extent of the matrix.
         * @return x-dimension
         */
        virtual size_t sizeX() const = 0;

        /**
         * Retrieve the vertical extent of the matrix.
         * @return y-dimension
         */
        virtual size_t sizeY() const = 0;

        /**
         * Check whether coordinates (x,y) are within the matrix.
         * @return true if so, false otherwise
         */
        bool withinBounds(size_t x, size_t y) const
        {
            return x < sizeX() && y < sizeY();
        }

        /**
         * Check whether this is a square matrix.
         * @return true if so, false otherwise
         */
        bool isSquare() const
        {
            return sizeX() == sizeY();
        }

        /**
         * Check whether matrix dimensions of the right-hand side are compatible
         * for adding to this matrix (x- and y- dimensions need to be same as in
         * this matrix)
         * @param rhs the right-hand-side matrix.
         * @return true if so, false otherwise
         */
        bool isAddCompatible(const matrix_interface& rhs) const
        {
            return sizeX() == rhs.sizeX() && sizeY() == rhs.sizeY();
        }

        /**
         * Check whether matrix dimensions of the right-hand side are compatible
         * for matrix multiplication with this matrix.
         * @param rhs the right-hand-side matrix.
         * @return true if so, false otherwise
         */
        bool isMultCompatible(const matrix_interface& rhs) const
        {
            return sizeX() == rhs.sizeY();
        }

        /**
         * Check whether this matrix is square and the rhs matrix has the same
         * number of rows for the solve algorithm to work.
         * @param rhs the right-hand-side matrix.
         * @return true if so, false otherwise
         */
        bool isSolveCompatible(const matrix_interface& rhs) const
        {
            return isSquare() && sizeY() == rhs.sizeY();
        }

        /**
         * Check whether this is a horizontal vector-shaped matrix.
         * @return true if so, false otherwise
         */
        bool isHVector() const
        {
            return (1 == sizeY());
        }

        /**
         * Check whether this is a vertical vector-shaped matrix.
         * @return true if so, false otherwise
         */
        bool isVVector() const
        {
            return (1 == sizeX());
        }

        /**
         * Check whether this is a diagonal matrix.
         * @return true if so, false otherwise
         */
        virtual bool isDiagonal() const = 0;

        /**
         * Check whether this is a scalar matrix.
         * @return true if so, false otherwise
         */
        virtual bool isScalar() const = 0;

        /**
         * Check whether this is a unit matrix.
         * @return true if so, false otherwise
         */
        virtual bool isUnit() const = 0;

        /**
         * Check whether this is a upper triangular matrix.
         * @return true if so, false otherwise
         */
        virtual bool isUpperTriangular() const = 0;

        /**
         * Check whether this is a lower triangular matrix.
         * @return true if so, false otherwise
         */
        virtual bool isLowerTriangular() const = 0;

        /**
         * Check whether this is a symmetric matrix.
         * @return true if so, false otherwise
         */
        virtual bool isSymmetric() const = 0;

        /**
         * Check whether this is a skew-symmetric matrix.
         * @return true if so, false otherwise
         */
        virtual bool isSkewSymmetric() const = 0;
    };

    class matrix_error : public std::logic_error
    {
    public:

        matrix_error(const std::string& what_arg)
        : logic_error(what_arg)
        {
        }
    };

    template<bool enable = false >
    void checkBounds(const matrix_interface& lhs,
                     size_t x,
                     size_t y,
                     const std::string& location);

    template<>
    void checkBounds<false> (const matrix_interface& lhs,
                             size_t x,
                             size_t y,
                             const std::string& location);

    /**
     * matrix template
     *
     * Note: This matrix template class defines majority of the matrix
     *  operations as overloaded operators or methods. It is assumed that
     * users of this class is familiar with matrix algebra. We have not
     * defined any specialization of this template here, so all the instances
     * of matrix will be created implicitly by the compiler. The data types
     * tested with this class are float, double, long double, complex<float>,
     * complex<double> and complex<long double>. Note that this class is not
     * optimized for performance.
     */
    template <typename T = long double, bool enableBoundsCheck = false >
    class matrix : public matrix_interface
    {
    private:
        typedef std::vector<T> row_t;
        typedef std::vector<row_t> mat_t;
        /**
         * Data-container.
         */
        mat_t m_;

        mat_t& initializeData(size_t xDim = 0, size_t yDim = 0)
        {
            m_.clear();
            if (xDim == 0)
                xDim = 1;
            if (yDim == 0)
                yDim = xDim;
            row_t newRow = row_t(xDim);
            for (size_t y = 0; y < yDim; y++)
                m_.push_back(newRow);
            return m_;
        }


    public:

        matrix(const matrix& rhs) = default;

        /**
         * Default constructor.
         * @param xDim x-dimension: if equal 0 than set to 1
         * @param yDim y-dimension: if equal 0 than make matrix square
         */
        explicit matrix(size_t xDim = 0,
                        size_t yDim = 0,
                        std::initializer_list<T> l = std::initializer_list<T>())
        {
            initializeData(xDim, yDim);
            size_t x = 0;
            size_t y = 0;
            auto it = l.begin();
            size_t count = 0;
            size_t maxCount = sizeX() * sizeY();
            while (it != l.end() && count < maxCount)
            {
                (*this)(x, y) = *it;
                x++;
                if (x == sizeX())
                {
                    x = 0;
                    y++;
                }
                count++;
                it++;
            }

        }

        matrix(matrix&& rhs)
        : m_(rhs.m_)
        {
            rhs.initializeData();
        }
        ~matrix() = default;
        matrix& operator=(const matrix& rhs) = default;

        matrix& operator=(matrix&& rhs)
        {
            if (this != &rhs)
            {
                m_ = rhs.m_;
                rhs.initializeData();
            }
            return *this;
        }

        static matrix<T, enableBoundsCheck> diag(std::initializer_list<T> l)
        {
            matrix<T, enableBoundsCheck> reval(l.size(), l.size());
            auto it = l.begin();
            for (size_t i = 0; i < l.size(); i++)
                reval(i, i) = *it++;
            return reval;
        }

        static matrix<T, enableBoundsCheck> scalar(size_t dim, const T& c = T(1.0))
        {
            matrix<T, enableBoundsCheck> reval(dim);
            for (size_t i = 0; i < reval.sizeX(); i++)
                reval(i, i) = c;
            return reval;
        }

        /**
         * Create a horizontal vector-matrix (size in Y-dimension is 1)
         * @param l list of values, determines also the size of the matrix
         * @return l.size() x 1 - matrix with values from the list
         */
        static matrix<T, enableBoundsCheck> hvect(std::initializer_list<T> l)
        {
            matrix<T, enableBoundsCheck> reval(l.size(), 1);
            auto it = l.begin();
            for (size_t i = 0; i < l.size(); i++)
                reval(i, 0) = *it++;
            return reval;
        }

        /**
         * Create a vertical vector-matrix (size in X-dimension is 1)
         * @param l list of values, determines also the size of the matrix
         * @return 1 x l.size() - matrix with values from the list
         */
        static matrix<T, enableBoundsCheck> vvect(std::initializer_list<T> l)
        {
            matrix<T, enableBoundsCheck> reval(1, l.size());
            auto it = l.begin();
            for (size_t i = 0; i < l.size(); i++)
                reval(0, i) = *it++;
            return reval;
        }

        /**
         * Retrieve the horizontal extent of the matrix.
         * @return x-dimension
         */
        size_t sizeX() const
        {
            return sizeY() == 0 ? 0 : m_[0].size();
        }

        /**
         * Retrieve the vertical extent of the matrix.
         * @return y-dimension
         */
        size_t sizeY() const
        {
            return m_.size();
        }

        static bool checkCompatibleSizes(const matrix<T, enableBoundsCheck>& lhs,
                                         const matrix<T, enableBoundsCheck>& rhs,
                                         const std::string& operation,
                                         const std::string& location)
        {
            if (operation == "+" || operation == "-")
            {
                if (!lhs.isAddCompatible(rhs))
                    throw matrix_error(location +
                                       ": matrix-size lhs (" +
                                       asString(lhs.sizeX()) +
                                       "," +
                                       asString(lhs.sizeY()) +
                                       ") is not equal matrix-size rhs(" +
                                       asString(rhs.sizeX()) +
                                       "," +
                                       asString(rhs.sizeY()) +
                                       ").");
            }
            else if (operation == "*")
            {
                if (lhs.sizeX() != rhs.sizeY())
                    throw matrix_error(location +
                                       ": matrix-x-dimension lhs " +
                                       asString(lhs.sizeX()) +
                                       " is not equal matrix-y-dimension rhs" +
                                       asString(rhs.sizeY()) +
                                       ".");
            }
            else if (operation == "solve")
            {
                if (lhs.sizeY() != rhs.sizeY())
                    throw matrix_error(location +
                                       ": matrix-y-dimension lhs " +
                                       asString(lhs.sizeY()) +
                                       " is not equal matrix-y-dimension rhs" +
                                       asString(rhs.sizeY()) +
                                       ".");
            }
            return true;
        }

        static bool checkNotZero(T c, const std::string& location)
        {
            if (c == (T) 0)
                throw matrix_error(location +
                                   ": scalar " +
                                   asString(c) +
                                   "must not be 0(Zero).");
            return true;
        }

        static bool checkSquare(const matrix<T, enableBoundsCheck>& lhs, const std::string& location)
        {
            if (!lhs.isSquare())
                throw matrix_error(location +
                                   ": operation only defined for square matrices.");
            return true;
        }

        /**
         * Subscript operator to get/set individual elements.
         *
         * @param x x-coordinate
         * @param y y coordinate
         * @return reference to the value of the element at (x,y)
         */
        T& operator()(size_t x, size_t y)
        {
            //if enableBoundsCheck==false, this will be an empty function
            checkBounds<enableBoundsCheck>(*this,
                                           x,
                                           y,
                                           "T& matrix<T,enableBoundsCheck>::operator()");
            return m_[y][x];
        }

        /**
         * Subscript operator to get individual elements.
         *
         * @param x x-coordinate
         * @param y y coordinate
         * @return the value of the element at (x,y)
         */
        T operator()(size_t x, size_t y) const
        {
            //if enableBoundsCheck==false, this will be an empty function
            checkBounds<enableBoundsCheck>(*this,
                                           x,
                                           y,
                                           "T matrix<T,enableBoundsCheck>::operator() const");
            return m_[y][x];
        }

        /**
         * Retrieve the whole row(y) if in bounds.
         * @param y row index
         * @return reference to the data in row y
         */
        row_t& row(size_t y)
        {
            checkBounds<enableBoundsCheck>(*this,
                                           0,
                                           y,
                                           "row_t& matrix<T,enableBoundsCheck>::row(y)");
            return m_[y];
        }

        /**
         * Retrieve the whole row(y) if in bounds.
         * @param y row index
         * @return the data in row y by value
         */
        row_t row(size_t y) const
        {
            checkBounds<enableBoundsCheck>(*this,
                                           0,
                                           y,
                                           "row_t& matrix<T,enableBoundsCheck>::row(y)");
            return m_[y];
        }

        /**
         * Unary + operator.
         */
        matrix& operator+()
        {
            return *this;
        }

        /**
         * Unary negation operator.
         * @param rhs right-hand-side matrix
         */
        matrix<T, enableBoundsCheck> operator-(const matrix<T, enableBoundsCheck>& rhs)
        {
            matrix<T, enableBoundsCheck> temp(rhs);

            for (size_t y = 0; y < temp.sizeY(); y++)
                for (size_t x = 0; x < temp.sizeX(); x++)
                    temp(x, y) = -temp(x, y);

            return temp;
        }

        /**
         * Global matrix addition operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return the sum of the two matrices
         */
        friend matrix<T, enableBoundsCheck> operator+(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {
            checkCompatibleSizes(lhs, rhs, "operator+(lhs,rhs)", "+");
            matrix<T, enableBoundsCheck> reval = lhs;
            for (size_t y = 0; y < reval.sizeY(); y++)
                for (size_t x = 0; x < reval.sizeX(); x++)
                    reval(x, y) += rhs(x, y);
            return reval;
        }

        /**
         * Combined addition and assignment operator.
         * @param rhs right-hand-side matrix
         * @return the sum of this with the rhs
         */
        matrix<T, enableBoundsCheck>& operator+=(const matrix<T, enableBoundsCheck>& rhs)
        {
            checkCompatibleSizes(*this, rhs, "operator+=(rhs)", "+");
            *this = *this+rhs;
            return *this;
        }

        /**
         * Global matrix subtraction operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return the result of lhs-rhs
         */
        friend matrix<T, enableBoundsCheck> operator-(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {
            checkCompatibleSizes(lhs, rhs, "operator-(lhs,rhs)", "+");
            matrix<T, enableBoundsCheck> reval = lhs;
            for (size_t y = 0; y < reval.sizeY(); y++)
                for (size_t x = 0; x < reval.sizeX(); x++)
                    reval(x, y, false) -= rhs(x, y, false);
            return reval;
        }

        /**
         * Combined subtraction and assignment operator.
         * @param rhs right-hand-side matrix
         * @return the result of *this - rhs
         */
        matrix<T, enableBoundsCheck>& operator-=(const matrix<T, enableBoundsCheck>& rhs)
        {
            checkCompatibleSizes(*this, rhs, "operator-=(rhs)", "-");
            *this = *this-rhs;
            return *this;
        }

        /**
         * Global scalar multiplication operator.
         * @param lhs left-hand-side matrix
         * @param c right-hand-side constant scalar value
         * @return the result of lhs*c
         */
        friend matrix<T, enableBoundsCheck> operator*(const matrix<T, enableBoundsCheck>& lhs, const T& c)
        {
            matrix<T, enableBoundsCheck> reval = lhs;
            for (size_t y = 0; y < reval.sizeY(); y++)
                for (size_t x = 0; x < reval.sizeX(); x++)
                    reval(x, y) *= c;
            return reval;
        }

        /**
         * Global scalar multiplication operator.
         * @param c left-hand-side constant scalar value
         * @param rhs left-hand-side matrix
         * @return the result of c*rhs
         */
        friend matrix<T, enableBoundsCheck> operator*(const T& c, const matrix<T, enableBoundsCheck>& rhs)
        {
            matrix<T, enableBoundsCheck> reval = rhs;
            for (size_t y = 0; y < reval.sizeY(); y++)
                for (size_t x = 0; x < reval.sizeX(); x++)
                    reval(x, y, false) *= c;
            return reval;
        }

        /**
         * Combined scalar multiplication and assignment operator.
         * @param c scalar
         * @return the result of *this * c
         */
        matrix<T, enableBoundsCheck>& operator*=(const T& c)
        {
            *this = *this * c;
            return *this;
        }

        /**
         * Matrix multiplication operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return the product lhs*rhs
         */
        friend matrix<T, enableBoundsCheck> operator*(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {
            checkCompatibleSizes(lhs, rhs, "operator*(lhs,rhs)", "*");
            matrix<T, enableBoundsCheck> reval(rhs.sizeX(), lhs.sizeY());

            for (size_t y = 0; y < lhs.sizeY(); y++)
                for (size_t x = 0; x < rhs.sizeX(); x++)
                {
                    reval(y, x) = T(0);
                    for (size_t k = 0; k < lhs.sizeX(); k++)
                        reval(y, x) += lhs(k, y) * rhs(x, k);
                }

            return reval;
        }

        /**
         * Combined  multiplication and assignment operator.
         * @param rhs right-hand-side matrix
         * @return the result of *this * rhs
         */
        matrix<T, enableBoundsCheck>& operator*=(const matrix<T, enableBoundsCheck>& rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        /**
         * Global scalar division operator.
         * @param lhs left-hand-side matrix
         * @param c right-hand-side constant scalar value
         * @return the result of lhs/c
         */
        friend matrix<T, enableBoundsCheck> operator/(const matrix<T, enableBoundsCheck>& lhs, const T& c)
        {
            checkNotZero(c, "operator/(lhs,c)");
            matrix<T, enableBoundsCheck> reval = lhs;
            for (size_t y = 0; y < reval.sizeY(); y++)
                for (size_t x = 0; x < reval.sizeX(); x++)
                    reval(x, y) /= c;
            return reval;
        }

        /**
         * Global scalar divided by matrix operator.
         * @param rhs right-hand-side matrix
         * @param c left-hand-side constant scalar value
         * @return the result of c/rhs
         */
        friend matrix<T, enableBoundsCheck> operator/(const T& c, const matrix<T, enableBoundsCheck>& rhs)
        {
            return (!rhs * c);
        }

        /**
         * Global matrix divided by matrix operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return the result of lhs/rhs
         */
        friend matrix<T, enableBoundsCheck> operator/(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {
            return (lhs * !rhs);
        }

        /**
         * Combined scalar division and assignment operator.
         * @param c scalar
         * @return the result of *this * c
         */
        matrix<T, enableBoundsCheck>& operator/=(const T& c)
        {
            checkNotZero(c, "operator/=(lhs,c)");
            *this = *this / c;
            return *this;
        }

        /**
         * Power operator.
         * @param pow power
         * @return lhs ^ pow
         */
        friend matrix<T, enableBoundsCheck> operator^(const matrix<T, enableBoundsCheck>& lhs, const size_t& pow)
        {
            checkSquare(lhs, "operator^(lhs,pow)");
            matrix<T, enableBoundsCheck> reval(lhs);
            for (size_t i = 2; i <= pow; i++)
                reval = lhs * reval;

            return reval;
        }

        /**
         * Combined power and assignment operator.
         * @param pow power
         * @return *this ^ pow
         */
        matrix<T, enableBoundsCheck>& operator^=(const size_t& pow)
        {
            checkSquare(*this, "operator^=(pow)");
            *this = *this ^ pow;
            return *this;
        }

        /**
         * This operator is used to return the transposition of the matrix.
         * @param rhs right-hand-side matrix
         * @return rhs.transposed
         */
        friend matrix<T, enableBoundsCheck> operator~(const matrix<T, enableBoundsCheck>& rhs)
        {
            matrix<T, enableBoundsCheck> reval(rhs.sizeY(), rhs.sizeX());

            for (size_t y = 0; y < rhs.sizeY(); y++)
                for (size_t x = 0; x < rhs.sizeX(); x++)
                {
                    reval(y, x) = rhs(x, y);
                }
            return reval;
        }

        /**
         * Resize the matrix to new dimensions whilst preserving the values where
         * possible.
         * @param newXDim new x-dimension
         * @param newYdimnew y-dimension
         */
        void resize(size_t newXDim, size_t newYdim)
        {
            if (newXDim == 0)
                newXDim = sizeX();
            if (newYdim == 0)
                newYdim = sizeY();
            mat_t tmp = initializeData(newYdim, newXDim);
            for (size_t y = 0; y < std::min(sizeY(), newYdim); y++)
                std::copy(row(y).begin(),
                          row(y).begin()+(std::min(newXDim, sizeX())),
                          tmp[y].begin());
            std::swap(tmp, m_);
        }

        /**
         * This operator has been used to calculate inversion of matrix.
         * @param rhs right-hand-side matrix
         * @return lhs^(-1)
         */
        friend matrix<T, enableBoundsCheck> operator!(const matrix<T, enableBoundsCheck>& rhs)
        {
            matrix<T, enableBoundsCheck> reval = rhs;
            return reval.inv();
        }

        /**
         * Inversion function.
         *
         * @return the inverted non-singular square matrix if possible.
         */
        matrix<T, enableBoundsCheck> inv()
        {
            checkSquare(*this, "matrix<T,enableBoundsCheck>::inv()");
            size_t y, x, k;
            T pivotVal, a2;

            // initialize the return matrix as the unit-matrix of sizeX
            matrix<T, enableBoundsCheck> reval = matrix<T, enableBoundsCheck>::scalar(sizeX(), T(1.0));

            for (k = 0; k < sizeX(); k++)
            {
                // if we can not find a new pivot-element then the matrix is
                // singular and cannot be inverted
                int pivIdx = pivot(k);
                if (pivIdx == -1)
                    throw matrix_error("matrix<T,enableBoundsCheck>::operator!: Inversion of a singular matrix");

                // swap rows so that the pivot element is on the diagonal at k
                if (pivIdx != 0)
                {
                    std::swap(row(k), row(pivIdx));
                }
                // divide all values at the row by the pivot value and thus
                // ensuring that the value at (k,k) == 1
                pivotVal = (*this)(k, k);
                for (x = 0; x < sizeX(); x++)
                {
                    (*this)(x, k) /= pivotVal;
                    reval(x, k) /= pivotVal;
                }
                for (y = 0; y < sizeY(); y++)
                    if (y != k)
                    {
                        a2 = (*this)(k, y);
                        for (x = 0; x < sizeX(); x++)
                        {
                            (*this)(x, y) -= a2 * (*this)(x, k);
                            reval(x, y) -= a2 * reval(x, k);
                        }
                    }
            }
            return reval;
        }

        /**
         * Solve simultaneous equations.
         * @param v matrix of values. Can be seen as set of vertical vectors
         * @return matrix representing the solutions
         */
        matrix<T, enableBoundsCheck> solve(const matrix<T, enableBoundsCheck>& v) const
        {
            checkSquare(*this, "matrix<T,enableBoundsCheck>::solve(v)");
            checkCompatibleSizes(*this, v, "solve", "matrix<T,enableBoundsCheck>::solve(v)");
            size_t x, y, k;
            T a1;

            matrix<T, enableBoundsCheck> temp(*this);
            temp.resize(sizeX() + v.sizeX(), sizeY());
            for (size_t x = sizeX(); x < sizeX() + v.sizeX(); x++)
            {
                for (size_t y = 0; y < v.sizeY(); y++)
                    temp(x, y) = v(x - sizeX(), y);
            }
            std::cout << "temp=\n" << temp << std::endl;
            for (k = 0; k < sizeX(); k++)
            {
                long long indx = temp.pivot(k);
                if (indx == -1)
                    throw matrix_error("matrix<T,enableBoundsCheck>::solve(): Singular matrix!");

                a1 = temp(k, k);
                for (y = k; y < temp.sizeY(); y++)
                    temp(k, y) /= a1;

                for (x = k + 1; x < sizeX(); x++)
                {
                    a1 = temp(x, k);
                    for (y = k; y < temp.sizeY(); y++)
                        temp(x, y) -= a1 * temp(k, y);
                }
            }
            matrix<T, enableBoundsCheck> reval(v.sizeX(), v.sizeY());
            for (k = 0; k < v.sizeY(); k++)
                for (int m = int(sizeX()) - 1; m >= 0; m--)
                {
                    reval(m, k) = temp(m, sizeY() + k);

                    for (y = m + 1; y < sizeY(); y++)
                        reval(m, k) -= temp(m, y) * reval(y, k);
                }
            return reval;
        }

        /**
         * Solve simultaneous equations.
         * @param v matrix of values. Can be seen as set of vertical vectors
         * @return matrix representing the solutions
         */
        matrix<T, enableBoundsCheck> solve_old(const matrix<T, enableBoundsCheck>& v) const
        {
            checkSquare(*this, "matrix<T,enableBoundsCheck>::solve(v)");
            checkCompatibleSizes(*this, v, "solve", "matrix<T,enableBoundsCheck>::solve(v)");
            size_t x, y, k;
            T a1;

            matrix<T, enableBoundsCheck> temp(sizeX(), sizeY() + v.sizeY());
            for (x = 0; x < sizeX(); x++)
            {
                for (y = 0; y < sizeY(); y++)
                    temp.m_[x][y] = m_[x][y];
                for (k = 0; k < v.sizeY(); k++, y++)
                    temp.m_[x][y] = v.m_[x][k];
            }
            for (k = 0; k < sizeX(); k++)
            {
                long long indx = temp.pivot(k);
                if (indx == -1)
                    throw matrix_error("matrix<T,enableBoundsCheck>::solve(): Singular matrix!");

                a1 = temp.m_[k][k];
                for (y = k; y < temp.sizeY(); y++)
                    temp.m_[k][y] /= a1;

                for (x = k + 1; x < sizeX(); x++)
                {
                    a1 = temp.m_[x][k];
                    for (y = k; y < temp.sizeY(); y++)
                        temp.m_[x][y] -= a1 * temp.m_[k][y];
                }
            }
            matrix<T, enableBoundsCheck> reval(v.sizeX(), v.sizeY());
            for (k = 0; k < v.sizeY(); k++)
                for (int m = int(sizeX()) - 1; m >= 0; m--)
                {
                    reval.m_[m][k] = temp.m_[m][sizeY() + k];

                    for (y = m + 1; y < sizeY(); y++)
                        reval.m_[m][k] -= temp.m_[m][y] * reval.m_[y][k];
                }
            return reval;
        }

        /**
         * Calculate the determinant of this matrix.
         * @return the determinant
         */
        T det() const
        {
            checkSquare(*this, "matrix<T,enableBoundsCheck>::det()");
            T piv(0);
            T reval = T(1.0);

            matrix<T, enableBoundsCheck> temp(*this);

            for (size_t k = 0; k < sizeX(); k++)
            {
                int indx = temp.pivot(k);
                if (indx == -1)
                    return 0;
                if (indx != 0)
                    reval = -reval;
                reval *= temp(k, k);
                for (size_t x = k + 1; x < sizeX(); x++)
                {
                    piv = temp(x, k) / temp(k, k);

                    for (size_t y = k + 1; y < sizeY(); y++)
                        temp(x, y) -= piv * temp(k, y);
                }
            }
            return reval;
        }

        /**
         * Calculate the norm of a matrix.
         * @return the norm
         */
        T norm()
        {
            T reval = T(0);

            for (size_t x = 0; x < sizeX(); x++)
                for (size_t y = 0; y < sizeY(); y++)
                    reval += (*this)(x, y)*(*this)(x, y);
            reval = sqrt(reval);

            return reval;
        }

        /**
         * Calculate the condition number of a matrix.
         * @return the condition number
         */
        T cond()
        {

            matrix<T, enableBoundsCheck> inv = !(*this);
            return (norm() * inv.norm());
        }

        /**
         * Calculate the cofactor of a matrix for a given element.
         * @param x x-coordinate
         * @param y y-coordinate
         * @return the cofactor of matrix value at (x,y)
         */
        T cofact(size_t x, size_t y)
        {
            checkSquare(*this, "cofact(x,y)");
            checkBounds(*this, x, y, "cofact(x,y)");

            matrix<T, enableBoundsCheck> subMat(sizeX() - 1, sizeY() - 1);

            for (size_t x1 = 0, x2 = 0; x1 < sizeX(); x1++)
            {
                if (x1 == x)
                    continue;
                for (size_t y1 = 0, y2 = 0; y1 < sizeY(); y1++)
                {
                    if (y1 == y)
                        continue;
                    subMat(x2, y2) = (*this)(x1, y1);
                    y2++;
                }
                x2++;
            }
            T cof = subMat.det();

            if ((x + y) % 2 == 1)
                cof = -cof;

            return cof;
        }

        /**
         * Calculate adjoin of a matrix.
         * @return the adjoin
         */
        matrix<T, enableBoundsCheck>&& adj()
        {
            checkSquare(*this, "matrix<T,enableBoundsCheck>::adj()");
            matrix<T, enableBoundsCheck> reval(sizeX(), sizeY());

            for (size_t x = 0; x < sizeX(); x++)
                for (size_t y = 0; y < sizeY(); y++)
                    reval(x, y) = cofact(x, y);
            return reval;
        }

        /**
         * Check whether this is a singular matrix.
         * @return true if so, false otherwise
         */
        bool isSingular() const
        {

            return isSquare() && det() == T(0);
        }

        /**
         * Check whether this is a diagonal matrix.
         * @return true if so, false otherwise
         */
        bool isDiagonal() const
        {
            if (isSquare())
                return false;
            for (size_t y = 0; y < sizeX(); y++)
                for (size_t x = 0; x < sizeY(); x++)
                    if (x != y && (*this)(x, y) != T(0))
                        return false;
            return true;
        }

        /**
         * Check whether this is a scalar matrix.
         * @return true if so, false otherwise
         */
        bool isScalar() const
        {
            if (!isDiagonal())
                return false;
            T v = (*this)(0, 0);
            for (size_t xy = 1; xy < sizeX(); xy++)
                if ((*this)(xy, xy) != v)
                    return false;
            return true;
        }

        /**
         * Check whether this is a unit matrix.
         * @return true if so, false otherwise
         */
        bool isUnit() const
        {
            if (isScalar() && (*this)(0, 0) == T(1))
                return true;
            return false;
        }

        /**
         * Check whether this is a null matrix.
         * @return true if so, false otherwise
         */
        bool isNull() const
        {
            for (size_t y = 0; y < sizeX(); y++)
                for (size_t x = 0; x < sizeY(); x++)
                    if ((*this)(x, y) != T(0))
                        return false;
            return true;
        }

        /**
         * Check whether this is a symmetric matrix.
         * @return true if so, false otherwise
         */
        bool isSymmetric() const
        {
            if (!isSquare())
                return false;
            for (size_t y = 1; y < sizeY(); y++)
                for (size_t x = 0; x < y; x++)
                    if ((*this)(x, y) != (*this)(y, x))
                        return false;
            return true;
        }

        /**
         * Check whether this is a skew-symmetric matrix.
         * @return true if so, false otherwise
         */
        bool isSkewSymmetric() const
        {
            if (!isSquare())
                return false;
            for (size_t y = 1; y < sizeY(); y++)
                for (size_t x = 0; x < y; x++)
                    if ((*this)(x, y) != -(*this)(y, x))
                        return false;
            return true;
        }

        /**
         * Check whether this is a upper triangular matrix.
         * @return true if so, false otherwise
         */
        bool isUpperTriangular() const
        {
            if (!isSquare())
                return false;
            for (size_t y = 1; y < sizeY(); y++)
                for (size_t x = 0; x < y; x++)
                    if ((*this)(x, y) != T(0))
                        return false;
            return true;
        }

        /**
         * Check whether this is a lower triangular matrix.
         * @return true if so, false otherwise
         */
        bool isLowerTriangular() const
        {
            if (!isSquare())
                return false;
            for (size_t x = 1; x < sizeX(); x++)
                for (size_t y = 0; y < x; y++)
                    if ((*this)(x, y) != T(0))
                        return false;

            return true;
        }

        /**
         * Equality operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return true if equal, false, otherwise
         */
        friend bool operator==(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {

            return lhs.m_ == rhs.m_;
        }

        /**
         * Inequality operator.
         * @param lhs left-hand-side matrix
         * @param rhs right-hand-side matrix
         * @return true if *NOT* equal, false, otherwise
         */
        friend bool operator!=(const matrix<T, enableBoundsCheck>& lhs, const matrix<T, enableBoundsCheck>& rhs)
        {

            return !(lhs == rhs);
        }

        /**
         * Input stream function.
         * @param istrm the input stream to read from
         * @param m the matrix to read into
         * @return reference to the stream
         */
        friend std::istream& operator>>(std::istream& istrm, matrix<T, enableBoundsCheck>& m)
        {
            for (size_t y = 0; y < m.sizeY(); y++)
                for (size_t x = 0; x < m.sizeX(); x++)
                {

                    T val;
                    istrm >> val;
                    m(y, x) = val;
                }
            return istrm;
        }

        /**
         * Output stream function.
         * @param ostrm the output stream to write to
         * @param m the matrix to write
         * @return reference to the stream
         */
        friend std::ostream& operator<<(std::ostream& ostrm, const matrix<T, enableBoundsCheck>& m)
        {
            for (size_t y = 0; y < m.sizeY(); y++)
            {
                for (size_t x = 0; x < m.sizeX(); x++)
                {

                    const T& v = m(x, y);
                    ostrm << v << '\t';
                }
                ostrm << std::endl;
            }
            return ostrm;
        }

    private:

        /**
         * Partial pivoting method.
         * @param pivX
         * @return
         */
        long long pivot(size_t pivX)
        {
            long long k = pivX;
            long double amax = -1.0;
            long double temp = 0.0;

            for (size_t x = pivX; x < sizeX(); x++)
                if ((temp = abs((*this)(pivX, x))) > amax && temp != 0.0)
                {
                    amax = temp;
                    k = x;
                }
            if ((*this)(pivX, k) == T(0))
                return -1;
            if (k != (long long) (pivX))
            {
                std::swap(row(k), row(pivX));
                return k;
            }
            return 0;
        }
    };

    template<bool enable = false >
    void checkBounds(const matrix_interface& lhs,
                     size_t x,
                     size_t y,
                     const std::string& location)
    {
        if (!lhs.withinBounds(x, y))
        {

            std::stringstream ss;
            ss << location
                    << ": index ("
                    << x
                    << ","
                    << y
                    << ") is out of bounds ("
                    << lhs.sizeX()
                    << ","
                    << lhs.sizeY()
                    << ").";
            throw matrix_error(ss.str());
        }
    }

    template<>
    void checkBounds<false>(const matrix_interface& lhs,
                            size_t x,
                            size_t y,
                            const std::string& location)
    {
    }

}

#endif //NS_UTIL_MATRIX_H_INCLUDED