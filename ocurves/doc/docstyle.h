//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCSTYLE_H_
#define DOCSTYLE_H_

/**
@page style Coding Style

Thus his page documents the coding style guidelines. In all cases, contributors are asked to
adhere to the coding standard. Exhaustively defining a coding style is a difficult and potentially
verbose undertaking. Thus this document focuses on guidelines. When in doubt, honour the existing
style of the source file being edited.

While this document does not address all style issues, it does address common or important
issues. For issues not covered by this document, please refer to the following additional
material:
- <a href="https://google.github.io/styleguide/cppguide.html">The Google C++ style guide.</a>
  This covers a more comprehensive list of issues and provides good justifications.
- <a href="https://wiki.qt.io/Qt_Coding_Style">The Qt style guide.</a> While this style guide
  differs slightly from the Qt style, there is a large amount of overlap.

[TOC]

# Guideline Summary #
- Two space tabs.
- Convert tabs to spaces.
- Use forward declarations and limit includes.
- Maintain const correctness.
- Pointer and reference symbols align with the variable declaration.
- Place spaces around arithmetic and logical operators.
  - Removing whitespace to enhance precedence readability is excepted.
- Brackets have external spacing only when used with branching keywords.
- Strive for zero compiler warnings.
- Order includes (see below).
- One class or structure per header file, nested types excepted.

# Options for astyler #
It is recommended that contributors use <a href="http://astyle.sourceforge.net/">astyle</a> to aid
in adhering to the coding standard formatting guidelines.

    --style=allman
    --indent=spaces=2
    --min-conditional-indent=0
    --max-instatement-indent=40
    --align-pointer=name
    --align-reference=name
    --indent-cases
    --indent-namespaces
    --pad-oper
    --pad-header
    --unpad-paren
    --add-brackets
    --keep-one-line-blocks
    --convert-tabs
    --mode=c


Compatibility Guidelines
========================
C++11 has now had time to mature and is well implemented across a variety of compilers.
Most features are well supported and may be freely used. Some more less common or less
important features are not so widely supported, such as @c consexpr and should still be
avoided.

Cross platform development is fraught with pitfalls. Different compilers make different
assumptions and adhere to standards differently. Different compilers generate different
warnings, define different built in types and may differ in standard header organisation.
Where possible, compile for at least two different platforms to promote improved
compatibility. Visual Studio and G++ are good choices as these represent the most widely
used compilers and have fairly disparate implementations.


Formatting Guidelines
=====================

*Two space tabs, replace tabs with spaces*
Use of real tab characters creates inconsistencies. Spaces are absolute and the alignment
and indentation is invariant regardless of editor settings.

*Place spaces around arithmetic and logical operators.*
Whitespace around arithmetic and logical operators improved readability. One exception
is where operational precedence may be lost in long arithmetic statements. That is,
whitespace may be removed for highest precedence operations to better group those operations.
    // Correct
    int i = a << 1;
    int j = a + 2;
    float y = 2 * x + 1;

    // Incorrect
    float y=x+1;

    // Exception: multiplication precedence enhanced:
    float y = 2*x*x  + 5*x + 1;

*One variable declaration per line.*
    // Correct
    int a;
    int b;
    int *intPtr;
    char *string;

    // Incorrect
    int a, b, *intptr;
    char *string;

*Pointer and reference declarations are adjacent to the variable name.*
Consider the following declaration:
    int* a, b;  // Incorrect
The declaration may be considered ambiguous as the pointer symbol is grouped with the type
declaration, however, it only affects the first variable, @c a, and not the second, @c b.
    int *a, b;  // Less incorrect
The second declaration more concisely identifies the relationship between the pointer
symbol and the affected variable. Note that it is still incorrect by this style guide,
as each variable should be declared on a separate line.

*Brackets have no leading or trailing whitespace for function definitions and calls.*
Function definitions calls do not have whitespace around the brackets.
    // Correct
    int fibonacci(int n);
    int i = fibonacci(5 * 3);

    // Incorrect
    int x = fibonacci( 10 );
    int y = fibonacci (5);


*Open brackets following branching statements have leading whitespace.*
The open bracket '(' is preceded by a space when used with if, switch, for, while.
    // Correct:
    for (int i = 0; i < 10; ++i) ...

    // Incorrect
    if(x == 0) ...
    if( x == 0 ) ...

*Scoping braces on separate lines.*
Open and closing scoping braces appear on new, separate lines. The only exception is
for single statement inline functions.
    // Correct
    if (x == 0)
    {
      return true;
    }
    else if (x > 0)
    {
      logWarning("...");
      return true;
    }
    else
    {
      return false;
    }

    // Incorrect
    if (x == 0) {
      return true;
    } else
    if (x > 0) {
      logWarning("...");
      return true;
    } else { return false; }

*Always use scoping brackets, even for single line content.*
A common source of bugs comes from failing to scope single line if statements, then
attempting to add instructions to the branch. This is easily avoided by always using
scoping brackets, even for single line statements.
    // Correct:
    if (x > 0)
    {
      --x;
    }

    // Incorrect:
    if (x > 0)
      --x;

*Switch statements always have a @c default branch.*
Always include a @c default branch for @c switch statements. Missing default branches
can result in improperly handled situations (and bugs) as the possible switch values
evolve during development. Some compilers rightly generate warnings about missing
default cases.

*Multi-line inline definitions belong outside the class definition.*
Inline functions requiring more than a single statement are to be defined after the class
definition. Only single statement inline functions belong in the class definition and the
function implementation should appear on the same line as the function name.

*The @c inline keyword appears with the function implementation.*
This is relevant only for inline implementations appearing after the class definition.
For such definitions, the @c inline keyword appears with the function implementation,
not in with the function prototype. This is because the inline nature is an implementation
detail and should not be relevant to the user.
    // Correct:
    class A
    {
    public:
      //...
      inline float timeScale() const { return _timeScale; }

      void setTimeScale(float scale);
    }

    inline void A::setTimeScale(float scale)
    {
      if (scale)
      {
        _timeScale = scale;
      }
      else
      {
        _timeScale = 1.0f;
      }
    }

*Member variables are private or protected, preferably private.*
Member variables should generally be private, with public and protected accessor methods
where required. Public members of simple or POD structure are acceptable.

*Avoid assignment in conditional statements.*
Avoid using assignment in if, and loop statement conditions. This can cause side effects.
Assignments in if statements are acceptable where the declaration is also present and
there are no other conditions.
    // Correct
    if (PlotView *view = window->activeView())
    {
      // ...
    }

    // Incorrect
    if ((bytesRead = connection.read()) > 0)
    {
      // ...
    }

Structural Guidelines
=====================

*One header file per class, structure, enumeration.*
In general, each code construct should have it's own header file, named after the contained
construct. While this creates more files, it makes it easier to locate class definitions and
reduces include file overhead for enumerations. Exceptions are allowed for collections of
simple, related definitions which are unlikely to change often, such as in
@c plotexpressionarithmetic.h

*Use forward declarations.*
Always use forward declarations where possible. Implementation details should be hidden
as much as possible. This greatly improves compile times especially as a code base grows.
This means that aggregated pointer members are preferred to non-pointer members for complex
types.

*Limit includes.*
Strive to include the minimal number of headers. Excess include statements lead to
excessive build times. This comes about both due to increased file dependencies and
increased disk IO in large projects.

*Order includes.*
Order include statements by library first, then alphabetically. Include most local library
files first and system headers last. Thus, files in the same directory should be include
first, the closely related library headers graduated to most distant libraries, where
system headers are considered the most distance. Each library block should be separated by
a blank line.

In a header file, the first included header should always be the configuration file for the
current library and each library should have such a configuration header.

System headers should not be included in header files, only in source files. This includes
stl headers. C headers such as \<cstdef\> and \<cinttypes\> are accepted in includes in header
files as they define commonly used types such as @c size_t (when not a built in type) and
various sized integers.

This is designed to promote local types first as these are generally the most pertinent
to the current source. It also serves to avoid troublesome coding styles from interfering
with local definitions. Examples of troublesome coding styles are header files containing
using namespace statements and the Windows API use of the preprocessor to select between
ASCII and Unicode function implementations. These can ambiguate local declarations.

*using namespace statements must not appear in header files.*
Head files with using namespace statements generally result in unintended ambiguities in
dependent files. These can be very difficult to avoid without modifying the offending header
file. Thus using namespace statements are only for source files.

*Maintain @c const correctness.*
Const correctness is an important part of building robust code and is difficult to retrofit.
Guidelines for maintaining const correctness are:
- Prefer const read accessor methods.
- Provide both const and non-const read accessors where required.
- Pass complex types and large POD types by constant reference. Avoids unnecessary
  construction/destruction and stack allocation.

*Limit use of templates.*
Templates are a useful programming tool for defining algorithms independently of specific
types. However, overuse of template programming has several drawbacks:
-# Overuse of templates leads to obfuscation.
-# Errors required greater interpretation.
-# Increased compilation time.

Techniques such as template meta programming, while powerful, are esoteric and require a
good understanding of template and compilers. Compiler errors generated from misuse of
templates are generally more difficult to interpret than other compiler errors.

Lazy instantiation of templates leads to increased work for the compiler. While the overhead
minimal for simple template types, significant come from using many template types, dependent
headers, partial specialisations. Different compilers have different overheads so the potential
overheads may not be readily apparent.

*Shared library exports must have minimal or no dependencies.*
Robust and reliable shared library APIs are a minefield, especially when dealing with with
multiple platforms. DLL hell, is well recognised on Windows platforms, but Unix based platforms
are not immune to share library issues. A full discussion of these issues is well beyond the
scope of this documentation. When exporting shared library symbols;
- Use pure virtual interface or the @em pimple paradigm. Factories are a good way of
  instantiating concrete implementations.
- Use only built in types. This includes avoiding @c stl types as there is no guarantee that
  the user will include the same stl implementation, in which case class sizes and member offsets
  may change.
- All memory allocation and deallocation must be hidden behind the exported API. A consumer
  library may be linking a different runtime which uses different a different resource allocation
  system.
- Avoid @c inline methods on exported types.

Interestingly, recent versions of Visual Studio provided additional compiler warnings about
potential dangers surrounding exporting shared library symbols.

*Hide implementation details.*
This follows on from using forward declarations and from shared library considerations.

*Use the same function order in header and source files.*
The order of declarations in a header file should be matched in the source file. This aids in
navigating between the two files.

*Two blank lines between function implementations.*
This creates better visual separation between scopes.

*Order class scopes : public, protected then private.*
This keeps order promotes increasing detail towards the end of the file. A reader can skim the
just top of the file, knowing the all of the public API is covered and stop once the public
scope is terminated.

*Non-public member variables at the end of classes.*
Member variables appear the the end of the class declarations. This makes it straight forward to
find all the member variables. Public member variables should not be used, with the exception of
public constants, which should appear at the top of the class.

*Member variables ordered by size.*
Order member variables by size (as in @c sizeof), largest to smallest. This can aid in packing
and alignment. This rule has some leniency for who understand alignment and packing.

*Constructors initialise all members, in order.*
Always initialise all member variables when a constructor is provided. Initialisation order should
match the declaration order. This makes it easier to see find omitted initialisations.

Documentation Guidelines
========================

*Use Doxygen style commenting.*
All classes and members should be documented using Doxygen style comments. The preferred
commenting structure is to use the triple slash pattern "///". Member variables may
be described using the back reference pattern "///<" where the documentation comment is
short.

*Use in-code documentation.*
In code documentation should be used. Code is not "self-documenting." Code requires analysis
and interpretation and cannot identify justifications, intent or authoring assumptions. Always
consider the following question; "Will I understand this code after a month of not touching it?"
"Will I think it's rubbish?"

Use code documentation to identify the "why?" of the code. Highlight justifications, intent,
assumptions and boundary conditions.

*Document intent.*
Class and function documentation should describe the intent of the construct. A class
overview should identify the general capabilities then focus on the most commonly used
operations of the class. Defining intended usage may be appropriate in cases where specific
calls sequences are expected.

Specific implementation details are generally not required as the implementation may change
while intent should remain unchanging.

*Document assumptions.*
Provide information about any assumptions made. For example, ordered call sequences,
initialisation order or range limitations. For example, document that null values are
not accepted as this is an assumption made by the underlying code. Assumptions also
relate to undefined behaviour

*Document error and boundary cases.*
Highlight where error cases are handled vs. where undefined behaviour may ensue. For
example, identify whether null pointer values are handled. Similarly, identify what
occurs when out of range values are provided.

*Document ranges.*
Identify acceptable ranges for parameter values where this may not be immediately apparent.
Use range notation: square brackets denote closed intervals (inclusive), while open brackets
are for open intervals (exclusive).

*Document units.*
Consider the declaration and documentation below.
    class Container
    {
    public:
      //...

      /// Gets the size.
      unsigned size() const;

      //...
    };
This is a poor comment for two reasons:
-# It adds nothing which cannot be gleamed from the function name.
-# It does not adequately describe how the size is measured (no units).
This comment raises questions which cannot be answered without looking at the implementation
or empirical testing. Is the size given in bytes or elements? Does the size refer to the

*Document accessor functions.*
Even though the functionality of read and write accessor methods may be readily apparent,
accessors must still be documented. Either the read or write accessor should describe the
semantics of the underlying member variable. The bulk of the documentation belongs with
whichever of the read or write function is more pertinent to class usage (which is more
important to the user?).

Simple accessor documentation may be somewhat repetitive, but it should add information
regarding units and assumptions as described above. Do not assume that usage is self-evident.

*Member variable documentation vs. accessor documentation.*
Accessor and member variable documentation may overlap. Member variables with accessor
methods need only a terse documentation comment, unless internal assumptions differ from
the public assumptions.

Naming Conventions
==================

*Header files have a ".h" suffix.*

*Source files have a ".cpp" suffix.*

*Use camel case for variable and type declarations.*
All declarations are in camelCase capitalising the first letter of each word.
The leading character may or may not be capitalised depending on the declaration
type. See below.

*Type declarations and constant variable declarations begin with a capital letter.*
Class, structure, enumeration and constant declarations begin with a capital letter.

*Only well known acronyms and abbreviations should be used.*
Acronyms and abbreviations in variable names should only be used if they are well
known. Network acronyms such as TCP or UDP and abbreviations such as Msg for message
are well understood and may be used. Abbreviating local names is confusing and to be
avoided. For sample abbreviating "MyTcpConnection" to mtc is ill defined and to be
avoided.

*All names are descriptive.*
All variable, function and type names are to be descriptive, based on the intended usage.
Using terse variable names simply to avoid typing is lazy. Single letter variables, or
abbreviated names are to be avoided, except in well known use cases such as using 'i', 'j'
and 'iter' for loop count iteration.

*Accessor methods are named after the accessed variable.*
A read accessor is named after a variable, while the write accessor is prefixed by 'set' and
the first letter of the accessed variable is capitalised. Examples are below.

Variable        | Read Accessor   | Write Accessor
--------------- | --------------- | ------------------
@c _count       | @c count()      | @c setCount()
@c _plot        | @c plot()       | @c setPlot()
@c _timeColumn  | @c timeColumn() | @c setTimeColumn()
@c _fullName    | @c fullName()   | @c setFullName()

*Imperative function names with identifiable subject.*
Member functions which do something should include an imperative verb to describe the
processing performed and should identify the subject of the operation. Here 'subject'
is defined in grammatical terms and refers to the target of the verb. Member functions
may omit the subject when the operation targets the object itself. The combination of
verb and subject helps define the operation semantics.

Read accessor functions and commonly used and well understood paradigms are accepted,
such as using @c size() for the number of elements in a container. However, note the
documentation style guides for such declarations.

Examples:

Function Name                     | Imperative Verb | Subject (noun)
--------------------------------- | --------------- | ---------------
@c setTimeScale()                 | set             | time scale
@c parseColour()                  | parse           | colour
@c calculateSampleRate()          | calculate       | sample rate
@c PlotFile::loadFile()           | load            | plot file
@c RealTimeTcpConnection::open()  | open            | TCP connection

*Non-public member variables are prefixed with a single underscore '_'.*
This helps distinguish member variables from local and parameter declarations. By contrast,
Hungarian notation tends to be cumbersome.

*Acronyms of three letters or more only have the first letter capitalised.*
Acronym in type and variable declarations have a leading capital letter, then are lower
case. Two letter acronyms may be all caps. The leading letter may be lower case when it
is the first letter of a variable declaration. Examples below.
- <tt>class RTPlot</tt> - two letter acronym, all caps
- <tt>class RealTimeTcpConnection</tt> - Type TCP three letter acronym.
- <tt>Socket *tcpSocket</tt> - TCP used at the start of a variable declaration, first letter lower case.
- <tt>Socket *socketTcp</tt> - TCP used later in a variable declaration.

*Enumeration declarations.*
Global namespace @c enum declarations must be unambiguously named to avoid potential
naming clashes. The general recommendation is to use prefix enumeration members with
an acronym derived from the the enumeration name. This overrides the acronym
conversions discussed elsewhere in this document. Scoped enumerations are free to skip
any prefix as the scope itself defines the context.
    // Correct
    enum GroupId
    {
      GID_None = -1,
      GID_Zoom = 0,
      GID_Tool,
      GID_Count
    };

    class PlotView
    {
    public:
      enum ToolMode
      {
        MultiTool,
        PanTool,
        ZoomTool
      };
    };

    // Incorrect
    enum GroupId
    {
      None = -1,
      Zoom = 0,
      Tool,
      Count
    };

Other Considerations
====================
*Zero compiler warnings.*
Compiler warnings often expose potential issues and should always be addressed. While some
warnings seem spurious, too many compiler warnings create noise in the compiler output and
important warnings are lost amongst the clutter.

Having said that, zero warnings can be difficult when developing cross-platform code and
compiling using different compilers and some warnings are difficult to eliminate. For example,
Microsoft compilers generate security warnings for standard C functions and are not always easily
eliminated. Similarly, Apple's Clang tends to generate warnings about logical operator precedence
which can be considered spurious as the precedence is well defined. Similarly the Clang warnings for
not using the C++11 override keyword may be spurious unless the project was started from with full
C++11 support.

Always address compiler warnings before moving on to keep the noise to a minimum. Disabling warnings
on issues similar to those noted above is reasonable. Disabling type conversion warnings is *not*.

*Checkin messages.*
Messages on checking to source control must be meaningful, starting with a one line summary of
the changes. After that, detail the changes being made. Someone will read the checkin message at
some point.

*/

#endif // DOCSTYLE_H_
