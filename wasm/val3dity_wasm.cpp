/*
  val3dity wasm bindings

  Copyright (c) 2011-2026, 3D geoinformation research group, TU Delft
*/

#include "val3dity.h"
#include "Feature.h"
#include "input.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using emscripten::val;
using json = nlohmann::json;

namespace val3dity {
extern std::string VAL3DITY_VERSION;
}

namespace {

std::string lowercase(std::string s)
{
  for (auto& c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

bool is_js_array(const val& v)
{
  return val::global("Array").call<bool>("isArray", v);
}

bool has_length(const val& v)
{
  return !v["length"].isUndefined() && !v["length"].isNull();
}

unsigned length_of(const val& v, const char* name)
{
  if (!has_length(v)) {
    throw std::invalid_argument(std::string(name) + " must be an array or typed array");
  }
  return v["length"].as<unsigned>();
}

bool is_number(const val& v)
{
  return v.typeOf().as<std::string>() == "number";
}

bool is_sequence(const val& v)
{
  return has_length(v);
}

double option_number(const json& options, const char* camel, const char* snake, double fallback)
{
  if (options.contains(camel)) {
    return options.at(camel).get<double>();
  }
  if (options.contains(snake)) {
    return options.at(snake).get<double>();
  }
  return fallback;
}

val3dity::Primitive3D primitive_from_string(const std::string& value)
{
  const std::string p = lowercase(value);
  if (p == "solid") {
    return val3dity::SOLID;
  }
  if (p == "multisurface" || p == "multi_surface") {
    return val3dity::MULTISURFACE;
  }
  if (p == "compositesurface" || p == "composite_surface") {
    return val3dity::COMPOSITESURFACE;
  }
  throw std::invalid_argument("primitive must be Solid, MultiSurface, or CompositeSurface");
}

val3dity::Parameters parse_options(const std::string& options_json)
{
  val3dity::Parameters params;
  if (options_json.empty()) {
    return params;
  }

  json options = json::parse(options_json);
  if (!options.is_object()) {
    throw std::invalid_argument("options must be a JSON object");
  }

  params.tol_snap(option_number(options, "tolSnap", "tol_snap", params._tol_snap));
  params.planarity_d2p_tol(option_number(options,
                                         "planarityD2pTol",
                                         "planarity_d2p_tol",
                                         params._planarity_d2p_tol));
  params.planarity_n_tol(option_number(options,
                                       "planarityNTol",
                                       "planarity_n_tol",
                                       params._planarity_n_tol));
  params.overlap_tol(option_number(options, "overlapTol", "overlap_tol", params._overlap_tol));

  if (options.contains("primitive")) {
    params.primitive(primitive_from_string(options.at("primitive").get<std::string>()));
  }

  return params;
}

json wasm_error(const std::string& message)
{
  return {
    {"type", "val3dity_wasm_error"},
    {"validity", false},
    {"errors", json::array({{{"code", "wasm_exception"}, {"message", message}}})},
  };
}

json validate_cityjsonseq_report(const std::string& input, val3dity::Parameters params)
{
  spdlog::set_level(spdlog::level::off);

  val3dity::IOErrors ioerrs;
  ioerrs.set_input_file_type("CityJSONSeq");

  std::vector<val3dity::Feature*> features;
  std::vector<val3dity::GeometryTemplate*> geometry_templates;
  json transform;
  bool has_transform = false;
  bool has_feature = false;

  std::istringstream stream(input);
  std::string line;
  unsigned line_number = 0;

  while (std::getline(stream, line)) {
    ++line_number;
    if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
      continue;
    }

    json item;
    try {
      item = json::parse(line);
    } catch (const json::parse_error&) {
      ioerrs.add_error(901, "Input has invalid JSON at line #" + std::to_string(line_number));
      break;
    }

    const std::string type = item.value("type", "");
    if (type == "CityJSON") {
      if (item.count("geometry-templates") == 1) {
        val3dity::process_cityjson_geometrytemplates(item["geometry-templates"],
                                                     geometry_templates,
                                                     params._tol_snap);
      }
      if (item.count("transform") == 0) {
        ioerrs.add_error(901, "Input first CityJSON line has no \"transform\" property");
        break;
      }
      transform = item["transform"];
      has_transform = true;
    } else if (type == "CityJSONFeature") {
      has_feature = true;
      if (has_transform && item.count("transform") == 0) {
        item["transform"] = transform;
      }
      val3dity::parse_cjseq(item, features, params._tol_snap, geometry_templates);
    } else {
      ioerrs.add_error(904, "Input has unsupported JSON object at line #" + std::to_string(line_number));
      break;
    }
  }

  if (!has_feature && !ioerrs.has_errors()) {
    ioerrs.add_error(901, "Input has no CityJSONFeature lines");
  }

  if (!ioerrs.has_errors()) {
    for (auto& feature : features) {
      feature->validate(params._planarity_d2p_tol, params._planarity_n_tol, params._overlap_tol);
    }
  }

  if (ioerrs.has_specific_error(901)) {
    features.clear();
  }

  return val3dity::get_report_json("CityJSONSeq object",
                                   features,
                                   val3dity::VAL3DITY_VERSION,
                                   params._tol_snap,
                                   params._overlap_tol,
                                   params._planarity_d2p_tol,
                                   params._planarity_n_tol,
                                   ioerrs);
}

std::vector<std::array<double, 3>> parse_vertices(const val& vertices)
{
  std::vector<std::array<double, 3>> parsed;
  const unsigned n = length_of(vertices, "vertices");
  if (n == 0) {
    return parsed;
  }

  const val first = vertices[0];
  if (is_number(first)) {
    if (n % 3 != 0) {
      throw std::invalid_argument("flat vertices must have a length divisible by 3");
    }
    parsed.reserve(n / 3);
    for (unsigned i = 0; i < n; i += 3) {
      parsed.push_back({vertices[i].as<double>(),
                        vertices[i + 1].as<double>(),
                        vertices[i + 2].as<double>()});
    }
    return parsed;
  }

  parsed.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    const val vertex = vertices[i];
    if (!is_sequence(vertex) || length_of(vertex, "vertex") < 3) {
      throw std::invalid_argument("vertices must be [x, y, z] triples or one flat xyz array");
    }
    parsed.push_back({vertex[0].as<double>(), vertex[1].as<double>(), vertex[2].as<double>()});
  }
  return parsed;
}

std::vector<int> parse_ring(const val& ring)
{
  const unsigned n = length_of(ring, "ring");
  std::vector<int> parsed;
  parsed.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    if (!is_number(ring[i])) {
      throw std::invalid_argument("rings must contain vertex indices");
    }
    parsed.push_back(ring[i].as<int>());
  }
  return parsed;
}

std::vector<std::vector<std::vector<int>>> parse_faces(const val& faces)
{
  std::vector<std::vector<std::vector<int>>> parsed;
  const unsigned n = length_of(faces, "faces");
  parsed.reserve(n);

  for (unsigned i = 0; i < n; ++i) {
    const val face = faces[i];
    const unsigned face_length = length_of(face, "face");
    if (face_length == 0) {
      throw std::invalid_argument("faces cannot be empty");
    }

    const val first = face[0];
    if (is_number(first)) {
      parsed.push_back({parse_ring(face)});
    } else if (is_sequence(first) || is_js_array(first)) {
      std::vector<std::vector<int>> rings;
      rings.reserve(face_length);
      for (unsigned ring_index = 0; ring_index < face_length; ++ring_index) {
        rings.push_back(parse_ring(face[ring_index]));
      }
      parsed.push_back(rings);
    } else {
      throw std::invalid_argument("faces must be arrays of rings");
    }
  }

  return parsed;
}

std::string validate_cityjson_impl(const std::string& input, const std::string& options_json)
{
  try {
    json cityjson = json::parse(input);
    json report = val3dity::validate(cityjson, parse_options(options_json));
    return report.dump();
  } catch (const std::exception& e) {
    return wasm_error(e.what()).dump();
  } catch (...) {
    return wasm_error("Unknown C++ exception").dump();
  }
}

std::string validate_cityjsonseq_impl(const std::string& input, const std::string& options_json)
{
  try {
    json report = validate_cityjsonseq_report(input, parse_options(options_json));
    return report.dump();
  } catch (const std::exception& e) {
    return wasm_error(e.what()).dump();
  } catch (...) {
    return wasm_error("Unknown C++ exception").dump();
  }
}

std::string validate_raw_arrays_impl(const val& vertices, const val& faces, const std::string& options_json)
{
  try {
    json report = val3dity::validate(parse_vertices(vertices), parse_faces(faces), parse_options(options_json));
    return report.dump();
  } catch (const std::exception& e) {
    return wasm_error(e.what()).dump();
  } catch (...) {
    return wasm_error("Unknown C++ exception").dump();
  }
}

std::string validate_cityjson_default(const std::string& input)
{
  return validate_cityjson_impl(input, "{}");
}

std::string validate_cityjsonseq_default(const std::string& input)
{
  return validate_cityjsonseq_impl(input, "{}");
}

std::string validate_raw_arrays_default(const val& vertices, const val& faces)
{
  return validate_raw_arrays_impl(vertices, faces, "{}");
}

} // namespace

EMSCRIPTEN_BINDINGS(val3dity_wasm)
{
  emscripten::function("validateCityJSON", &validate_cityjson_default);
  emscripten::function("validateCityJSONWithOptions", &validate_cityjson_impl);
  emscripten::function("validateCityJSONSeq", &validate_cityjsonseq_default);
  emscripten::function("validateCityJSONSeqWithOptions", &validate_cityjsonseq_impl);
  emscripten::function("validateRawArrays", &validate_raw_arrays_default);
  emscripten::function("validateRawArraysWithOptions", &validate_raw_arrays_impl);
}
