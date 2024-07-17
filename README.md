
[![GitHub license](https://img.shields.io/github/license/tudelft3d/val3dity?style=flat-square)](https://github.com/tudelft3d/val3dity/blob/master/LICENSE) [![docs](https://img.shields.io/badge/docs-val3dity.rtfd.io-brightgreen?style=flat-square)](https://val3dity.rtfd.io/) [![webapp](https://img.shields.io/badge/webapp-geovalidation.bk.tudelft.nl%2Fval3dity%2F-fea93b?style=flat-square)](http://geovalidation.bk.tudelft.nl/val3dity/) [![DOI](https://img.shields.io/badge/DOI-10.1186%2Fs40965--018--0043--x-blueviolet?style=flat-square)](http://dx.doi.org/10.1186/s40965-018-0043-x)

# val3dity

val3dity---pronounced 'val-three-dity'---allows us to validate 3D primitives according to the international standard ISO19107.
Think of it as [PostGIS ST_IsValid](http://postgis.net/docs/ST_IsValid.html), but for 3D primitives.

In short, it verifies whether a 3D primitive respects the definition as given in [ISO19107](https://www.iso.org/standard/66175.html).

The validation of the following 3D primitives is fully supported:

  - ``MultiSurface``
  - ``CompositeSurface`` 
  - ``Solid``
  - ``MultiSolid``
  - ``CompositeSolid``

Unlike many other validation tools in 3D GIS, inner rings in polygons/surfaces are supported and so are cavities in solids (also called voids or inner shells).

However, as is the case for many formats used in practice, only planar and linear primitives are allowed: no curves or spheres or other parametrically-modelled primitives are supported. 
There is no plan to support these geometries.

val3dity accepts as input:

  - [CityJSON](http://www.cityjson.org)
  - [CityJSON Sequences (CityJSONSeq)](https://www.cityjson.org/cityjsonseq/)
  - [tu3djson](https://github.com/tudelft3d/tu3djson)
  - [JSON-FG (OGC Features and Geometries JSON)](https://github.com/opengeospatial/ogc-feat-geo-json)
  - [OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
  - [OFF](https://en.wikipedia.org/wiki/OFF_(file_format))
  - [IndoorGML](http://indoorgml.net/)

For the CityJSON and IndoorGML formats, extra validations are performed (specific to the format), eg the overlap between different parts of a building, or the validation of the navigation graph in IndoorGML.


## Installation of the command-line tool

### macOS

The simplest, if you have [Homebrew](http://brew.sh/), is:

    $ brew tap tudelft3d/software
    $ brew install val3dity

You can also get the code, and compile it yourself.
You first need to install the following free libraries:

  1. [CGAL v5.4+](http://www.cgal.org) 
  1. [Eigen library](http://eigen.tuxfamily.org)
  1. [GEOS](http://trac.osgeo.org/geos/)
  1. [CMake](http://www.cmake.org)

Under macOS, we suggest using [Homebrew](http://brew.sh/):

    $ brew install cgal
    $ brew install eigen
    $ brew install geos
    $ brew install cmake

and then

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make


### Linux

Under Linux (at least Ubuntu), CGAL has to be compiled because apt-get doesn't give you a version with Eigen.
Thus, in a nutshell,

  1. install Eigen library: `$ sudo apt install libeigen3-dev`
  1. install GEOS library: `$ sudo apt install libgeos++-dev`
  1. install CGAL: `$ sudo apt-get install libcgal-dev`
    - if it's not CGAL v5.4+ that is installed, download [latest CGAL code](https://github.com/CGAL/cgal/releases) and unzip somewhere
    - set `CGAL_DIR` to that folder, eg `export CGAL_DIR=/home/hledoux/software/CGAL-5.4.2`, this will tell your shell to use that version of CGAL (thus more version of CGAL can be installed on the same computer; see that [handy manual](https://github.com/CGAL/cgal/wiki/Branch-Build))


### Windows

For Windows, we offer [an executable](https://github.com/tudelft3d/val3dity/releases).

It's also possible to compile val3dity yourself with the CMake file we offer, but it's slightly more complex with the dependencies.
We suggest you look at [how we build it on GitHub Actions](https://github.com/tudelft3d/val3dity/blob/main/.github/workflows/build_exe.yml) to get some inspiration (and help).


## Usage

To run val3dity:

    val3dity ./data/cityjson/cube.json

The summary of the validation is reported, and you should see that `cube.json` contains one valid primitive.

Finally, to see all the options possible:

    val3dity --help

To validate a [CityJSONSeq stream](https://www.cityjson.org/cityjsonseq/), you need to pipe the stream into val3dity and use ``stdin`` for the input. 
If you have a CityJSONSeq serialised in a file, then you can cat it:

    cat myfile.city.jsonl | val3dity stdin

The output shows, line by line, what the errors are. 
If the list of error is empty (``[]``) this means the feature is geometrically valid 🎉


## Using val3dity as a library

val3dity can be compiled and used as a library:

    cmake .. -DVAL3DITY_LIBRARY=true

There is a simple example in `./demo_lib` with instructions to compile it.


## Web application

If you don't want to go through the troubles of compiling and/or installing val3dity, we suggest you use the [web application](http://geovalidation.bk.tudelft.nl/val3dity).
Simply upload your file to our server and get a validation report back.
We delete the file as soon as it has been validated (promised!).
However, a file is limited to 50MB.


## Simple visualiser for the errors: viz3dity

![](./tools/viz3dity/screenshot.png)

In the folder `tools/viz3dity/`, there is a simple Python script where you can load a CityJSON file with geometries and one with the val3dity report, and you can see which objects have specific errors (each error gets a colour).

However, it's not possible to see where in an object the error is (eg which surface is not planar).
This tool helps to quickly visualise where the errors are in a large dataset, for instance a city.

## Documentation and help

Read the full documentation at [https://val3dity.rtfd.io](https://val3dity.rtfd.io).

The primary channel to communicate with the developers is the Issues section.

If you have a question or came across a bug, please submit an issue there.
However we ask you check first whether your problem has already been solved by someone else.


## Unit tests

To verify that everything went fine during the compilation, run the unit tests (from the root folder of val3dity) (this requires `pip install pytest pyyaml`):

    python -m pytest --runfull

You shouldn't get any errors, if you do then let us know by opening an issue.


## If you use val3dity in a scientific context, please cite these two articles:

```bibtex
@article{Ledoux18,
    title = {{val3dity}: validation of {3D GIS} primitives according to the international standards},
    author = {Ledoux, Hugo},
    journal = {Open Geospatial Data, Software and Standards},
    year = {2018},
    volume = {3},
    number = {1},
    pages = {1},
    doi = {http://dx.doi.org/10.1186/s40965-018-0043-x}
}

@article{Ledoux13,
  title = {On the validation of solids represented with the international standards for geographic information},
  author = {Ledoux, Hugo},
  journal = {Computer-Aided Civil and Infrastructure Engineering},
  year = {2013},
  volume = {28},
  number = {9},
  pages = {693--706},
  doi = {http://dx.doi.org/10.1111/mice.12043}
}
```


## Error codes

![](./docs/_static/errorcodes.png)
