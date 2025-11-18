#include "CrsMatrix.h"
#include <iostream>
int main() {
	unsigned int rows = 5;
	unsigned int cols = 5;
	CrsMatrix<float> test_matrix(rows, cols);
	std::unique_ptr<unsigned int[]> col_values = std::make_unique<unsigned int[]>(2);
	col_values[0] = 1;
	col_values[1] = 4;
	std::unique_ptr<float[]> values = std::make_unique<float[]>(2);
	values[0] = 4.1;
	values[1] = 2.0;
	test_matrix.print();
	test_matrix.insert_data_at_row(0, 2, values.get(), col_values.get());
	test_matrix.print();
	test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
	test_matrix.print();
	values[0] = 3.1;
	values[1] = 1.7;
	col_values[0] = 2;
	col_values[1] = 0;
	test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
	test_matrix.insert_data_at_row(4, 2, values.get(), col_values.get());
	test_matrix.compress();
	test_matrix.print();
	return 0;
}