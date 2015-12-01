//
// author
//
// Copyright (c) CSIRO 2015
//
#ifndef FUNCTIONDEFINITION_H_
#define FUNCTIONDEFINITION_H_

#include "plotsconfig.h"

#include <QString>

struct PlotFunctionResult;
struct PlotFunctionInfo;

/// @ingroup expr
/// Defines a function which can be used with the @c FunctionRegister.
///
/// The @c FunctionDefinition defines the name, category and parameters as well
/// as supporting execution of the function given a particular set of values.
///
/// During expression evaluation, a @c PlotFunction manages binding the sub-expressions
/// used to evaluate the arguments, evaluates those arguments and calls @c evaluate()
/// with the results.
///
/// Before calling @c evalulate() the @c PlotFunction creates a context via @c createContext().
/// This represents context data used to track the valuation for a particular binding of
/// the function. For example, this may contain a history of previous evaluations.
/// The context is an arbitrary type and is released by calling @c destroyContext().
///
/// A function may also be variadic, requiring a minimum number of arguments, but supporting
/// additional values. For example, the @c minof function supports any number of values,
/// returning the minimum value.
class FunctionDefinition
{
public:
  /// Construct a function definition.
  /// @param category The function's sorting category.
  /// @param name The function's evaluation name. The display name is deduced using @c deduceDisplayName().
  /// @param argc The expected number of arguments. Exact if @p variadic is false, or a minimum otherwise.
  /// @param variadic Is the function variadic?
  FunctionDefinition(const QString &category, const QString &name, unsigned argc = 0, bool variadic = false);

  /// Construct a function definition.
  /// @param category The function's sorting category.
  /// @param name The function's evaluation name. The display name is deduced using @c deduceDisplayName().
  /// @param description The function description.
  /// @param argc The expected number of arguments. Exact if @p variadic is false, or a minimum otherwise.
  /// @param variadic Is the function variadic?
  FunctionDefinition(const QString &category, const QString &name, const QString &description, unsigned argc = 0, bool variadic = false);

  /// Virtual destructor.
  virtual ~FunctionDefinition();

  /// Access the function categorisation for UI grouping.
  /// @return The function category name.
  inline const QString &category() const { return _category; }

  /// Set the category name for the function.
  /// @param category The new category name.
  inline void setCategory(const QString &category) { _category = category; }

  /// Access the function name as used by the expression generator.
  /// @return The function name as used by the expression generator.
  inline const QString &name() const { return _name; }

  /// Set the function name as used by the generator.
  /// @param name The new name.
  inline void setName(const QString &name) { _name = name; }

  /// Return the function name as displayed with the user. This better
  /// reflects how the function is used and should include parameter names
  /// and brackets for the parameter list.
  ///
  /// @see @c deduceDisplayName
  /// @return The function name as displayed to the user.
  inline const QString &displayName() const { return _displayName; }

  /// Set the function name as displayed to the user.
  /// @param name The new name.
  inline void setDisplayName(const QString &name) { _displayName = name; }

  /// Access the function description.
  /// @return The function description.
  inline const QString &description() const { return _description; }

  /// Set the function description.
  /// @param description The new description.
  inline void setDesciption(const QString &description) { _description = description; }

  /// Access the minimum argument count.
  /// @return The minimum number of arguments required.
  inline unsigned argc() const { return _argc; }

  /// Set the minimum required arguments.
  /// @param count the Minimum argument count.
  inline void setArgc(unsigned count) { _argc = count; }

  /// Is the function variadic beyond @c argc()?
  /// @return True if the function can accept additional arguments.
  inline bool variadic() const { return _variadic; }

  /// Set the variadic flag.
  /// @param variadic True if the function can accept additional arguments.
  inline void setVariadic(bool variadic) { _variadic = variadic; }

  /// Evaluate the function with the given context data.
  ///
  /// This must look at the current state in @p info and evaluate results to @p result.
  ///
  /// @param[out] result The evaluation result is written here.
  /// @param time The sample time being evaluated for.
  /// @param argc The number of arguments actually given.
  /// @param argv The list of calculated arguments, length @p argc. These are always logical values.
  /// @param info Additional information about the current sampling.
  /// @param context Additional context of the operation. May hold working data.
  ///     This is optained via @c createContext().
  virtual void evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const = 0;

  /// Called to create an operating context for calculating function values.
  ///
  /// The context may be any type and represents working data required for the function.
  /// For example, this may hold a window of previous values for calculating a running average.
  /// The returned context is passed to @c evaluate().
  ///
  /// The context is destroyed by @c destroyContext().
  ///
  /// The default implementation returns null.
  /// @return Working context data.
  virtual void *createContext() const;

  /// Destroys the context created by @c createContext().
  ///
  /// The default implementation is empty.
  /// @param context The context created in @c createContext().
  virtual void destroyContext(void *context) const;

  /// Deduces the display name of the function to show usage.
  ///
  /// This takes the function name, adds brackets and a list of sequential parameter
  /// terms based on the number of expected parameters. Examples are below.
  ///
  /// Name      | @c argc() | @c variadic() | Result
  /// --------- | --------- | ------------- | --------------------
  /// abs       | 1         | false         | abs(x)
  /// sqrt      | 1         | false         | abs(x)
  /// abserr    | 2         | false         | abserr(x,y)
  /// noargs    | 0         | false         | noargs()
  /// onearg    | 1         | false         | onearg(x)
  /// twoargs   | 2         | false         | twoargs(x,y)
  /// threeargs | 3         | false         | threeargs(x,y,z)
  /// fourargs  | 4         | false         | fourargs(a,b,c,d)
  /// maxofFunc | 1         | true          | maxof(x...)
  ///
  /// For three or less parameters, they are labelled x, y, z respectively.
  /// For more parameters, they are labelled a, b, c, ... to the required
  /// number of parameters. Finally, '...' is appended for variadic functions.
  QString deduceDisplayName() const;

private:
  QString _category;    ///< Sorting category.
  QString _name;        ///< Evaluation name.
  QString _displayName; ///< Display name.
  QString _description; ///< Description.
  unsigned _argc;       ///< Argument count.
  bool _variadic;       ///< Variadic?
};

#endif // FUNCTIONDEFINITION_H_
