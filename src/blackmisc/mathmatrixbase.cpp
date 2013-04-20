/*  Copyright (C) 2013 VATSIM Community / authors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "blackmisc/mathmatrix3x3.h"
#include "blackmisc/mathmatrix3x1.h"

namespace BlackMisc
{
namespace Math
{

/*
 * Get element by column / row
 */
template<class ImplMatrix, int Rows, int Columns> double CMatrixBase<ImplMatrix, Rows, Columns>::getElement(size_t row, size_t column) const
{
    this->checkRange(row, column);
    return this->m_matrix(row, column);
}

/*
 * Set element by column / row
 */
template<class ImplMatrix, int Rows, int Columns> void CMatrixBase<ImplMatrix, Rows, Columns>::setElement(size_t row, size_t column, double value)
{
    this->checkRange(row, column);
    this->m_matrix(row, column) = value;
}

/*
 * Check range
 */
template<class ImplMatrix, int Rows, int Columns> void CMatrixBase<ImplMatrix, Rows, Columns>::checkRange(size_t row, size_t column) const
{
    bool valid = (row >= 0 && column >= 0 && row < Rows && column < Columns);
    Q_ASSERT_X(valid, "getElement()", "Row or column invalid");
    if (!valid) throw std::range_error("Row or column invalid");
}

/*
 * All values to random value
 */
template<class ImplMatrix, int Rows, int Columns> void CMatrixBase<ImplMatrix, Rows, Columns>::setRandom()
{
    for (int r = 0; r < Rows; r++)
    {
        for (int c = 0; c < Columns; c++)
        {
            this->m_matrix(r, c) = (qrand() % 101);  // 0...100
        }
    }
}

/*
 * All values zero?
 */
template<class ImplMatrix, int Rows, int Columns> bool CMatrixBase<ImplMatrix, Rows, Columns>::isZero() const
{
    for (int r = 0; r < Rows; r++)
    {
        for (int c = 0; c < Columns; c++)
        {
            if (this->m_matrix(r, c) != 0) return false;
        }
    }
    return true;
}

/*
 * All values zero?
 */
template<class ImplMatrix, int Rows, int Columns> bool CMatrixBase<ImplMatrix, Rows, Columns>::allValuesEqual() const
{
    double v = this->getElement(0,0);
    for (int r = 0; r < Rows; r++)
    {
        for (int c = 0; c < Columns; c++)
        {
            if (this->m_matrix(r, c) != v) return false;
        }
    }
    return true;
}


/*
 * Convert to string
 */
template <class ImplMatrix, int Rows, int Columns> QString CMatrixBase<ImplMatrix, Rows, Columns>::stringForConverter() const
{
    QString s = "{";
    for (int r = 0; r < Rows; r++)
    {
        s = s.append("{");
        for (int c = 0; c < Columns; c++)
        {
            QString n = QString::number(this->m_matrix(r, c), 'f', 2);
            if (c > 0) s = s.append(",");
            s = s.append(n);
        }
        s = s.append("}");
    }
    s = s.append("}");
    return s;
}

// see here for the reason of thess forward instantiations
// http://www.parashift.com/c++-faq/separate-template-class-defn-from-decl.html
template class CMatrixBase<CMatrix3x3, 3, 3>;
template class CMatrixBase<CMatrix3x1, 3, 1>;
template class CMatrixBase<CMatrix1x3, 1, 3>;

} // namespace

} // namespace

#include "mathmatrixbase.h"

