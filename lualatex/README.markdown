Title:                  LaTeX input processor for MultiMarkdown  
Author:                 Marko Mahnič  
Base Header Level:      2  
Status:                 Proof of concept

# Description #

The LateX module multimarkdown provides a TeX input buffer processor that
transforms a region of MarkDown text into LaTeX. It defines a new command
\startmarkdown that redirects the text from TeX into a Lua table. The
redirection ends at the command \stopmarkdown at which point the collected text
is transformed into LaTeX using a library based on peg-multimarkdown and
inserted into the buffer.

The generated LaTeX code is inserted into the TeX buffer with the \input
command using a tex file reader callback that reads lines from a Lua table
instead of a file. Reading from the table is activated when the filename passed
to \input starts with 'multimarkdowninput:'.

Note: This package is currently at a level of a proof-of-concept. It was
developed and tested on Ubuntu 11.10 with TeXLive-20111220.


# Installation and Test #

    cd lualatex
    make
    cd test
    bash doit.sh


# The \startmarkdown Command #

The \startmarkdown uses the successive group as comma deilimited list of parameters:

    \startmarkdown{parameters}

It recognizes the following parameters:

* *baseheaderlevel=value* where value is a number between 1 and 8 or one of the
  following: part, chapter, section, subsection, subsubsection, paragraph,
  subparagraph, subsubparagraph.


# The \stopmarkdown Command #

The \stopmarkdown command ends a region started by \startmarkdown. It is
detected and removed by the input buffer processor. If the command doesn't have
a matching \startmarkdown it will be picked up by TeX and will throw an error.

Note: Currently the command should be placed on its own line, oterwise it won't
be detected by ite input buffer processor.


# The \includemarkdown Command #

The \includemarkdown uses two groups. The first group names the file to be
included, the second lists the parameters as in \startmarkdown:

    \includemarkdown{filename}{parameters}

