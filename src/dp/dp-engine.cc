/*
   Copyright (C) 2004-2007,2010 Benjamin Redelings

This file is part of BAli-Phy.

BAli-Phy is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

BAli-Phy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with BAli-Phy; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

/**
 * @file dp-engine.C
 *
 * @brief This file contains a generic dynamic programming class.
 *
 */

#include <iostream>
#include "dp-engine.H"
#include "util/myexception.H"

using std::vector;
using std::cerr;
using std::endl;

log_double_t DPengine::Pr_sum_all_paths() const {
  return Pr_total;
}

void DPengine::check_sampling_probability(const vector<int>& g_path) const
{
  log_double_t P = path_P(g_path);
  log_double_t ratio = path_Q(g_path)/Pr_sum_all_paths();
  double diff = std::abs(log(ratio) - log(P));
  if (std::abs(diff) > 1.0e-9) {
    throw myexception()
      <<" Incorrect sampling probabilities!\n"
      <<" P(sample) = "<<log(P)<<"     P(path)/P(ALL paths) = "<<log(ratio)<<"   diff = "<<diff;
  }
}

DPengine::DPengine(const HMM& H)
  :dp_HMM(H),
   Pr_total(0)
{ }
