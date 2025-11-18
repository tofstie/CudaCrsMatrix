#ifndef __CRSMATRIX__
#define __CRSMATRIX__

#include "cuda_runtime.h"
#include <memory>
template <typename real>
class CrsMatrix
{
	// Information
	unsigned int global_rows;
	unsigned int global_cols;
	unsigned int number_of_non_zeros;
	using matrix_type = real;
	bool compressed;

	// Values
	std::unique_ptr<real[]> values;
	std::unique_ptr<unsigned int[]> cols;
	std::unique_ptr<unsigned int[]> rows;

public:
	/// Constructor
	__host__ CrsMatrix(unsigned int rows, unsigned int cols);
	/** <summary>
		 Inserts data at specified values
		 </summary>
		 <param name="values"></param>
		 <param name="rows"></param>
		 <param name="cols"></param>
		 <returns></returns>*/
	__host__ int insert_data_at_row(unsigned int row, unsigned int length, real *values, unsigned int *cols);
	/// <summary>
	/// Compresses the CrsMatrix. Call this after all adjustments are made
	/// </summary>
	/// <returns></returns>
	__host__ int compress();
	/// <summary>
	/// Adds two CrsMatrices
	/// </summary>
	/// <param name="B"></param>
	/// <param name="scaler"></param>
	/// <param name="C"></param>
	/// <returns></returns>
	__host__ int add(const CrsMatrix<real> &B, const double scaler, CrsMatrix<real> &C);

	__host__ void print();

private:
	template <typename T1, typename T2>
	/** Extends two pointers with new values
	* 
	*/
	__host__ void extend_unique_ptr_2(	const unsigned int location_for_insert, 
										const unsigned int length_of_data,
										const unsigned int length_of_original,
										const std::unique_ptr<T1[]> &original_ptr_1, std::unique_ptr<T1[]> &resulting_ptr_1,
										const std::unique_ptr<T2[]> &original_ptr_2, std::unique_ptr<T2[]> &resulting_ptr_2,
										const T1* data_1, const T2* data_2);
};

#endif // !__CRSMATRIX__