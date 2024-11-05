#ifndef BASE_SHAPE_H
#define BASE_SHAPE_H

#include "utils.h"
#include "XMLNode.h"
#include "InverseIndex.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>

class IPaint {
public:
  virtual std::unique_ptr<IPaint> clone() const = 0; 
};

class RGB : public IPaint {
public: 
  double r, g, b;
  RGB(double r, double g, double b) : r(r), g(g), b(b) {}

  std::unique_ptr<IPaint> clone() const override {
    
    return std::make_unique<RGB>(*this);
  }
};

enum StrokeLineJoin {
  LINE_JOIN_ARCS = 0,
  LINE_JOIN_BEVEL,
  LINE_JOIN_MITER,
  LINE_JOIN_MITER_CLIP,
  LINE_JOIN_ROUND,
  LINE_JOIN_COUNT,
};

constexpr std::string_view linejoin_name[LINE_JOIN_COUNT] {
  "arcs",
  "bevel",
  "miter",
  "miter-clip",
  "round",
};

constexpr InverseIndex<LINE_JOIN_COUNT> inv_linejoin = {&linejoin_name};

enum StrokeLineCap {
  LINE_CAP_BUTT = 0,
  LINE_CAP_ROUND,
  LINE_CAP_SQUARE,
  LINE_CAP_COUNT,
};

constexpr std::string_view linecap_name[LINE_CAP_COUNT] {
  "butt",
  "round",
  "square",
};

constexpr InverseIndex<LINE_CAP_COUNT> inv_linecap = {&linecap_name};

enum FillRule {
  FILL_RULE_NONZERO = 0,
  FILL_RULE_EVENODD,
  FILL_RULE_COUNT
};

constexpr std::string_view fillrule_name[FILL_RULE_COUNT] {
  "nonzero",
  "evenodd",
};

constexpr InverseIndex<FILL_RULE_COUNT> inv_fillrule = {&fillrule_name};

class BaseShape {
public:
  bool visible;

  std::unique_ptr<IPaint> fill;
  std::unique_ptr<IPaint> stroke;

  double opacity;
  double fill_opacity;
  double stroke_opacity;

  double stroke_width;
  float stroke_dash_offset;
  float stroke_dash_array[8];
  int stroke_dash_count;

  StrokeLineJoin stroke_line_join;
  StrokeLineCap stroke_line_cap;

  double miter_limit;
  FillRule fill_rule;

  double transform[2][3];

  std::unique_ptr<BaseShape> next;

  BaseShape(Attribute *attrs, int attrs_count, BaseShape *parent);

  virtual void draw() {
  }

private: 
  void solve_style(std::string_view value);
};


#endif
