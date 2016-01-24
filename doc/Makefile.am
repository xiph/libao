docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)
src_files=\
	build/api.rst \
	build/plugins.rst \
	build/config.rst \
	build/drivers.rst \
	build/index.rst
if BUILD_HTML
html_files= \
	search.html \
	config.html \
	index.html \
	plugins.html \
	genindex.html \
	api.html \
	drivers.html
else
html_files=
endif

$(html_files) .sphinx-cache: $(src_files)
	sphinx-build -b html -d .sphinx-cache $(top_srcdir)/doc/build .

doc_DATA=$(html_files) $(wildcard _static/*) $(src_files)
# distribute pre-built docs
EXTRA_DIST=$(doc_DATA)

