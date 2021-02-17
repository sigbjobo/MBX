#include <iostream> 

#include "potential/fragmented/ConnectivityMatrix.h"

ConnectivityMatrix::ConnectivityMatrix(const int size) {

    this->size = size;

    this->matrix = new bool* [size]; 

    for(int index1 = 0; index1 < this->size; index1++) {
        this->matrix[index1] = new bool [size];
        for(int index2 = 0; index2 < this->size; index2++)
            this->matrix[index1][index2] = false;
    }
}

ConnectivityMatrix::ConnectivityMatrix(const ConnectivityMatrix& old) {
    this->size = old.size;

    this->matrix = new bool* [this->size]; 

    for(int index1 = 0; index1 < this->size; index1++) {
        this->matrix[index1] = new bool [this->size];
        for(int index2 = 0; index2 < this->size; index2++)
            this->matrix[index1][index2] = old.has_connection(index1, index2);
    }

}

ConnectivityMatrix::~ConnectivityMatrix() {

    for(int index1 = 0; index1 < this->size; index1++) {
        delete[] this->matrix[index1];
    }

    delete[] this->matrix;
}

void ConnectivityMatrix::add_connection(const int index1, const int index2) {

    this->matrix[index1][index2] = true;

}

void ConnectivityMatrix::remove_connection(const int index1, const int index2) {

    this->matrix[index1][index2] = false;

}

bool ConnectivityMatrix::has_connection(const int index1, const int index2) const {
    return this->matrix[index1][index2];
}

bool ConnectivityMatrix::operator==(const ConnectivityMatrix other) const {

    if(this->size != other.size) return false;

    for(int index1 = 0; index1 < this->size; index1++)
        for(int index2 = 0; index2 < this->size; index2++)
            if(this->matrix[index1][index2] != other.matrix[index1][index2]) return false;

    return true;

}

        
std::ostream& operator<<(std::ostream& os, const ConnectivityMatrix& con_matrix) {

    for(int index1 = 0; index1 < con_matrix.size; index1++) {
        for(int index2 = 0; index2 < con_matrix.size; index2++)
            os << con_matrix.has_connection(index1, index2);
        if(index1 < con_matrix.size - 1) os << std::endl;
    }
    return os;
}
