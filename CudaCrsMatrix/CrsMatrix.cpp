#include "CrsMatrix.h"
#include <stdexcept>
#include <map>
#include <type_traits>
#include <utility>
#include <iostream>
template <typename real>
CrsMatrix<real>::CrsMatrix(unsigned int total_rows, unsigned int total_cols)
	: global_rows(total_rows)
	, global_cols(total_cols)
	, number_of_non_zeros(0)
	, compressed(false)
	, values(std::make_unique<real[]>(0))
	, cols(std::make_unique<unsigned int[]>(0))
	, rows(std::make_unique<unsigned int[]>(total_rows+1))
{
	for (unsigned int i = 0; i < total_rows+1; i++) {
		rows[i] = 0;
	}
}

template <typename real>
int CrsMatrix<real>::insert_data_at_row(const unsigned int row, const unsigned int length, real* values_to_insert, unsigned int* cols_to_insert)
{
	if (row > global_rows - 1) throw std::out_of_range("Requested row is out of range");
	const unsigned int row_start_location = rows[row];
	const unsigned int row_end_location = rows[row + 1];
	std::unique_ptr<unsigned int[]>new_cols = std::make_unique<unsigned int[]>(number_of_non_zeros + length);
	std::unique_ptr<real[]>new_values = std::make_unique<real[]>(number_of_non_zeros + length);
	extend_unique_ptr_2(row_end_location, length, number_of_non_zeros, cols, new_cols, values, new_values, cols_to_insert, values_to_insert);
	for (unsigned int i = row + 1; i < global_rows+1; i++) {
		rows[i] += length;
	}

	number_of_non_zeros += length;
	cols = std::move(new_cols);
	values = std::move(new_values);
	return 0;
}

template <typename real>
int CrsMatrix<real>::compress() {
	if (compressed) return -1;
	std::unique_ptr<unsigned int[]>new_cols = std::make_unique<unsigned int[]>(0);
	std::unique_ptr<real[]>new_values = std::make_unique<real[]>(0);
	unsigned int length_of_new_values = 0;
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
		rows[i + 1] = length_of_new_values;
	}
	cols = std::move(new_cols);
	values = std::move(new_values);
	number_of_non_zeros = length_of_new_values;
	compressed = true;
	return 0;
}

template <typename real>
int CrsMatrix<real>::add(const CrsMatrix<real>& B, const double scaler, CrsMatrix<real>& C) {
	if (this->global_rows != B.global_rows || this->global_rows != C.global_rows)
		throw std::invalid_argument("The input and/or output matrices do not have a matching number of rows");
	if (this->global_cols != B.global_cols) 
		throw std::invalid_argument("The input and/or output matrices do not have a matching number of cols");

	return 0;
}

template <typename real>
void CrsMatrix<real>::print() {
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

}

template <typename real>
template <typename T1, typename T2>
void CrsMatrix<real>::extend_unique_ptr_2(	const unsigned int location_for_insert,
											const unsigned int length_of_data,
											const unsigned int length_of_original,
											const std::unique_ptr<T1[]>& original_ptr_1, std::unique_ptr<T1[]>& resulting_ptr_1,
											const std::unique_ptr<T2[]>& original_ptr_2, std::unique_ptr<T2[]>& resulting_ptr_2,
											const T1* data_1, const T2* data_2) {

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
}

template class CrsMatrix <double>; //<-- Only defined on GPUs that can handle double precision floating point numbers
template class CrsMatrix <float>; //<-- Universily defined.