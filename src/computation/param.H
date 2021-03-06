#ifndef PARAM_H
#define PARAM_H

#include <optional>
#include "computation/expression/expression_ref.H"
#include "util/bounds.H"

class context;

class param
{
    std::optional<int> head;
    std::optional<expression_ref> value;

public:
    expression_ref ref(const context& C) const;

    expression_ref get_value(const context& C) const;
    void set_value(context& C, const expression_ref& v) const;

    std::optional<expression_ref> constant_value() const {return value;}
    std::optional<int> is_modifiable(const context& C) const;
    std::optional<int> is_random_variable(const context& C) const;
    std::optional<bounds<double>> has_bounds(const context& C) const;

    param& operator=(const param&) = default;

    explicit operator bool() const {return head or value;}

    param() {}
    param(int h):head(h) {}
    param(const expression_ref& v):value(v) {}
};

param get_param(context& C, const expression_ref& E);

class ConstParam
{
    param x;
    const context* C;
public:
    expression_ref ref() const;

    expression_ref get_value() const;

    std::optional<expression_ref> constant_value() const;

    std::optional<int> is_modifiable() const;

    std::optional<int> is_random_variable() const;
    std::optional<bounds<double>> has_bounds() const;

    explicit operator bool();

    ConstParam();
    ConstParam(param, const context*);
};

class Param
{
    param x;
    context* C;
public:
    expression_ref ref() const;

    expression_ref get_value() const;
    void set_value(const expression_ref& v) const;

    std::optional<expression_ref> constant_value() const;
    std::optional<int> is_modifiable() const;
    std::optional<int> is_random_variable() const;
    std::optional<bounds<double>> has_bounds() const;

    explicit operator bool();

    Param();
    Param(param, context*);
};

#endif
