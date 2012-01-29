Title:	peg-multimarkdown User's Guide  
Author:	Fletcher T. Penney  
Base Header Level:	2  

# Introduction #

[Markdown] is a simple markup language used to convert plain text into HTML. 

[MultiMarkdown] is a derivative of Markdown that adds new syntax features, such as footnotes, tables, and metadata. Additionally, it offers mechanisms to convert plain text into LaTeX in addition to HTML. 

[peg-multimarkdown] is an implementation of MultiMarkdown derived from John MacFarlane's [peg-markdown]. It makes use of a parsing expression grammar (PEG), and is written in C. It should compile for most any (major) operating system. 

Thanks to work by Daniel Jalkut, MMD no longer requires GLib2 as a dependency. This should make it easier to compile on various operating systems. 


# Installation #


## Mac OS X ##

On the Mac, you can choose from using an installer to install the program for you, or you can compile it yourself from scratch. If you know what that means, follow the instructions below in the Linux section. Otherwise, definitely go for the installer! 

You can also install MultiMarkdown with the package manager [MacPorts] with the following command: 

	sudo port install multimarkdown

Or using [homebrew]: 

	brew install multimarkdown

**NOTE**: I don't maintain either of these ports/packages and can't vouch that they are up to date or working properly.   That said, I have started using [homebrew] to install the latest development build on my machine, while using `make` in my working directory while editing:

	brew install multimarkdown --HEAD

If you don't know what any of that means, just [grab the installer][downloads]. 

If you want to compile for yourself, be sure you have the Developer Tools installed, and then follow the directions for [Linux]. 

If you want to make your own installer, you can use the `make mac-installer` command after compiling the `multimarkdown` binary itself. 


## Windows ##

The easiest way to get peg-multimarkdown running on Windows is to download the installer from the [downloads page][downloads]. It is created with the help of BitRock's software. 

If you want to compile this yourself, you do it in the same way that you would install peg-markdown for Windows. The instructions are on the peg-multimarkdown [wiki] (https://github.com/fletcher/peg-multimarkdown/wiki/Building-for-Windows). I was able to compile for Windows fairly easily using Ubuntu linux following those instructions. I have not tried to actually compile on a Windows machine. 

As a shortcut, if running on a linux machine you can use: 

	make windows

This creates the `multimarkdown.exe` binary. You can then install this manually. 

The `make win-installer` command is what I use to package up the BitRock installer into a zipfile. You probably won't need it. 


## Linux ##

You can either download the source from [peg-multimarkdown], or (preferentially) you can use git: 

	git clone git://github.com/fletcher/peg-multimarkdown.git

You can run the `update_submodules.sh` script to update the submodules if you want to run the test commands, download the sample files and the Support directory, or compile the documentation. 

Then, simply run `make` to compile the source. 

You can also run some test commands to verify that everything is working properly. Of note, it is normal to fail one test in the Markdown tests, but the others should pass. You can then install the binary wherever you like. 

	make
	make test
	make mmd-test
	make latex-test
	make compat-test

**NOTE** As of version 3.2, the tests including obfuscated email addresses will also fail due to a change in how random numbers are generated. 

## FreeBSD ##

If you want to compile manually, you should be able to follow the directions for Linux above.  However, apparently MultiMarkdown has been put in the ports tree, so you can also use: 

	cd /usr/ports/textproc/multimarkdown
	make install

(I have not tested this myself, and cannot guarantee that it works properly.  Come to think of it, I don't even know which version of MMD they use.) 


# Usage #

Once installed, you simply do something like the following: 

* `multimarkdown file.txt` --- process text into HTML. 

* `multimarkdown -c file.txt` --- use a compatibility mode that emulates the original Markdown. 

* `multimarkdown -t latex file.txt` --- output the results as LaTeX instead of HTML. This can then be processed into a PDF if you have LaTeX installed. You can further specify the `LaTeX Mode` metadata to customize output for compatibility with `memoir` or `beamer` classes. 

* `multimarkdown -t odf file.txt` --- output the results as an OpenDocument Text Flat XML file. Does require the plugin be installed in your copy of OpenOffice, which is available at the [downloads] page. LibreOffice includes this plugin by default. 

* `multimarkdown -t opml file.txt` --- convert the MMD text file to an MMD OPML file, compatible with OmniOutliner and certain other outlining and mind-mapping programs (including iThoughts and iThoughtsHD). 

* `multimarkdown -h` --- display help and additional options. 

* `multimarkdown -b *.txt` --- `-b` or `--batch` mode can process multiple files at once, converting `file.txt` to `file.html` or `file.tex` as directed. Using this feature, you can convert a directory of MultiMarkdown text files into HTML files, or LaTeX files with a single command without having to specify the output files manually. **CAUTION**: This will overwrite existing files with the `html` or `tex` extension, so use with caution. 

**Note**: Several convenience scripts are available to simplify things: 

	mmd			=> multimarkdown -b
	mmd2tex		=> multimarkdown -b -t latex
	mmd2odf		=> multimarkdown -b -t odf
	mmd2opml	=> multimarkdown -b -t opml
	
	mmd2pdf		=> Unsupported script to try and run latex/xelatex.
				   You can direct questions to the discussion list, but
				   I may or may not respond.  It works for me, so I share
				   it with those who are interested but make no
				   guarantees.


# Why create another version of MultiMarkdown? #

* Maintaining a growing collection of nested regular expressions was going to become increasingly difficult. I don't plan on adding much (if any) in the way of new syntax features, but it was a mess. 

* Performance on longer documents was poor. The nested perl regular expressions was slow, even on a relatively fast computer. Performance on something like an iPhone would probably have been miserable. 

* The reliance on Perl made installation fairly complex on Windows. That didn't bother me too much, but it is a factor. 

* Perl can't be run on an iPhone/iPad, and I would like to be able to have MultiMarkdown on an iOS device, and not just regular Markdown (which exists in C versions). 

* I was interested in learning about PEG's and revisiting C programming. 

* The syntax has been fairly stable, and it would be nice to be able to formalize it a bit --- which happens by definition when using a PEG. 

* I wanted to revisit the syntax and features and clean things up a bit. 

* Did I mention how much faster this is? And that it could (eventually) run on an iPhone? 


# What's different? #


## "Complete" documents vs. "snippets" ##

A "snippet" is a section of HTML (or LaTeX) that is not a complete, fully-formed document. It doesn't contain the header information to make it a valid XML document. It can't be compiled with LaTeX into a PDF without further commands. 

For example: 

	# This is a header #
	
	And a paragraph.

becomes the following HTML snippet: 

	<h1 id="thisisaheader">This is a header</h1>
	
	<p>And a paragraph.</p>

and the following LaTeX snippet: 

	\part{This is a header}
	\label{thisisaheader}
	
	
	And a paragraph.

It was not possible to create a LaTeX snippet with the original MultiMarkdown, because it relied on having a complete XHTML document that was then converted to LaTeX via an XSLT document (requiring a whole separate program). This was powerful, but complicated. 

Now, I have come full-circle. peg-multimarkdown will now output LaTeX directly, without requiring XSLT. This allows the creation of LaTeX snippets, or complete documents, as necessary. 

To create a complete document, simply include metadata. You can include a title, author, date, or whatever you like. If you don't want to include any real metadata, including "format: complete" will still trigger a complete document, just like it used to. 

**NOTE**: If the *only* metadata present is `Base Header Level` then a complete document will not be triggered. This can be useful when combining various documents together. 

The old approach (even though it was hidden from most users) was a bit of a kludge, and this should be more elegant, and more flexible. 


## Creating LaTeX Documents ##

LaTeX documents are created a bit differently than under the old system. You no longer have to use an XSLT file to convert from XHTML to LaTeX. You can go straight from MultiMarkdown to LaTeX, which is faster and more flexible. 

To create a complete LaTeX document, you can process your file as a snippet, and then place it in a LaTeX template that you already have. Alternatively, you can use metadata to trigger the creation of a complete document. You can use the `LaTeX Input` metadata to insert a `\input{file}` command. You can then store various template files in your texmf directory and call them with metadata, or with embedded raw LaTeX commands in your document. For example: 

	LaTeX Input:		mmd-memoir-header  
	Title:				Sample MultiMarkdown Document  
	Author:				Fletcher T. Penney  
	LaTeX Mode:			memoir  
	LaTeX Input:		mmd-memoir-begin-doc  
	LaTeX Footer:		mmd-memoir-footer  

This would include several template files in the order that you see. The `LaTeX Footer` metadata inserts a template at the end of your document. Note that the order and placement of the `LaTeX Include` statements is important. 

The `LaTeX Mode` metadata allows you to specify that MultiMarkdown should use the `memoir` or `beamer` output format. This places subtle differences in the output document for compatibility with those respective classes. 

This system isn't quite as powerful as the XSLT approach, since it doesn't alter the actual MultiMarkdown to LaTeX conversion process. But it is probably much more familiar to LaTeX users who are accustomed to using `\input{}` commands and doesn't require knowledge of XSLT programming. 

I recommend checking out the default [LaTeX Support Files] that are available on github. They are designed to serve as a starting point for your own needs. 

**Note**: You can still use this version of MultiMarkdown to convert text into XHTML, and then process the XHTML using XSLT to create a LaTeX document, just like you used to in MMD 2.0. 

[LaTeX Support Files]: https://github.com/fletcher/peg-multimarkdown-latex-support


## Footnotes ##

Footnotes work slightly differently than before. This is partially on purpose, and partly out of necessity.  Specifically: 

* Footnotes are anchored based on number, rather than the label used in the MMD source. This won't show a visible difference to the reader, but the XHTML source will be different. 

* Footnotes can be used more than once. Each reference will link to the same numbered note, but the "return" link will only link to the first instance. 

* Footnote "return" links are a separate paragraph after the footnote. This is due to the way peg-markdown works, and it's not worth the effort to me to change it. You can always use CSS to change the appearance however you like. 

* Footnote numbers are surrounded by "[]" in the text. 


## Raw HTML ##

Because the original MultiMarkdown processed the text document into XHTML first, and then processed the entire XHTML document into LaTeX, it couldn't tell the difference between raw HTML and HTML that was created from plaintext. This version, however, uses the original plain text to create the LaTeX document. This means that any raw HTML inside your MultiMarkdown document is **not** converted into LaTeX. 

The benefit of this is that you can embed one piece of the document in two formats --- one for XHTML, and one for LaTeX: 

	<blockquote>
	<p>Release early, release often!</p>
	<blockquote><p>Linus Torvalds</p></blockquote>
	</blockquote>
	
	<!-- \epigraph{Release early, release often!}{Linus Torvalds} -->

In this section, when the document is converted into XHTML, the `blockquote` sections will be used as expected, and the `epigraph` will be ignored since it is inside a comment. Conversely, when processed into LaTeX, the raw HTML will be ignored, and the comment will be processed as raw LaTeX. 

You shouldn't need to use this feature, but if you want to specify exactly how a certain part of your document is processed into LaTeX, it's a neat trick. 


## Processing MultiMarkdown inside HTML ##

In the original MultiMarkdown, you could use something like `<div markdown=1>` to tell MultiMarkdown to process the text inside the div. In peg-multimarkdown, you can do this, or you can use the command-line option `--process-html` to process the text inside all raw HTML. 


## Math Support ##

MultiMarkdown 2.0 supported [ASCIIMathML] embedded with MultiMarkdown documents. This syntax was then converted to MathML for XHTML output, and then further processed into LaTeX when creating LaTeX output. The benefit of this was that the ASCIIMathML syntax was pretty straightforward. The downside was that only a handful of browsers actually support MathML, so most of the time it was only useful for LaTeX. Many MMD users who are interested in LaTeX output already knew LaTeX, so they sometimes preferred native math syntax, which led to several hacks. 

MultiMarkdown 3.0 does not have built in support for ASCIIMathML. In fact, I would probably have to write a parser from scratch to do anything useful with it, which I have little desire to do. So I came up with a compromise. 

ASCIIMathML is no longer supported by MultiMarkdown. Instead, you *can* use LaTeX to code for math within your document. When creating a LaTeX document, the source is simply passed through, and LaTeX handles it as usual. *If* you desire, you can add a line to your header when creating XHTML documents that will allow [MathJax] to appropriately display your math. 

Normally, MathJax *and* LaTeX supported using `\[ math \]` or `\( math \)` to indicate that math was included. MMD stumbled on this due to some issues with escaping, so instead we use `\\[ math \\]` and `\\( math \\)`. See an example: 

	latex input:	mmd-article-header  
	Title:			MultiMarkdown Math Example  
	latex input:	mmd-article-begin-doc  
	latex footer:	mmd-memoir-footer  
	xhtml header:	<script type="text/javascript"
		src="http://localhost/~fletcher/math/mathjax/MathJax.js">
		</script>
				
				
	An example of math within a paragraph --- \\({e}^{i\pi }+1=0\\)
	--- easy enough.
	
	And an equation on it's own:
	
	\\[ {x}_{1,2}=\frac{-b\pm \sqrt{{b}^{2}-4ac}}{2a} \\]
	
	That's it.

You would, of course, need to change the `xhtml header` metadata to point to your own installation of MathJax. 

**Note**: MultiMarkdown doesn't actually *do* anything with the code inside the brackets. It simply strips away the extra backslash and passes the LaTeX source unchanged, where it is handled by MathJax *if* it's properly installed, or by LaTeX. If you're having trouble, you can certainly email the [MultiMarkdown Discussion List], but I do not provide support for LaTeX code. 

[ASCIIMathML]:	http://www.chapman.edu/~jipsen/mathml/Asciimath.html
[MathJax]: 		http://www.mathjax.org/
[MultiMarkdown Discussion List]: http://groups.google.com/group/multimarkdown/


# Acknowledgments #

Thanks to John MacFarlane for [peg-markdown]. Obviously, this derivative work would not be possible without his work. Additionally, he was very gracious in giving me some pointers when I was getting started with trying to modify his software, and he continues to update peg-markdown with the various edge cases MultiMarkdown users have found.   Hopefully both programs are better as a result.

Thanks to Daniel Jalkut for his work on enabling MultiMarkdown to run without relying on GLib2.  This makes it much more flexible! 

Thanks to John Gruber for the original [Markdown]. 'Nuff said. 

And thanks to the many contributors and users of the original MultiMarkdown that helped me refine the syntax and search out bugs. 


[peg-markdown]:			https://github.com/jgm/peg-markdown
[Markdown]:				http://daringfireball.net/projects/markdown/
[MultiMarkdown]:		http://fletcherpenney.net/multimarkdown/
[peg-multimarkdown]:	https://github.com/fletcher/peg-multimarkdown
[fink]:					http://www.finkproject.org/
[downloads]:			http://github.com/fletcher/peg-multimarkdown/downloads
[GTK+]:					http://www.gtk.org/
[homebrew]:				https://github.com/mxcl/homebrew
[MacPorts]:             http://www.macports.org/
