#include "pch.h"
#include "CppUnitTest.h"
#include "../CudaCrsMatrix/CrsMatrix.h"
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	/** Compares two floating point values for 1E-3
	*  This tolerance is selected because of the numbers used in the unit tests.
	*/
	template<typename real>
	bool floating_point_equal(real num1, real num2) {
		const real tol = 1E-3;
		if (num1 + tol > num2 && num2 > num1 - tol)
			return true;
		else return false;
	}
	TEST_CLASS(CRS_CPU)
	{
	public:

		TEST_METHOD(CRS_INITIALIZATION)
		{
			std::cout << "CRS_INITIALIZATION TEST" << std::endl;
			unsigned int rows = 10;
			unsigned int cols = 10;

			CrsMatrix<float> test_matrix(rows, cols);

			std::stringstream buffer;
			std::streambuf* sbuf = std::cout.rdbuf();
			std::cout.rdbuf(buffer.rdbuf());

			test_matrix.print();
			std::cout.rdbuf(sbuf);
			std::cout << "std original buffer: \n";
			std::cout << buffer.str();
			std::string expected = "INFORMATION\n------------------\nNumber of Rows: 10\nNumber of Columns: 10\nNumber of Non-Zeros: 0\nVALUES\n------------------\n";
			Assert::AreEqual(expected, buffer.str());
		}
		TEST_METHOD(CRS_INSERTION)
		{
			std::cout << "CRS_INSERTION TEST" << std::endl;
			unsigned int rows = 10;
			unsigned int cols = 10;

			CrsMatrix<float> test_matrix(rows, cols);
			std::unique_ptr<unsigned int[]> col_values = std::make_unique<unsigned int[]>(2);
			col_values[0] = 1;
			col_values[1] = 4;
			std::unique_ptr<float[]> values = std::make_unique<float[]>(2);
			values[0] = 4.1;
			values[1] = 2.0;
			test_matrix.insert_data_at_row(0, 2, values.get(), col_values.get());
			test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
			values[0] = 3.1;
			values[1] = 1.7;
			col_values[0] = 2;
			col_values[1] = 0;
			test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
			test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
			std::unique_ptr<unsigned int[]> return_cols = std::make_unique<unsigned int[]>(6);
			std::unique_ptr<float[]> return_values = std::make_unique<float[]>(6);
			unsigned int return_length;
			test_matrix.print();
			// Before Compression
			for (unsigned int row_number = 0; row_number < rows; row_number++) {
				test_matrix.get_values_in_row(row_number, 6, return_length, return_values.get(), return_cols.get());
				if (row_number != 0 && row_number != 4) {
					Assert::AreEqual(return_length, (unsigned int)0);
				}
				else if (row_number == 0) {
					Assert::AreEqual(return_length, (unsigned int)2);
					Assert::AreEqual(return_values[0], (float)4.1);
					Assert::AreEqual(return_cols[0], (unsigned int)1);
					Assert::AreEqual(return_values[1], (float)2.0);
					Assert::AreEqual(return_cols[1], (unsigned int)4);
				} 
				else {
					Assert::AreEqual(return_length, (unsigned int)6);
					Assert::AreEqual(return_values[0], (float)4.1);
					Assert::AreEqual(return_cols[0], (unsigned int)1);
					Assert::AreEqual(return_values[1], (float)2.0);
					Assert::AreEqual(return_cols[1], (unsigned int)4);
					Assert::AreEqual(return_values[2], (float)3.1);
					Assert::AreEqual(return_cols[2], (unsigned int)2);
					Assert::AreEqual(return_values[3], (float)1.7);
					Assert::AreEqual(return_cols[3], (unsigned int)0);
					Assert::AreEqual(return_values[4], (float)3.1);
					Assert::AreEqual(return_cols[4], (unsigned int)2);
					Assert::AreEqual(return_values[5], (float)1.7);
					Assert::AreEqual(return_cols[5], (unsigned int)0);
				}
			}
			// After Compression
			test_matrix.compress();
			test_matrix.print();
			for (unsigned int row_number = 0; row_number < rows; row_number++) {
				test_matrix.get_values_in_row(row_number, 6, return_length, return_values.get(), return_cols.get());
				if (row_number != 0 && row_number != 4) {
					Assert::AreEqual(return_length, (unsigned int)0);
				}
				else if (row_number == 0) {
					Assert::AreEqual(return_length, (unsigned int)2);
					Assert::AreEqual(return_values[0], (float)4.1);
					Assert::AreEqual(return_cols[0], (unsigned int)1);
					Assert::AreEqual(return_values[1], (float)2.0);
					Assert::AreEqual(return_cols[1], (unsigned int)4);
				}
				else {
					Assert::AreEqual(return_length, (unsigned int)4);
					Assert::AreEqual(return_values[0], (float)3.4);
					Assert::AreEqual(return_cols[0], (unsigned int)0);
					Assert::AreEqual(return_values[1], (float)4.1);
					Assert::AreEqual(return_cols[1], (unsigned int)1);
					Assert::AreEqual(return_values[2], (float)6.2);
					Assert::AreEqual(return_cols[2], (unsigned int)2);
					Assert::AreEqual(return_values[3], (float)2.0);
					Assert::AreEqual(return_cols[3], (unsigned int)4);
				}
			}
		}
		
		TEST_METHOD(CRS_ADD) {
			unsigned int rows = 10;
			unsigned int cols = 10;

			CrsMatrix<float> A_matrix(rows, cols);
			CrsMatrix<float> B_matrix(rows, cols);
			CrsMatrix<float> C_matrix(rows, cols);
			std::unique_ptr<unsigned int[]> col_values = std::make_unique<unsigned int[]>(2);
			std::unique_ptr<float[]> values = std::make_unique<float[]>(2);
			col_values[0] = 1;
			col_values[1] = 4;
			values[0] = 4.1;
			values[1] = 2.0;
			// Row 0 - One Row
			A_matrix.insert_data_at_row(0, 2, values.get(), col_values.get());
			// Row 1 - Same values in both matrices
			A_matrix.insert_data_at_row(1, 2, values.get(), col_values.get());
			B_matrix.insert_data_at_row(1, 2, values.get(), col_values.get());
			// Row 2 - Two rows none matching
			B_matrix.insert_data_at_row(2, 2, values.get(), col_values.get());
			col_values[0] = 0;
			col_values[1] = 2;
			A_matrix.insert_data_at_row(2, 2, values.get(), col_values.get());
			// Row 3 - Two rows, one matching
			A_matrix.insert_data_at_row(3, 2, values.get(), col_values.get());
			col_values[0] = 3;
			B_matrix.insert_data_at_row(3, 2, values.get(), col_values.get());
			// Compress
			A_matrix.compress();
			A_matrix.print();
			B_matrix.compress();
			B_matrix.print();
			// 3*A+2*B = C
			A_matrix.add(3.0, B_matrix, 2.0, C_matrix);
			C_matrix.print();
			std::unique_ptr<unsigned int[]> return_cols = std::make_unique<unsigned int[]>(4);
			std::unique_ptr<float[]> return_values = std::make_unique<float[]>(4);
			unsigned int return_length;
			for (unsigned int row_number = 0; row_number < rows; row_number++) {
				C_matrix.get_values_in_row(row_number, 6, return_length, return_values.get(), return_cols.get());
				if (row_number > 3) {
					Assert::AreEqual((unsigned int)0, return_length);
				}
				else if (row_number == 0) {
					Assert::AreEqual((unsigned int)2, return_length);
					Assert::AreEqual((unsigned int)1, return_cols[0]);
					Assert::IsTrue(floating_point_equal((float)12.3, return_values[0]));
					Assert::AreEqual((unsigned int)4, return_cols[1]);
					Assert::IsTrue(floating_point_equal((float)6.0, return_values[1]));
				}
				else if (row_number == 1) {
					Assert::AreEqual((unsigned int)2, return_length);
					Assert::AreEqual((unsigned int)1, return_cols[0]);
					Assert::IsTrue(floating_point_equal((float)20.5, return_values[0]));
					Assert::AreEqual((unsigned int)4, return_cols[1]);
					Assert::IsTrue(floating_point_equal((float)10.0, return_values[1]));
				}
				else if (row_number == 2) {
					Assert::AreEqual((unsigned int)4, return_length);
					Assert::AreEqual((unsigned int)0, return_cols[0]);
					Assert::IsTrue(floating_point_equal((float)12.3, return_values[0]));
					Assert::AreEqual((unsigned int)1, return_cols[1]);
					Assert::IsTrue(floating_point_equal((float)8.2, return_values[1]));
					Assert::AreEqual((unsigned int)2, return_cols[2]);
					Assert::IsTrue(floating_point_equal((float)6.0, return_values[2]));
					Assert::AreEqual((unsigned int)4, return_cols[3]);
					Assert::IsTrue(floating_point_equal((float)4.0, return_values[3]));
				}
				else if (row_number == 3) {
					Assert::AreEqual((unsigned int)3, return_length);
					Assert::AreEqual((unsigned int)0, return_cols[0]);
					Assert::IsTrue(floating_point_equal((float)12.3, return_values[0]));
					Assert::AreEqual((unsigned int)2, return_cols[1]);
					Assert::IsTrue(floating_point_equal((float)10.0, return_values[1]));
					Assert::AreEqual((unsigned int)3, return_cols[2]);
					Assert::IsTrue(floating_point_equal((float)8.2, return_values[2]));
				}
			}
		}
	};
}
