"""This script parses a CMake script and extract function and macro comments into a Doxygen
friendly format. It is designed to be run as a Doxygen filter.

Essentially, the script reads comments preceding CMake function and macro declarations and
outputs C style function declarations with equivalent Doxygen style comments.

See usage for more details.
"""
import argparse
import glob
import os.path
# Requires replacement regex package as re can get hung up in parsing.
import regex as re
import shutil
import subprocess
import sys

# out = open(r'D:\temp\out.c', 'w')
# out.write("XXX:\n")
# for arg in sys.argv:
#     out.write(arg + '\n')
# sys.exit(1)


# Regular expression constants.
# Comment match: any whitespace, a '#' character, then any non-newline characters.
CommentRePart = r'([ \t]*#[ \t]?[^\r\n]*)'
# Function definition (includes ignoring various whitespace):
# - 'function' or 'macro' keyword (case insensitive)
# - Parameter list
FuncRePart = r'(?<![Ee][Nn][Dd])((?i)function|macro)[ \t]*\((?:[ \t]*(\S+)[ \t]*(.*))\)'
# Variable definition. Matches "SET" and "OPTION" statements and custom 'make_option' macro.
# - 'set', 'option' or 'make_option' ignoring case.
# - Extract variable name, then ignore what follows.
SetRePart = r'((?i)set|option|make_option)[ \t]*\((?:[ \t]*([_a-zA-Z][_a-zA-Z0-9]*)[ \t]*([^ \t\n]*[ \t]*)*)\)'

def extractCommentLine(comment):
    """Extract the comment text from a comment line. This removes preceeding comment characters.
    :param comment: The comment line to extract.
    :return: The extracted comment without leading whitespace or comment characters '#'.
    """
    # Extract lines with start with # after any whitespace (only).
    # Group what follows the comment character.
    extractRe = re.compile(r'^[ \t]*#[ \t]?([^\r\n].*)[\r\n]?$')
    match = extractRe.match(comment)
    if match is not None:
        line = match.group(1)
        # line = line.replace('\\', '\\')
        # line = line.replace('>', '\\>')
        # line = line.replace('<', '\\<')
        # line = line.replace('@', '\\@')
        return line
    return ""

def ignoreComment(comment):
    """Check if the given comment should be ignored.

    Ignores comment text of the form:
    '#####################################'
    or
    '#------------------------------------'
    or
    '#===================================='
    or any combination of '#-=' characters.
    :param comment: The comment string.
    :return: True if it should be ignored.
    """
    # Match comments as described above.
    ignoreRe = re.compile(r'^[ \t]*#[ \t]?[#=-]+[ \t]*$')
    return ignoreRe.match(comment) is not None

def parseFileComment(content):
    """
    Parse and extract the comment block at the top of the file.
    :param content: The file content
    :return: The extracted comment block at the top of the file or an empty
    string when there are no such comments.
    """
    # Expression to match start and end anchors to lines, rather than entire content.
    # (?m) is the multi-line mode modifier.
    lineRe = re.compile(r'(?m)^.*$')
    # Match preceding comment lines and blank lines.
    commentRe = re.compile(CommentRePart + '|((?m)^[ \t]$)')

    # First find leading comments in the file. I.e., keep adding comments until we find a non-comment, non-empty line.
    commentsDone = False
    ignoreBlankLines = True # Used to stop after the first comment block (first blank or non-comment stops).
    searchOffset = 0
    fileComments = ''
    while not commentsDone:
        match = lineRe.search(content, searchOffset)
        if match:
            line = match.group(0)
            # Check this a valid comment line or blank
            if len(line) == 0 and ignoreBlankLines or commentRe.match(line):
                # Ignore if this a comment line of the form '#-------'
                if not ignoreComment(line):
                    # We have a valid comment line. Start ignoring blank lines if needed.
                    ignoreBlankLines = len(line) > 0
                    # Extract and append the comment from the line.
                    fileComments += extractCommentLine(line) + '\n'
            else:
                commentsDone = True
            searchOffset = match.end() + 1

    return fileComments


def parseFunctions(content):
    """
    Parse functions in the given file content string.
    :param content: The CMake script content to parse.
    :return: A string of extracted function comment blocks in Doxygen form.
    """
    functionBlocks = []
    # Line matching expression looking for commented function definitions.
    # Extracts functions which aren't commented too.
    funcRe = re.compile('(?m)((^' + CommentRePart + '$\n)*)^[ \t]*(' + FuncRePart + ')')
    # Find functions and macros with their leading comments.
    funcMatches = funcRe.finditer(content)
    for match in funcMatches:
        functionBlocks.append(processFunction(match.group()))
    return functionBlocks


def processFunction(match):
    """
    Process a match from parseFunctions(), converting it to a Doxygen macro.
    :param match: The matched string containing the function, parameters and leading comments.
    :return: The function converted into a C style macro definition with leading Doxygen comments.
    """
    functionString = ''
    commentRe = re.compile(CommentRePart)
    funcRe = re.compile(FuncRePart)
    funcMatch = funcRe.search(match)
    if not funcMatch:
        return ''
    isMacro = funcMatch.group(1).lower() == 'macro'
    # The function/macro name (first parameter in CMake terms)
    name = funcMatch.group(2)
    # group 4 is full parameter list with leading bracket. We slice to remove the leading bracket and
    # function name (latter in group 5 with trailing spaces)
    params = funcMatch.group(3)
    # We define functions and macros and C macros for Doxygen. That way
    # parameter types aren't expected.
    functionString += '/// @def {0}({1})\n'.format(name, params)
    prefix = ''
    if isMacro:
        prefix = 'macro: '
    else:
        prefix = 'function: '
    comments = commentRe.findall(match)
    for commentMatch in comments:
        if not ignoreComment(commentMatch):
            functionString += '/// {0}{1}'.format(prefix, extractCommentLine(commentMatch)) + '\n'
            prefix = ''
    functionString += '#define {0}({1})\n'.format(name, params)
    return functionString


def parseVariables(content):
    """
    Parse variable declarations (SET, OPTION, make_option) in the given file content string.
    :param content: The CMake script content to parse.
    :return: A string of extracted function comment blocks in Doxygen form.
    """
    varBlocks = []
    # Extract documented variables that are not indented.
    varRe = re.compile('(?m)((^' + CommentRePart + '$)\n+)^(' + SetRePart + ')')
    # Find functions and macros with their leading comments.
    results = varRe.finditer(content)
    for match in results:
        varBlocks.append(processVariable(match.group()))
    return varBlocks

def processVariable(match):
    """
    Process a match from parseVariables(), converting it to a Doxygen macro.
    :param match: The matched string containing the variable and leading comments.
    :return: The variable converted into a C style macro definition with leading Doxygen comments.
    """
    variableString = ''
    commentRe = re.compile(CommentRePart)
    varRe = re.compile(SetRePart)
    varMatch = varRe.search(match)
    if not varMatch:
        return ''
    name = varMatch.group(2)
    isOption = varMatch.group(1).endswith('option')

    # Define a C style macro for the variable.
    variableString = '/// @def {0}\n'.format(name)
    prefix = ''
    if isOption:
        prefix = 'option: '
    comments = commentRe.findall(match)
    for commentMatch in comments:
        if not ignoreComment(commentMatch):
            variableString += '/// {0}{1}'.format(prefix, extractCommentLine(commentMatch)) + '\n'
            prefix = ''
    variableString += '#define {0}\n'.format(name)
    return variableString


def convertCMakeFile(content, scriptFileName, outFile):
    """
    Parse a CMake script and convert it to a C style header with Doxygen comments.

    This function reads a CMake script from the content string, parsing it looking
    for commented SET() and OPTION() statements, macro and function declarations. Such
    declarations are extracted and converted into C macro declarations (#define statements)
    with the leading comments added in Doxygen style. Uncommented variables are not included,
    while uncommented functions and macros are.

    All definitions are added to a Doxygen group defined by the scriptFileName, using
    the file base name. Additionally, if the script is a "FindPackage" script, it is added
    the script group is nested within the "Packages" group.

    Example - Variable Declaration:
    Input:
        # Build with additional debug logging?
        set(ENABLE_DEBUG_LOGGING NO)

    Output:
        /// @def ENABLE_DEBUG_LOGGING
        /// Build with additional debug logging?
        #define ENABLE_DEBUG_LOGGING

    Example - Function Declaration:
    Input:
        #===============================================================================
        # Tests a configuration name to see if it is a debug configuration.
        # By default, the only debug configuration is "DEBUG" unless the global property
        # DEBUG_CONFIGURATIONS is specified. This function handles both cases, with
        # the property taking precedence.
        #===============================================================================
        function(is_debug_config config var)

    Output:
        /// @def is_debug_config config(var)
        /// function: Tests a configuration name to see if it is a debug configuration.
        /// By default, the only debug configuration is "DEBUG" unless the global property
        /// DEBUG_CONFIGURATIONS is specified. This function handles both cases, with
        /// the property taking precedence.
        #define is_debug_config config(var)

    :param content: A string object containing the CMake script.
    :param scriptFileName: The name of the script file.
    :param outFile: A file-like object to write output to.
    """
    # Create a Doxygen group based on the file name.
    groupName = os.path.splitext(scriptFileName)[0]
    isFindModule = groupName.startswith("Find")
    if isFindModule:
        # Add "FindPackage" scripts to the Packages group.
        outFile.write('/*! @addtogroup Packages\n')
        # Bracket what follows in the Packages group
        outFile.write('  @{\n')
        outFile.write(' */\n')
    # Define the group for the file.
    outFile.write('/*!\n')
    outFile.write('  @defgroup {0} {0}\n'.format(groupName))
    # Add comments at the to of the file as the group description.
    fileComments = parseFileComment(content)
    if len(fileComments) > 0:
        outFile.write(fileComments)
    # Note the source.
    outFile.write('\n  Generated from {0}\n'.format(scriptFileName))
    outFile.write(' */\n')
    # Add what follows into the groupName group.
    outFile.write('/*! @addtogroup {0}\n'.format(groupName))
    outFile.write('  @{\n')
    outFile.write('*/\n')

    # Parse and add variables
    blocks = parseVariables(content)
    for block in blocks:
        outFile.write(block)
        outFile.write('\n')
    # Parse functions and macros.
    blocks = parseFunctions(content)
    for block in blocks:
        outFile.write(block)
        outFile.write('\n')

    # Close the groupName group
    outFile.write('/*! @} */\n')
    if isFindModule:
        # Close the packages group.
        outFile.write('/*! @} */\n')


def cmake2c(cmakeFileName, outdir=None):
    """
    Reads a CMake script file and convert it to a Doxygen commented C file either in
     the current directory or in outdir.

    The converted file name is based on the cmakeFile name by changing the extension
    to '.h' rather than '.cmake'. This is not designed for use with 'CMakeLists.txt'
    files.

    See convertCMakeFile() for details on how the script is coverted.
    :param cmakeFileName: The name of the CMake script file to convert.
    :param outdir: Optionally, specifies the output directory for the file. Otherwise
    outputs alongside the cmakeFileName.
    """
    # Read the input script file.
    content = ''
    with open(cmakeFileName, 'r') as infile:
        content = infile.read()
    baseFileName = os.path.basename(cmakeFileName)
    if not outdir:
        outdir = os.path.dirname(cmakeFileName)
    outFileName = os.path.join(outdir, baseFileName) + '.h'
    with open(outFileName, 'w') as outfile:
        convertCMakeFile(content, baseFileName, outfile)


# -----------------------------------------------------------------------------
# Main script
# -----------------------------------------------------------------------------
parser = argparse.ArgumentParser(description=
"""Parses CMake scripts looking for CMake variables, macros and functions and
convert them for Doxygen parsing. The leading comments of the macro or function
are used as the Doxygen comments for that macro or function. The comment is
prefixed with either "macro" or "function" for identification.

The first comment block in the file is also extracted as provided as a module
style comment.

The extracted documentation may then be parsed by Doxygen to generate HTML
documentation.""")
parser.add_argument('-g', help="Generate Doxygen documentation after parsing. Requires Doxygen excutable.", nargs=1)
parser.add_argument('-o', help="Specify the output directory. Outputs alongside each input file if not specified.", nargs=1)
parser.add_argument('input_args', nargs='*')
args = parser.parse_args()

# Check for Doxygen.
doxyexe = 'doxygen'
doxyexe = r'C:\Program Files\doxygen\bin\doxygen.exe'
if args.g:
    ecode = 0;
    try:
        ecode = subprocess.call([doxyexe, '--version'])
    except FileNotFoundError:
        ecode = 1
    if ecode != 0:
        sys.stderr.write('Doxygen excutable not found. Please ensure Doxygen is installed and available on the path.')
        sys.exit(1)

outdir = None
if args.o and len(args.o):
    outdir = args.o[0]
    os.makedirs(outdir, exist_ok=True)

# Parsing.
for inspec in args.input_args:
    # Handle files, directories and globs.
    infiles = None
    if os.path.isfile(inspec):
        infiles = [inspec]
    elif os.path.isdir(inspec):
        infiles = glob.glob(os.path.join(inspec, '*'))
    else:
        infiles = glob.glob(inspec)
    if infiles:
        for infile in infiles:
            if os.path.isfile(infile):
                cmake2c(infile, outdir)

if args.g:
    # Generate Doxygen documentation.
    # Convert Doxygen spec file into an absolute path in case the working directory is changed.
    doxyfile = os.path.abspath(args.g[0])
    subprocess.call([doxyexe, doxyfile], cwd=outdir)
