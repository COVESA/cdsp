This directory contains the project documentation including the sources for the online documentation website.

# Central Data Service Playground (CDSP) documentation site

The [CDSP documentation site](https://covesa.github.io/cdsp/) is realized with GitHub Pages. It is generated from
the files in the ```/docs-gen``` directory tree of this repository.
The `assets` folder within that tree contains assets such as architectural diagrams that will be referenced from multiple pages. The `content` folder contains the files such as the markdown files that provide the rest of the content.
The static webpage is generated automatically after every PR merged to master
and deployed into a branch called `gh-pages`.

## Known limitations

Links in this folder are typically not possible to follow in Github as they refer to locations in the generated documentation
rather than the source documentation. That could possibly be solved by [Hugo Render Hooks](https://gohugo.io/templates/render-hooks/)
but it is not trivial to get a solution that works for all types of internal links.

## Guidelines

Never link directly to a specific page at `github.io` - in the future we might want to keep multiple versions of
documentation and then links must go to the correct version.

## Verifying links

We can use the OSS package `markdown-link-check` to check markdown files. It can be installed like this

```
sudo apt install nodejs npm
sudo npm install -g markdown-link-check
```

What to run:
```
cd cdsp
markdown-link-check *.md
```

For generated files (Hugo) links can be checked by running the cloud service [W3C Link Checker](https://validator.w3.org/checklink), against the location of the online documentation at `https://covesa.github.io/cdsp/`. It checks against latest master.
Run the check with the option `Check linked documents recursively` enabled.


## Dependencies

The static page is generated with:

- [HUGO](https://gohugo.io/)
- [Learn Theme](https://github.com/matcornic/hugo-theme-learn)

Please follow the [documentation](https://gohugo.io/documentation/) for installation and further questions around the framework.
Currently, the HUGO version used for generating VSS documentation is `0.124.0`,
as controlled by the [buildcheck.yml](https://github.com/COVESA/cdsp/blob/main/.github/workflows/buildcheck.yaml)


## Run the documentation server locally

Once hugo is installed please follow the following steps:

### Check that HUGO is working:
```
hugo version
```
The following outcome is expected:

```
hugo v0.xx.xx ...
```

### Clone the submodule containing the theme

Run the following git commands to init and fetch the submodules:

```
git submodule init
git submodule update
```

Reference: [Git Documentation](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

### Test locally with your Hugo server:

Within the repository

```
hugo server --disableFastRender -D -s ./docs-gen/
```

Optional ```-D``` include draft pages as well. Afterwards, you can access the
page under http://localhost:1313/cdsp/.

## Contribute

If you want to contribute, do the following:

1. Change documentation in ```/docs-gen```

1. Test your changes locally, as described above

1. Create Github Pull Request for review
