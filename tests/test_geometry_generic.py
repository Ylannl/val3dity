"""Testing generic error cases on geometry primitives.

The intention is to isolate the geometry from the file format.
The POLY format is simple, supports inner rings and the parser is reliable.
Therefore each geometry error case (http://val3dity.readthedocs.io/en/v2/errors/#)
is tested with the POLY format. If possilbe don't replicate these geometry error cases
in different file formats. For some cases POLY is not suitable though, eg 402,
thus a different format is used, and the test is still here.
"""

import pytest
import os.path


#------------------------------------------------------------------------ Data
@pytest.fixture(scope="module",
                params=["101.poly"])
def data_101(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)
# TODO: add GML ring where the first and the last points are the same


@pytest.fixture(scope="module",
                params=["102.poly"])
def data_102(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)
# TODO: add GML ring where consecutive points are the same


@pytest.fixture(scope="module",
                params=["104.poly",
                        "104_1.poly",
                        "104_2.poly",
                        "104_3.poly",
                        "104_4.poly"])
def data_104(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["201.poly",
                        "201_1.poly"])
def data_201(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["202.poly"])
def data_202(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["203.poly",
                        "203_1.poly",
                        "203_2.poly"])
def data_203(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["204_1.poly"])
def data_204(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["204_valid_1.poly",
                        "204_valid_2.poly"])
def data_204_valid(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["205.poly"])
def data_205(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["206.poly",
                        "206_1.poly"])
def data_206(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["207.poly",
                        "207_1.poly"])
def data_207(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)
# TODO: add 207_2.poly once val3dity can parse the holes in a poly file


@pytest.fixture(scope="module",
                params=["208.poly"])
def data_208(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["301.poly",
                        "301_1.poly"])
def data_301(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["302.poly",
                        "302_1.poly",
                        "303_cs.poly"])
def data_302(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)



@pytest.fixture(scope="module",
                params=["303.poly",
                        "303_1.poly",
                        "303_2.poly",
                        "304_1.poly",
                        "304_2.obj"])
def data_303(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)

@pytest.fixture(scope="module",
                params=["303_cs.poly"])
def data_303_cs(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["305.poly",
                        "305_1.poly"])
def data_305(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["306.poly",
                        "306_1.poly"])
def data_306(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)


@pytest.fixture(scope="module",
                params=["307.poly"])
def data_307(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)

@pytest.fixture(scope="module",
                params=["307_1.poly"])
def data_307_1(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)

@pytest.fixture(scope="module",
                params=["401.poly",
                        "401_1.poly",
                        ["401_2.poly", "inner_shell.poly"],
                        ["401_3.poly", "inner_shell.poly"],
                        "401_4.poly",
                        "401_5.poly",
                        "401_6.poly"])
def data_401(request, dir_geometry_generic, data_basecube):
    ishell = request.param
    ishell_path = []
    if isinstance(ishell, list):
        for i in ishell:
            i_path = os.path.abspath(
                        os.path.join(dir_geometry_generic, i)
                        )
            ishell_path = ishell_path + ["--ishell", i_path]
    else:
        ishell_path = ["--ishell",
                       os.path.abspath(
                           os.path.join(dir_geometry_generic, ishell)
                           )
                       ]
    inner_outer_path = ishell_path + data_basecube
    return(inner_outer_path)


@pytest.fixture(scope="module",
                params=[["402_in_1.poly", "inner_shell.poly"]])
def data_402_1(request, dir_geometry_generic, data_basecube):
    ishell = request.param
    ishell_path = []
    if isinstance(ishell, list):
        for i in ishell:
            i_path = os.path.abspath(
                        os.path.join(dir_geometry_generic, i)
                        )
            ishell_path = ishell_path + ["--ishell", i_path]
    else:
        ishell_path = ["--ishell",
                       os.path.abspath(
                           os.path.join(dir_geometry_generic, ishell)
                           )
                       ]
    inner_outer_path = ishell_path + data_basecube
    return(inner_outer_path)


@pytest.fixture(scope="module",
                params=["403.poly"])
def data_403(request, dir_geometry_generic, data_basecube):
    ishell_path = ["--ishell",
                   os.path.abspath(
                       os.path.join(dir_geometry_generic, request.param)
                       )
                   ]
    inner_outer_path = ishell_path + data_basecube
    return(inner_outer_path)


@pytest.fixture(scope="module",
                params=["404.poly"])
def data_404(request, dir_geometry_generic, data_basecube):
    ishell_path = ["--ishell",
                   os.path.abspath(
                       os.path.join(dir_geometry_generic, request.param)
                       )
                   ]
    inner_outer_path = ishell_path + data_basecube
    return(inner_outer_path)


@pytest.fixture(scope="module",
                params=["405.poly"])
def data_405(request, dir_geometry_generic):
    file_path = os.path.abspath(
        os.path.join(
            dir_geometry_generic,
            request.param))
    return(file_path)

#----------------------------------------------------------------------- Tests
def test_101(validate, data_101, solid):
    error = validate(data_101, options=solid)
    assert(error == [101])

def test_102(validate, data_102, solid):
    error = validate(data_102, options=solid)
    assert(error == [102])

def test_104(validate, data_104, solid):
    error = validate(data_104, options=solid)
    assert(error == [104])

def test_201(validate, data_201, solid):
    error = validate(data_201, options=solid)
    assert(error == [201])

def test_202(validate, data_202, solid):
    error = validate(data_202, options=solid)
    assert(error == [202] or error ==[201])

def test_203(validate, data_203, solid):
    error = validate(data_203, options=solid)
    assert(error == [203])

def test_204(validate, data_204, solid):
    error = validate(data_204, options=solid)
    assert(error == [204])

def test_204_valid(validate, data_204_valid, solid):
    error = validate(data_204_valid, options=solid)
    assert(error == [])

def test_205(validate, data_205, solid):
    error = validate(data_205, options=solid)
    assert(error == [205])

def test_206(validate, data_206, solid):
    error = validate(data_206, options=solid)
    assert(error == [206] or error == [201])

def test_207(validate, data_207, solid):
    error = validate(data_207, options=solid)
    assert(error == [207])

def test_208(validate, data_208, solid):
    error = validate(data_208, options=solid)
    assert(error == [208])

def test_301(validate, data_301, solid):
    error = validate(data_301, options=solid)
    assert(error == [301])

def test_302(validate, data_302, solid):
    error = validate(data_302, options=solid)
    assert(error == [302])

def test_303(validate, data_303, solid):
    error = validate(data_303, options=solid)
    assert(error == [303])

def test_303_cs(validate, data_303_cs, compositesurface):
    error = validate(data_303_cs, options=compositesurface)
    assert(error == [303])

def test_305(validate, data_305, solid):
    error = validate(data_305, options=solid)
    assert(error == [305])

def test_306(validate, data_306, solid):
    error = validate(data_306, options=solid)
    assert(error == [306])

def test_307(validate, data_307, solid):
    """See #62"""
    error = validate(data_307, options=solid)
    assert(error == [303, 307])

def test_307_1(validate, data_307_1, solid):
    error = validate(data_307_1, options=solid)
    assert(error == [307])

def test_401(validate, data_401, solid):
    error = validate(data_401, options=solid)
    assert(error == [401])

def test_402_inner(validate, data_402_1, solid):
    error = validate(data_402_1, options=solid)
    assert(error == [401] or error == [402])

def test_403(validate, data_403, solid):
    error = validate(data_403, options=solid)
    assert(error == [403])

def test_404(validate, data_404, solid):
    error = validate(data_404, options=solid)
    assert(error == [404])

def test_405(validate, data_405, solid):
    error = validate(data_405, options=solid)
    assert(error == [405])
