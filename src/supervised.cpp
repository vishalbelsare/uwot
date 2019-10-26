//  UWOT -- An R package for dimensionality reduction using UMAP
//
//  Copyright (C) 2018 James Melville
//
//  This file is part of UWOT
//
//  UWOT is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  UWOT is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with UWOT.  If not, see <http://www.gnu.org/licenses/>.

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector fast_intersection_cpp(const IntegerVector rows,
                                    const IntegerVector cols,
                                    NumericVector values,
                                    const IntegerVector target,
                                    double unknown_dist = 1.0,
                                    double far_dist = 5.0) {
    double ex_unknown = std::exp(-unknown_dist);
    double ex_far = std::exp(-far_dist);

    auto len = values.length();

    for (auto nz = 0; nz < len; ++nz) {
      auto i = rows[nz];
      auto j = cols[nz];
      if (IntegerVector::is_na(target[i]) || IntegerVector::is_na(target[j])) {
        values[nz] = values[nz] * ex_unknown;
      }
      else if (target[i] != target[j]) {
        values[nz] = values[nz] * ex_far;
      }
    }

    return values;
}


// [[Rcpp::export]]
NumericVector general_sset_intersection_cpp(
    const IntegerVector indptr1,
    const IntegerVector indices1,
    NumericVector data1,
    const IntegerVector indptr2,
    const IntegerVector indices2,
    NumericVector data2,
    const IntegerVector result_row,
    const IntegerVector result_col,
    NumericVector result_val,
    double mix_weight = 0.5) {
  
  double left_min = std::max(Rcpp::min(data1) / 2.0, 1.0e-8);
  double right_min = std::max(Rcpp::min(data2) / 2.0, 1.0e-8);
  
  for (auto idx = 0; idx < result_row.length(); idx++) {
    auto i = result_col[idx];
    auto j = result_row[idx];

    auto left_end = indices1.begin() + indptr1[i + 1];
    auto left_it = std::lower_bound(indices1.begin() + indptr1[i], left_end, j);
    double left_val = (left_it!=left_end && *left_it==j ? data1[left_it - indices1.begin()] : left_min);

    auto right_end = indices2.begin() + indptr2[i + 1];
    auto right_it = std::lower_bound(indices2.begin() + indptr2[i], right_end, j);
    double right_val = (right_it!=right_end && *right_it==j ? data2[right_it - indices2.begin()] : right_min);
      
    if (left_val > left_min || right_val > right_min) {
      if (mix_weight < 0.5) {
        result_val[idx] = left_val * std::pow(right_val, (mix_weight / (1.0 - mix_weight)));
      }
      else {
        result_val[idx] = right_val * std::pow(left_val, (((1.0 - mix_weight) / mix_weight)));
      }
    }
  }
  
  return result_val;
}
