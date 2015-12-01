//
// author
//
// Copyright (c) CSIRO 2015
//
#ifndef FUNCTIONSIMPLE_H_
#define FUNCTIONSIMPLE_H_

#include "plotsconfig.h"

#include "functiondefinition.h"

#include <functional>

/// @ingroup expr
/// Defines a function which is implemented by a given function object.
///
/// Supports two kinds of function objects:
/// - @c ValueFunction accepting and returning a single double precision value.
/// - @c ExpandedFunction which allows greater control over the inputs and results.
///
/// Note that a @c ValueFunction is always wrapped up in a
/// function call which accepts all the arguments of @c ExpandedFunction, but only passes
/// through @c argv[0].
class FunctionSimple : public FunctionDefinition
{
public:
  /// Simple implementation parameterisation.
  typedef std::function<double (double)> ValueFunction;
  /// @c ValueFunction typedef to a C function.
  typedef double (*ValueFunctionPtr)(double);

  /// Expanded function parameterisation.
  typedef std::function<void(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info)> ExpandedFunction;
  /// @c ExpandedFunction typedef to a C function.
  typedef void(*ExpandedFunctionPtr)(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info);

  /// Constructor routing the function implementation through a @c ValueFunction.
  /// Expects exactly one argument.
  ///
  /// @param func The function object to call through to from @c evaluate().
  /// @param category The sorting category for this function.
  /// @param name The name of the function. Used to match expressions.
  /// @param description The readable description of the function. Displayed to the user.
  FunctionSimple(const ValueFunction &func, const QString &category, const QString &name, const QString &description = QString());

  /// Constructor routing the function implementation through a @c ExpandedFunction.
  /// @param func The function object to call through to from @c evaluate().
  /// @param category The sorting category for this function.
  /// @param name The name of the function. Used to match expressions.
  /// @param argc The exact number of arguments the function expects, or the minimum number
  ///   if @p variadic is @c true.
  /// @param variadic Is the function variadic, accepting more arguments than just @p argc?
  FunctionSimple(const ExpandedFunction &func, const QString &category, const QString &name, unsigned argc = 0, bool variadic = false);

  /// Constructor routing the function implementation through a @c ExpandedFunction.
  /// @param func The function object to call through to from @c evaluate().
  /// @param category The sorting category for this function.
  /// @param name The name of the function. Used to match expressions.
  /// @param description The readable description of the function. Displayed to the user.
  /// @param argc The exact number of arguments the function expects, or the minimum number
  ///   if @p variadic is @c true.
  /// @param variadic Is the function variadic, accepting more arguments than just @p argc?
  FunctionSimple(const ExpandedFunction &func, const QString &category, const QString &name, const QString &description, unsigned argc = 0, bool variadic = false);

  /// Returns the function object invoked from @c evaluate().
  ///
  /// This will wrap a @c ValueFunction when constructed from such.
  /// @return The function implementation.
  inline ExpandedFunction function() const { return _function; }

  /// Evaluates the function results.
  ///
  /// For a @c ExpandedFunction, this simply invokes that function. For a @c ValueFunction, this
  /// will invoke that function in a more round about way, passing through only @c argv[0].
  /// @param[out] result The evaluation result is written here.
  /// @param time The sample time being evaluated for.
  /// @param argc The number of arguments actually given.
  /// @param argv The list of calculated arguments, length @p argc. These are always logical values.
  /// @param info Additional information about the current sampling.
  /// @param context Additional context of the operation. May hold working data.
  ///     This is optained via @c createContext().
  void evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const override;

protected:
  ExpandedFunction _function; ///< The function object.
};

#endif // FUNCTIONSIMPLE_H_
