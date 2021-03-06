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
 * @date: 2019-10-13
 * @author: Dieter J Kybelksties
 */

#ifndef MATRIXTEST_H
#define MATRIXTEST_H

#include <cppunit/extensions/HelperMacros.h>

class matrixTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(matrixTest);

    CPPUNIT_TEST(testMatrixConstruction);
    CPPUNIT_TEST(testExceptions);
    CPPUNIT_TEST(testMatrixOperations);
    CPPUNIT_TEST(testSquareMatrixOperations);
    CPPUNIT_TEST_SUITE_END();

    public:
    matrixTest();
    virtual ~matrixTest();
    void setUp();
    void tearDown();

    private:
    void testMatrixConstruction();
    void testExceptions();
    void testMatrixOperations();
    void testSquareMatrixOperations();
};

#endif /* MATRIXTEST_H */
