#ifndef __CRSMATRIX__
#define __CRSMATRIX__

#include "cuda_runtime.h"
#include <memory>
#include <stdexcept>
#include <map>
#include <type_traits>
#include <utility>
#include <iostream>
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
	__host__ CrsMatrix(unsigned int total_rows, unsigned int total_cols)
		: global_rows(total_rows)
		, global_cols(total_cols)
		, number_of_non_zeros(0)
		, compressed(false)
		, values(std::make_unique<real[]>(0))
		, cols(std::make_unique<unsigned int[]>(0))
		, rows(std::make_unique<unsigned int[]>(total_rows + 1))
	{
		for (unsigned int i = 0; i < total_rows + 1; i++) {
			rows[i] = 0;
		}
	};
	/** <summary>
		 Inserts data at specified values
		 </summary>
		 <param name="values"></param>
		 <param name="rows"></param>
		 <param name="cols"></param>
		 <returns></returns>*/
	__host__ int insert_data_at_row(
		const unsigned int row, 
		const unsigned int length, 
		real *values_to_insert, 
		unsigned int * cols_to_insert)
	{
		if (row > global_rows - 1) throw std::out_of_range("Requested row is out of range");
		const unsigned int row_start_location = rows[row];
		const unsigned int row_end_location = rows[row + 1];
		std::unique_ptr<unsigned int[]>new_cols = std::make_unique<unsigned int[]>(number_of_non_zeros + length);
		std::unique_ptr<real[]>new_values = std::make_unique<real[]>(number_of_non_zeros + length);
		extend_unique_ptr_2(row_end_location, length, number_of_non_zeros, cols, new_cols, values, new_values, cols_to_insert, values_to_insert);
		for (unsigned int i = row + 1; i < global_rows + 1; i++) {
			rows[i] += length;
		}

		number_of_non_zeros += length;
		cols = std::move(new_cols);
		values = std::move(new_values);
		return 0;
	};

	__host__ int get_values_in_row(
		const unsigned int row_index,
		const unsigned int array_length,
		unsigned int& number_returned,
		real* values_return,
		unsigned int* columns_return) 
	{
		const unsigned int row_start_location = rows[row_index];
		const unsigned int row_end_location = rows[row_index + 1];
		const unsigned int length_row = row_end_location - row_start_location;
		if (length_row > array_length) return -1; // Input array too small
		unsigned int replace_index = 0;
		for (unsigned int i = row_start_location; i < row_end_location; i++, replace_index++) {
			values_return[replace_index] = values[i];
			columns_return[replace_index] = cols[i];
		}
		number_returned = length_row;
		return 0;
	}

	/// <summary>
	/// Compresses the CrsMatrix. Call this after all adjustments are made
	/// </summary>
	/// <returns></returns>
	__host__ int compress()
	{
		if (compressed) return -1;
		std::unique_ptr<unsigned int[]>new_cols = std::make_unique<unsigned int[]>(0);
		std::unique_ptr<real[]>new_values = std::make_unique<real[]>(0);
		std::unique_ptr<unsigned int[]>new_rows = std::make_unique<unsigned int[]>(global_rows+1);
		unsigned int length_of_new_values = 0;
		for (unsigned int i = 0; i < global_rows + 1; i++) {
			new_rows[i] = 0;
		}
		for (unsigned int i = 0; i < global_rows; i++) {
			std::map<unsigned int, real> compressed_values_map;
			unsigned int row_start_location = rows[i];
			unsigned int row_end_location = rows[i + 1];
			for (unsigned int j = row_start_location; j < row_end_location; j++) {
				if (compressed_values_map.count(cols[j])) {
					compressed_values_map[cols[j]] += values[j];
				}
				else {
					compressed_values_map[cols[j]] = values[j];
				}
			}
			unsigned int* cols_to_insert = new unsigned int[compressed_values_map.size()];
			real* values_to_insert = new real[compressed_values_map.size()];
			unsigned int idx = 0;
			for (auto key_value_pair = compressed_values_map.begin(); key_value_pair != compressed_values_map.end(); key_value_pair++, idx++) {
				cols_to_insert[idx] = key_value_pair->first;
				values_to_insert[idx] = key_value_pair->second;
			}
			std::unique_ptr<unsigned int[]>temp_cols = std::make_unique<unsigned int[]>(length_of_new_values + compressed_values_map.size());
			std::unique_ptr<real[]>temp_values = std::make_unique<real[]>(length_of_new_values + compressed_values_map.size());
			extend_unique_ptr_2(length_of_new_values, compressed_values_map.size(), length_of_new_values, new_cols, temp_cols, new_values, temp_values, cols_to_insert, values_to_insert);
			delete[] cols_to_insert;
			delete[] values_to_insert;
			length_of_new_values += compressed_values_map.size();
			new_cols = std::move(temp_cols);
			new_values = std::move(temp_values);
			new_rows[i + 1] = length_of_new_values;
	
		}
		rows = std::move(new_rows);
		cols = std::move(new_cols);
		values = std::move(new_values);
		number_of_non_zeros = length_of_new_values;
		compressed = true;
		return 0;
	};
	/// <summary>
	/// Adds two CrsMatrices
	/// </summary>
	/// <param name="B"></param>
	/// <param name="scaler"></param>
	/// <param name="C"></param>
	/// <returns></returns>
	__host__ int add(
		const real scalerA, 
		const CrsMatrix<real>& B, 
		const real scalerB, 
		CrsMatrix<real>& C)
	{
		if (this->global_rows != B.global_rows || this->global_rows != C.global_rows)
			throw std::invalid_argument("The input and/or output matrices do not have a matching number of rows");
		if (this->global_cols != B.global_cols)
			throw std::invalid_argument("The input and/or output matrices do not have a matching number of cols");

		// two pointer
		for (unsigned int i = 0; i < global_rows; i++) {
			const unsigned int row_start_location_A = rows[i];
			const unsigned int row_end_location_A = rows[i + 1];
			const unsigned int length_A = row_end_location_A - row_start_location_A;

			const unsigned int row_start_location_B = B.rows[i];
			const unsigned int row_end_location_B = B.rows[i + 1];
			const unsigned int length_B = row_end_location_B - row_start_location_B;
			
			std::unique_ptr<unsigned int[]> new_cols = std::make_unique<unsigned int[]>(length_A + length_B);
			std::unique_ptr<real[]> new_values = std::make_unique<real[]>(length_A + length_B);

			unsigned int length_C = 0;
			unsigned int index_A = row_start_location_A;
			unsigned int index_B = row_start_location_B;
			
			while (index_A < row_end_location_A || index_B < row_end_location_B) {
				if (index_A < row_end_location_A) {
					new_cols[length_C] = cols[index_A];
					new_values[length_C] = scalerA*values[index_A];
					length_C++; index_A++;
				}
				if (index_B < row_end_location_B) {
					new_cols[length_C] = B.cols[index_B];
					new_values[length_C] = scalerB*B.values[index_B];
					length_C++; index_B++;
				}
			}

			C.insert_data_at_row(i, length_C, new_values.get(), new_cols.get());
		}
		C.compress();
		return 0;
	};

	

	__host__ int hadamard(const real scalerA, const CrsMatrix<real>& B, const real scalerB, CrsMatrix<real>& C);

	__device__ int hadamard_gpu(const real scalerA, const CrsMatrix<real>& B, const real scalerB, CrsMatrix<real>& C);

	__host__ void print()
	{
		std::cout << "INFORMATION" << "\n------------------\n";
		std::cout << "Number of Rows: " << global_rows << '\n';
		std::cout << "Number of Columns: " << global_cols << '\n';
		std::cout << "Number of Non-Zeros: " << number_of_non_zeros << '\n';
		std::cout << "VALUES" << "\n------------------\n";
		for (unsigned int i = 0; i < global_rows; i++) {
			const unsigned int row_start_location = rows[i];
			const unsigned int row_end_location = rows[i + 1];
			for (unsigned int j = row_start_location; j < row_end_location; j++) {
				std::cout << "Row: " << i << " Col: " << cols[j] << " Value " << values[j] << '\n';
			}
		}
		std::cout << std::flush;

	};

private:
	template <typename T1, typename T2>
	/** Extends two pointers with new values
	* 
	*/
	__host__ void extend_unique_ptr_2(	
		const unsigned int location_for_insert, 
		const unsigned int length_of_data,
		const unsigned int length_of_original,
		const std::unique_ptr<T1[]> &original_ptr_1, std::unique_ptr<T1[]> &resulting_ptr_1,
		const std::unique_ptr<T2[]> &original_ptr_2, std::unique_ptr<T2[]> &resulting_ptr_2,
		const T1* data_1, const T2* data_2)
	{
		for (unsigned int i = 0; i < location_for_insert; i++) {
			resulting_ptr_1[i] = original_ptr_1[i];
			resulting_ptr_2[i] = original_ptr_2[i];
		}
		for (unsigned int i = location_for_insert; i < length_of_original; i++) {
			resulting_ptr_1[i + length_of_data] = original_ptr_1[i];
			resulting_ptr_2[i + length_of_data] = original_ptr_2[i];
		}
		unsigned int j = location_for_insert;
		for (unsigned int i = 0; i < length_of_data; i++, j++) {
			resulting_ptr_1[j] = data_1[i];
			resulting_ptr_2[j] = data_2[i];
		}

		return;
	};
};

template class CrsMatrix <double>; //<-- Only defined on GPUs that can handle double precision floating point numbers
template class CrsMatrix <float>; //<-- Universily defined.
#endif // !__CRSMATRIX__