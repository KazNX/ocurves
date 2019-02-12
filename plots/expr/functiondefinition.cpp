//
// author
//
// Copyright (c) CSIRO 2015
//
#include "functiondefinition.h"

#include <QTextStream>

FunctionDefinition::FunctionDefinition(const QString &category, const QString &name, unsigned argc, bool variadic)
  : _category(category)
  , _name(name)
  , _argc(argc)
  , _variadic(variadic)
{
  _displayName = deduceDisplayName();
}


FunctionDefinition::FunctionDefinition(const QString &category, const QString &name, const QString &description, unsigned argc, bool variadic)
  : _category(category)
  , _name(name)
  , _description(description)
  , _argc(argc)
  , _variadic(variadic)
{
  _displayName = deduceDisplayName();
}


FunctionDefinition::~FunctionDefinition()
{
}


void *FunctionDefinition::createContext() const
{
  return nullptr;
}


void FunctionDefinition::destroyContext(void * /*context*/) const
{

}

QString FunctionDefinition::deduceDisplayName() const
{
  QString display;
  QTextStream stream(&display);
  stream << name() << '(';
  if (argc())
  {
    if (argc() <= 3)
    {
      const char *names = "xyz";
      stream << names[0];
      for (unsigned i = 1; i < argc(); ++i)
      {
        stream << ',' << names[i];
      }
    }
    else
    {
      const char *names = "abcdefghijklmnopqrstuvwxyz";
      const unsigned namesCount = unsigned(strlen(names));
      stream << names[0];
      for (unsigned i = 1; i < argc() && i < namesCount; ++i)
      {
        stream << ',' << names[i];
      }

      // It would be strange to have more arguments than namesCount above, but
      // handle it, just in case.
      if (argc() > namesCount)
      {
        for (unsigned i = namesCount; i < argc(); ++i)
        {
          stream << ",.";
        }
      }
    }
  }

  if (variadic())
  {
    stream << "...";
  }

  stream << ')';

  return display;
}
