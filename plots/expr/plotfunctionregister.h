//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __PLOTFUNCTIONREGISTER_H_
#define __PLOTFUNCTIONREGISTER_H_

#include "plotsconfig.h"

#include "plotfunction.h"

#include "functionsimple.h"

#include <QStringList>

/// @ingroup expr
/// A @c PlotFunctionRegister stores function implementations to expose to the plot expression generator.
///
/// These functions represent arbitrary operations which can be performed in a plot expression.
/// Function implementations are free to perform whatever calculations they wish (within reason)
/// based on the given inputs. For example, the max() function tracks the running maximum of the input
/// expression.
///
/// Function implementations are added by calling @c add().
class PlotFunctionRegister
{
public:
  /// Constructor - optinally register default/built-in functions.
  ///
  /// Built-in functions can be registered later by calling @c registerDefaultFunctions().
  /// @param registerDefaults True to register the default/build-in functions.
  PlotFunctionRegister(bool registerDefaults = true);

  /// Destructor. Destroys all functions which the register owns.
  ~PlotFunctionRegister();

  /// Add a function to the register using a @c FunctionSimple object with a @c FunctionSimple::ValueFunction implementation.
  /// @param function The function pointer handling calculation of the results.
  /// @param category The category for the function, used for grouping in the UI.
  /// @param name The function name. Used as the function identifier. Follows identifier rules for valid characters.
  /// @param description Description shown to the user. Cannot be translated yet.
  /// @return The @c FunctionSimple object instantiated to invoke @p function, or null on failure.
  FunctionSimple *add(FunctionSimple::ValueFunctionPtr function, const QString &category, const QString &name, const QString &description);

  /// Add a function to the register using a @c FunctionSimple object with a @c FunctionSimple::ExpandedFunction implementation.
  /// @param function The function pointer handling calculation of the results.
  /// @param category The category for the function, used for grouping in the UI.
  /// @param name The function name. Used as the function identifier. Follows identifier rules for valid characters.
  /// @param argc The minimum number of arguments the function requires.
  /// @param description Description shown to the user. Cannot be translated yet.
  /// @param variadic Set to true if the function is variadic and supports a variable number of parameters
  ///     after @p argc.
  /// @return The @c FunctionSimple object instantiated to invoke @p function, or null on failure.
  FunctionSimple *add(FunctionSimple::ExpandedFunctionPtr function, const QString &category, const QString &name, const QString &description, unsigned argc = 0, bool variadic = false);

  /// A a function by object.
  /// @param functionDef The function definition. This class takes ownership of the pointer if @p takeOwnership is true.
  /// @param takeOwnership True if this class will delete @p functionDef (call delete).
  /// @return The @c FunctionDefinition object @p functionDef on success or null on failure.
  FunctionDefinition *add(FunctionDefinition *functionDef, bool takeOwnership = true);

  /// Search for a function by name : exact match with @c FunctionDefinition::name().
  /// @param name The name to match.
  /// @return The matched function, or null on failure.
  const FunctionDefinition *find(const QString &name) const;

  /// Register built in functions. May have already been called from the constructor.
  void registerDefaultFunctions();

  /// Fetch all registered functions.
  /// @return A list of registered function definitions.
  inline const QVector<FunctionDefinition *> &definitions() const { return _definitions; }

  /// Return the list of category names which covers all registered functions.
  /// @return The category list.
  inline const QStringList categories() const { return _categories; }

  /// Request definitions in a particular category.
  /// @param categoryName The category to match (exact match).
  /// @param definitions Matching definitions are added to this container.
  /// @return The number of items added to @p definitions.
  unsigned getDefinitionsInCategory(const QString &categoryName, QVector<const FunctionDefinition *> &definitions) const;

protected:
  /// Ensure that @p categoryName is present in the list of categories.
  /// @param categoryName The category name to ensure is present.
  void ensureCategoryIsPresent(const QString &categoryName);

private:
  QVector<FunctionDefinition *> _definitions;
  QStringList _categories;  ///< List of registered category names.
  QVector<bool> _ownership;
};

#endif // __PLOTFUNCTIONREGISTER_H_
