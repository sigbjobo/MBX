#pragma once

#include <iostream>

class ConnectivityMatrix
{
    public:
        ConnectivityMatrix(const int size);
        ConnectivityMatrix(const ConnectivityMatrix& old);
        ~ConnectivityMatrix();

        void add_connection(const int index1, const int index2);
        void remove_connection(const int index1, const int index2);
        bool has_connection(const int index1, const int index2) const;

        bool operator==(const ConnectivityMatrix other) const;
        
        friend std::ostream& operator<<(std::ostream& os, const ConnectivityMatrix& con_matrix);

    private:
        bool** matrix;
        int size;

};
