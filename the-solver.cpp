// the-solver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <vector>

#include <set>
#include <map>
#include <array>


#include <stdint.h>


typedef std::array<int, 3> XistenceTuple;

typedef std::set<XistenceTuple> XistenceSet; // sorted stuff
typedef std::vector<XistenceTuple> XistenceVector;

typedef std::map<XistenceTuple, XistenceSet> XistenceSetMap;

typedef std::map<XistenceTuple, XistenceVector> XistenceVectorMap;

typedef std::vector<XistenceSet> XistenceSetVector;


auto extract_intersects(XistenceVectorMap& rows, 
    XistenceSetMap& columns, const XistenceTuple& base_row)
{
    XistenceSetVector buf;
    for (auto& elt : rows[base_row])
    {
        // take out the current column from the table to the buffer
        auto it = columns.find(elt);
        buf.push_back(it->second);
        columns.erase(it);

        // remove all overlapping rows from all remaining columns
        for (auto& intersecting_row : buf.back())
            for (auto& other_elt : rows[intersecting_row])
                if (other_elt != elt)
                {
                    columns[other_elt].erase(intersecting_row);
                }
    }
    return buf;
}

void restore_intersects(XistenceVectorMap& rows, XistenceSetMap& columns, 
    const XistenceTuple& base_row, XistenceSetVector& buf)
{
    // columns were deleted from the first intersection with base_row to the last one,
    // restore in reverse order
    auto& row = rows[base_row];
    for (auto it(row.rbegin()), itEnd(row.rend()); it != itEnd; ++it)
    {
        auto& elt = *it;
        columns[elt] = buf.back();
        buf.pop_back();
        for (auto& added_row : columns[elt])
            for (auto& col : rows[added_row])
            {
                columns[col].insert(added_row);
            }
    }
}


bool algorithm_x(XistenceVectorMap& rows, 
    XistenceSetMap& columns, XistenceVector& cover)
{
    if (columns.empty())
        return true;
    else
    {
        // looking for a column with a minimum number of elements
        XistenceTuple c;
        size_t count = SIZE_MAX;
        for (auto& v : columns)
        {
            if (v.second.size() < count)
            {
                count = v.second.size();
                c = v.first;
                if (count == 1)
                    break;
            }
        }

        const auto collect = columns[c];
        for (auto& subset : collect)
        {
            cover.push_back(subset);
            // remove overlapping subsets and elements contained in the subset
            auto buf_cols = extract_intersects(rows, columns, subset);
            if (algorithm_x(rows, columns, cover))
                return true;
            // if a non-empty solution is found - ready, exit
            restore_intersects(rows, columns, subset, buf_cols);
            cover.pop_back();
        }
        // we get here either if columns [c] is empty,
        // or when the recursive search did not find a solution
        return false;
    }
}


typedef std::array<std::array<int, 9>, 9> Matrix;


// Sudoku is given by a 9x9 matrix, with zeros in place of unknown numbers
// row identifiers are tuples of the form (row, col, num)
// column identifiers:
// (0, row, col) - there is a number at the intersection of row and col
// (1, row, num) - 'row' row contains number num
// (2, col, num) - there is a number num in the 'col' column
// (3, q, num) - there is a number num in the q quadrant

template<typename T>
Matrix xsudoku(const T& puzzle)
{
    // fill in the lines

    XistenceVectorMap rows;

    for (int row = 1; row <= 9; ++row)
        for (int col = 1; col <= 9; ++col)
            for (int num = 1; num <= 9; ++num)
            {
                const auto quad = ((row - 1) / 3) * 3 + (col - 1) / 3 + 1;

                rows[{row, col, num}] = {
                    { 0, row, col },
                    { 1, row, num },
                    { 2, col, num },
                    { 3, quad, num}
                };
            }
    // fill in the columns

    XistenceSetMap cols;

    for (auto&[rk, rv] : rows)
        for (auto& v : rv)
        {
            cols[v].insert(rk);
        }

    // s - template for the result
    // for a start, put the numbers that are already filled in

    XistenceVector s;

    for (int i = 1; i <= 9; ++i)
        for (int j = 1; j <= 9; ++j)
        {

            if (puzzle[i - 1][j - 1] > 0)
            {
                XistenceTuple elt{ i, j, puzzle[i - 1][j - 1] };
                s.push_back(elt);
                // adding a cell to the solution, remove all incompatible elements from the matrix
                extract_intersects(rows, cols, elt);
            }
        }

    Matrix ret;

    // all that's left is to find the cover
    auto success = algorithm_x(rows, cols, s);
    // give the result in the form of a matrix
    if (success)
    {
        for (auto& v : s)
        {
            ret[v[0] - 1][v[1] - 1] = v[2];
        }
    }
    return ret;
}


const int test[9][9]{
    { 0, 0, 0, 0, 0, 0, 4, 0, 0 },
    { 3, 0, 6, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 9, 6, 0, 3, 0 },
    { 0, 7, 0, 0, 0, 0, 0, 1, 0 },
    { 8, 0, 0, 2, 5, 0, 0, 9, 0 },
    { 0, 4, 0, 0, 0, 0, 8, 0, 0 },
    { 0, 6, 0, 4, 0, 9, 0, 0, 8 },
    { 0, 0, 5, 0, 0, 0, 0, 2, 0 },
    { 0, 0, 0, 5, 0, 0, 0, 0, 7 },
};

int main()
{
    auto res = xsudoku(test);

    for (auto& l : res)
    {
        bool first = true;
        for (auto& v : l)
        {
            if (!first)
                std::cout << ' ';
            first = false;
            std::cout << v;
        }
        std::cout << '\n';
    }

    return 0;
}
