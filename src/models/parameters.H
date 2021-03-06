/*
  Copyright (C) 2004-2012 Benjamin Redelings

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

///
/// \file parameters.H
///
/// \brief This file defines the Parameters class which holds the model and state
///        for the MCMC.
///

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <algorithm>                                // for copy
#include <boost/smart_ptr/intrusive_ptr.hpp>        // for intrusive_ptr
#include <boost/filesystem.hpp>                     // for boost::filesystem::path
#include <memory>                                   // for shared_ptr, alloc...
#include <optional>                                 // for optional
#include <string>                                   // for string
#include <tuple>                                    // for tuple
#include <vector>                                   // for vector
#include "TreeInterface.H"                          // for TreeInterface
#include "alignment/alignment.H"                    // for alignment
#include "computation/expression/expression_ref.H"  // for expression_ref
#include "computation/object.H"                     // for Object, intrusive...
#include "computation/param.H"                      // for param
#include "models/model.H"                           // for Model::key_map_t
#include "sequence/sequence.H"                      // for sequence
#include "tools/partition.H"                        // for Partition
#include "tree/sequencetree.H"                      // for SequenceTree
#include "tree/tree.H"                              // for tree_edge
#include "util/assert.hh"                           // for assert
#include "util/math/log-double.H"                   // for log_double_t
#include "util/matrix.H"                            // for Matrix, matrix
#include "imodel/imodel.H"
class Likelihood_Cache_Branch;
class Parameters;
class alphabet;
class context;
class model_t;
class module_loader;
class pairwise_alignment_t;

//------------------------------- parameter-containing class --------------------------//

struct data_partition_constants
{
    /// The pairwise alignment for each branch
    std::vector<param> pairwise_alignment_for_branch;

    param likelihood_index;

    param cl_index;

    param ancestral_sequences_index;

    std::vector<int> conditional_likelihoods_for_branch;

    std::vector<param> leaf_sequence_indices;

    std::vector<param> sequence_length_indices;

    std::vector<param> sequence_length_pr_indices;

    std::vector<param> transition_p_method_indices;

    std::vector<param> branch_HMMs;

    param alignment_prior_index;

    /// The names and comments for the sequences
    std::vector< sequence > seqs;

    /// The leaf sequence data of this partition
    std::vector< std::vector<int> > sequences;

    std::shared_ptr<alphabet> a;

    std::vector<int> branch_HMM_type;

    /// Alignment constraint
    matrix<int> alignment_constraint;

    int likelihood_calculator;

    data_partition_constants(Parameters* P, int i, const alphabet&, int like_calc);
};

/// Each data_partition is a model with one parameter: mu (the branch mean)
class data_partition
{
    friend class Parameters;
  
public:
    const Parameters* P;

protected:
    int partition_index;

    const data_partition_constants& DPC() const;

    int likelihood_calculator() const;

public:

    /// Is the alignment allowed to vary?
    bool variable_alignment() const;

    alignment A() const;                        // used in dp-alignment-sums.cc (debug), mcmc/logger.cc (log), sample-alignment.cc (debug), sample-node.cc (debug), mcmc/setup.cc [fraction_non_gap]

    TreeInterface t() const;

    /// Cached Transition Probabilities
    const EVector& transition_P(int b) const;   // used only in sample_subst_history( ) in substitution.cc
  
    const EVector& get_sequence(int i) const;   // used in substitution.cc [get_leaf_seq_likelihoods, sample_subst_history], parsimony.cc [peel_muts_leaf_branch, accumulate_root_leaf]

    int n_base_models() const;                  // used in substitution.CC for get_letter_likelihoods, get_leaf_seq_likelihoods, get_column_likelihoods, sample_subst_history

    int n_states() const;                       // used in substitution.CC for get_letter_likelihoods, get_leaf_seq_likelihoods, get_column_likelihoods, sample_subst_history

    // Perhaps this should also be per-mixture-component.
    EVector state_letters() const;
  
    Matrix WeightedFrequencyMatrix() const;     // used in sample-alignment.cc, sample-cube.cc, sample-tri.cc, substitution.cc[sample_subst_history]

    const Likelihood_Cache_Branch& cache(int b) const;  // used in get_column_likelihoods( ), other_subst( ), sample_subst_history( )

    int subst_root() const;

    /// cached branch HMMs
    const indel::PairHMM& get_branch_HMM(int b) const;
    std::vector<indel::PairHMM> get_branch_HMMs(const std::vector<int>&) const;

    double sequence_length_pr(int i) const;     // used in dp/3way.cc, dp/5way.cc, dp/alignment-sums.cc

    double get_beta() const;

    // get the alphabet for partition i
    const alphabet& get_alphabet() const;

    bool has_IModel() const;

    int seqlength(int n) const;

    expression_ref get_pairwise_alignment_(int b) const;

    const pairwise_alignment_t& get_pairwise_alignment(int b) const;

    bool pairwise_alignment_is_unset(int b) const;

    const matrix<int>& alignment_constraint() const;

    EVector ancestral_sequences() const;

    // The prior, likelihood, and probability
    log_double_t prior_alignment() const;
    log_double_t likelihood() const;
    log_double_t heated_likelihood() const;

    data_partition(const Parameters* P, int i);
};

struct mutable_data_partition: public data_partition
{
    void set_alignment(const alignment&);

    void unalign_sequences();

    void set_pairwise_alignment(int b, const pairwise_alignment_t&);

    void unset_pairwise_alignment(int b);

    mutable_data_partition(const Parameters* P, int i);
};

struct smodel_methods
{
    int main;
    int n_base_models;
    int n_states;
    int state_letters;
    int weighted_frequency_matrix;
    int get_alphabet;
    int rate;
  
    // also:
    // Q1: can we use evaluate_expression( ) to pass in arguments?  what are the pitfalls?
    // Q2: how could we control the peeling from haskell?
    // 
    smodel_methods(const expression_ref&, context&);
};

struct tree_constants: public Object
{
    // The tree
    int tree_head = -1;
    std::vector<std::vector<param>> parameters_for_tree_node;
    std::vector<std::tuple<param,param,param>> parameters_for_tree_branch;

    std::vector<param> branch_durations;

    int n_leaves;

    std::vector<std::string> node_labels;
  
    void register_branch_lengths(context*, const expression_ref&);

    tree_constants* clone() const {return new tree_constants(*this);}

    tree_constants(context*, const std::vector<std::string>&, int);
};

struct parameters_constants: public Object
{
    param heat;
    param variable_alignment;
    param subst_root;
    param atmodel_export;
    param atmodel;
    param partition_likelihoods;
    param partition_cond_likes;
    param partition_ancestral_seqs;
    param partition_transition_ps;
    param imodel_training;
    param sequence_names;

    std::vector<param> branch_categories;

    /// The substitution::Model
    std::vector<smodel_methods> SModels;
    std::vector<std::optional<int>> smodel_for_partition;

    /// The IndelModel
    std::vector<std::optional<int>> imodel_for_partition;
    int n_imodels;
    param imodels_param;

    // Scale for partition
    std::vector<std::optional<int>> scale_for_partition;
    int n_scales;
    std::vector<param> branch_scales_;

    /// Do we have an Exponential (0) or Gamma-0.5 (1) prior on branch lengths?
    int branch_prior_type = 0;

    /// Topology and Branch-Length Constraint
    SequenceTree TC;

    /// Alignment-constrained branches
    std::vector<int> AC;

    std::vector<int> branch_HMM_type;

    /// Tree partitions to weight
    std::vector<Partition> partitions;
    std::vector<log_double_t> partition_weights;

    /// Is pinning enabled or not?
    std::vector<double> constants;
    /// Is pinning enabled or not?
    int features = 0;

    std::vector<double> beta_series;

    std::vector<double> all_betas;

    std::vector<data_partition_constants> DPC;

    parameters_constants* clone() const {return new parameters_constants(*this);}

    parameters_constants(int n_partitions, const SequenceTree&, 
			 const std::vector<std::optional<int>>&,
			 const std::vector<std::optional<int>>&,
			 const std::vector<std::optional<int>>&);
};

/// A class to contain all the MCMC state except the alignment
class Parameters: public Model
{
    friend class data_partition;
    friend class TreeInterface;
public:
    object_ptr<parameters_constants> PC;
    object_ptr<tree_constants> TC;

    int beta_index;

    bool variable_alignment_;

private:

    std::vector<int> affected_nodes;
    std::vector<std::vector<int>*> branches_from_affected_node;

    bool variable_alignment_from_param() const;

public:

            data_partition get_data_partition(int i) const;

    mutable_data_partition get_data_partition(int i);

    int n_data_partitions() const {return PC->DPC.size();}

            data_partition operator[](int i) const {return get_data_partition(i);}

    mutable_data_partition operator[](int i)       {return get_data_partition(i);}

    std::optional<int> smodel_index_for_partition(int i) const {return PC->smodel_for_partition[i];}

    std::optional<int> imodel_index_for_partition(int i) const {return PC->imodel_for_partition[i];}

    std::vector<int> partitions_for_imodel(int i) const;

    std::vector<int> partitions_for_scale(int i) const;

    std::optional<int> scale_index_for_partition(int i) const {return PC->scale_for_partition[i];}
  
    TreeInterface t() const;

    std::vector<std::string> get_labels() const;
  
    void reconnect_branch(int s1, int t1, int t2);
    void prune_subtree(const tree_edge& b_subtree);
    void regraft_subtree(const tree_edge& b_subtree, const tree_edge& b_target);
    void begin_modify_topology();
    void end_modify_topology();

private:
    void exchange_subtrees(int b1, int b2);
public:
    void NNI(const tree_edge& b1, const tree_edge& b2, bool disconnect_subtree=false);
    void NNI(int b1, int b2, bool disconnect_subtree=false);
    void show_h_tree() const;

    /// Did we most recently hit beta==1 (1) or beta=0 (0)
    int updown;
    int branch_prior_type() const {return PC->branch_prior_type;}
    int branch_HMM_type(int b) const {
	assert(0 <= b and b < PC->branch_HMM_type.size());
	return PC->branch_HMM_type[b];
    }
  
    // The prior, likelihood, and probability
    log_double_t prior_no_alignment() const;
    log_double_t prior_alignment() const;

    void cache_likelihood_branches() const;

    /// How many substitution models?
    int n_smodels() const {return PC->SModels.size();}
  
    /// How many substitution models?
    int n_imodels() const {return PC->n_imodels;}

    /// How many substitution models?
    int n_branch_scales() const {return PC->n_scales;}

    /// Set branch 'b' to have length 'l', and compute the transition matrices
    void setlength_unsafe(int b, double l);
    void setlength(int b,double l); 
  
    // void
    expression_ref my_tree() const;
    // void
    expression_ref my_atmodel() const;
    expression_ref my_atmodel_export() const;

    expression_ref my_partition_likelihoods() const;
    expression_ref my_partition_cond_likes() const;
    expression_ref my_partition_transition_ps() const;
    expression_ref my_partition_ancestral_sequences() const;

    // bool
    expression_ref my_variable_alignment() const;
    bool variable_alignment() const;
    void variable_alignment(bool b);

    // int
    expression_ref my_subst_root() const;
    void select_root(int b) const;
    void set_root_(int n) const;
    void set_root(int n) const;
    int subst_root() const;

    // int
    int  get_branch_category(int b) const;
    void set_branch_category(int b, int cat);

    // double
    expression_ref heat_exp() const;
    void set_beta(double b);
    double get_beta() const;

    // bool
    expression_ref imodel_training_exp() const;
    bool get_imodel_training() const;
    void set_imodel_training(bool) const;

    // double
    const param& branch_scale(int i) const;
    double get_branch_scale(int i) const;
    void set_branch_scale(int i,double);

    double branch_mean() const;

    std::string name() const {return "Full Model";}

    Parameters* clone() const {return new Parameters(*this);}

    Parameters& operator=(const Parameters&) = default;
    Parameters& operator=(Parameters&&) = default;

    Parameters(const Parameters&) = default;
    Parameters(Parameters&&) = default;

    Parameters(const Program& P,
	       const key_map_t&,
	       const std::vector<alignment>& A,
	       const std::vector<std::pair<std::string,std::string>>& filename_ranges,
               const SequenceTree&,
	       const std::vector<std::optional<int>>&,
	       const std::vector<std::optional<int>>&,
	       const std::vector<std::optional<int>>&,
	       const std::vector<int>&);
};

std::string write_newick(const Parameters&, bool=true);

expression_ref get_alphabet_expression(const alphabet& a);

Program gen_atmodel_program(const std::shared_ptr<module_loader>& L,
                            const Model::key_map_t& k,
                            const boost::filesystem::path& program_filename,

                            const std::vector<expression_ref>& alphabet_exps,
                            const std::vector<std::pair<std::string,std::string>>& filename_ranges,
                            int n_leaves,
                            const std::vector<model_t>& SMs,
                            const std::vector<std::optional<int>>& s_mapping,
                            const std::vector<model_t>& IMs,
                            const std::vector<std::optional<int>>& i_mapping,
                            const std::vector<model_t>& scaleMs,
                            const std::vector<std::optional<int>>& scale_mapping,
                            const model_t& branch_length_model,
                            const std::vector<int>& like_calcs);

#endif
