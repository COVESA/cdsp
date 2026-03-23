This directory contains the project documentation including the sources for the online documentation website.

# Central Data Service Playground (CDSP) documentation site

The [CDSP documentation site](https://covesa.github.io/cdsp/) is generated using the Hugo static website generator and is hosted with GitHub Pages.

It is generated from
the files in the ```/doc-site``` directory tree of this repository.
The `static/images` folder within that tree contains assets such as architectural diagrams that will be referenced from multiple pages. The `content` folder contains the files such as the markdown files that provide the rest of the content.

The static web pages are generated automatically after every PR merged to main and deployed using a github workflow.

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

The static pages are generated with:

- [Hugo](https://gohugo.io/)
- [Hextra Hugo Theme](https://github.com/imfing/hextra)

Please follow the [Hugo](https://gohugo.io/documentation/) and [Hextra](https://imfing.github.io/hextra/) documentation for installation and further questions around the framework.
Currently, the Hugo version used for generating VSS documentation is `0.156.0`,
as controlled by the Github Workflow [pages.yaml](https://github.com/COVESA/cdsp/blob/main/.github/workflows/pages.yaml)


## Run the documentation server locally

Once hugo is installed please follow the following steps:

### Check that Hugo is working:
```
hugo version
```
The following outcome is expected:

```
hugo v0.xx.xx ...
```

### Test locally with your Hugo server:

Within the repository

```
hugo server --disableFastRender -D -s ./docs/doc-site/
```

Optional ```-D``` include draft pages as well. Afterwards, you can access the page under http://localhost:1313/cdsp/.

## Contribute

If you want to contribute, do the following:

1. Change documentation in ```/doc-site```

1. Test your changes locally, as described above

1. Create Github Pull Request for review
