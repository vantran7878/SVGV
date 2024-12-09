#include "BaseShape.h"
#include "InverseIndex.h"
#include <cmath>
#include <cctype>
#include "common.h"


enum FontWeight {
  FONTWEIGHT_NORMAL = 0,
  FONTWEIGHT_BOLD,
  FONTWEIGHT_BOLDER,
  FONTWEIGHT_LIGHTER,
  FONTWEIGHT_COUNT,
};

constexpr std::string_view fontweight_name[FONTWEIGHT_COUNT] = {
  "normal",
  "bold",
  "bolder",
  "lighter",
};

constexpr InverseIndex<FONTWEIGHT_COUNT> inv_fontweight{&fontweight_name};

constexpr std::string_view fillrule_name[FILL_RULE_COUNT] = {
  "nonzero",
  "evenodd",
};

constexpr InverseIndex<FILL_RULE_COUNT> inv_fillrule{&fillrule_name};

constexpr std::string_view linecap_name[LINE_CAP_COUNT] = {
  "butt",
  "round",
  "square",
};

constexpr InverseIndex<LINE_CAP_COUNT> inv_linecap{&linecap_name};

constexpr std::string_view linejoin_name[LINE_JOIN_COUNT] = {
  "arcs",
  "bevel",
  "miter",
  "miter-clip",
  "round",
};

constexpr InverseIndex<LINE_JOIN_COUNT> inv_linejoin{&linejoin_name};

constexpr std::string_view fontstyle_name[FONTSTYLE_COUNT] = {
  "normal",
  "italic",
  "oblique",
};

constexpr InverseIndex<FONTSTYLE_COUNT> inv_fontstyle{&fontstyle_name};


enum StyleType {
  STYLE_VISIBLE = 0,
  STYLE_FILL,
  STYLE_STROKE,
  STYLE_FONT_SIZE,
  STYLE_OPACITY,
  STYLE_FILL_OPACITY,
  STYLE_STROKE_OPACITY,
  STYLE_STROKE_WIDTH,
  STYLE_STROKE_DASH_OFFSET,
  STYLE_STROKE_DASH_ARRAY,
  STYLE_STROKE_LINE_JOIN,
  STYLE_STROKE_LINE_CAP,
  STYLE_MITER_LIMIT,
  STYLE_FILL_RULE,
  STYLE_FONT_STYLE,
  STYLE_FONT_WEIGHT,
  STYLE_FONT_FAMILY,
  STYLE_COUNT,
};

constexpr std::string_view style_name[STYLE_COUNT] = {
  "visibility",
  "fill",
  "stroke",
  "font-size",
  "opacity",
  "fill-opacity",
  "stroke-opacity",
  "stroke-width",
  "stroke-dashoffset",
  "stroke-dasharray",
  "stroke-linejoin",
  "stroke-linecap",
  "stroke-miterlimit",
  "fill-rule",
  "font-style",
  "font-weight",
  "font-family",
};

constexpr InverseIndex<STYLE_COUNT> inv_style = {&style_name};


enum AttributeType {
  ATTRIBUTE_TRANSFORM = 0,
  ATTRIBUTE_STYLE, 
  ATTRIBUTE_COUNT,
};

constexpr std::string_view attribute_name[ATTRIBUTE_COUNT] = {
  "transform",
  "style",
};

constexpr InverseIndex<ATTRIBUTE_COUNT> inv_attribute{&attribute_name};


enum TransformType {
  TRANSFORM_MATRIX = 0,
  TRANSFORM_TRANSLATE,
  TRANSFORM_SCALE,
  TRANSFORM_ROTATE,
  TRANSFORM_SKEWX,
  TRANSFORM_SKEWY,
  TRANSFORM_COUNT,
};

constexpr std::string_view transform_name[TRANSFORM_COUNT] = {
  "matrix",
  "translate",
  "scale",
  "rotate",
  "skewX",
  "skewY",
};

constexpr InverseIndex<TRANSFORM_COUNT> inv_transform = {&transform_name};

struct Array {
  double a[6];
  int n;
};

static Array split_number(std::string_view inf) {
  Array arr;
  arr.n = 0;
  while (inf.size() > 0) {
    inf = trim_start(inf);
    while(isspace(inf[0]) != (inf[0] == ',')) inf = inf.substr(1);
    char *out;
    arr.a[arr.n] = strtod(inf.data(), &out);
    inf = inf.substr(out - inf.data());
    arr.n++;
  }

  return arr;
} 

static Transform translate(Array arr) {
  Transform transform = Transform::identity();
  if (arr.n == 2) {
    transform.d[0] = arr.a[0];
    transform.d[1] = arr.a[1];
  } else {
    transform.d[0] = arr.a[0];
  }

  return transform;
}

static Transform matrix(Array arr) {
  Transform transform;
  transform.m[0][0] = arr.a[0]; 
  transform.m[1][0] = arr.a[1]; 
  transform.m[0][1] = arr.a[2]; 
  transform.m[1][1] = arr.a[3];
  transform.d[0] = arr.a[4], 
  transform.d[1] = arr.a[5];

  return transform;
}

static Transform scale(Array arr) {
  Transform transform = Transform::zeros();
  if (arr.n == 2) {
    transform.m[0][0] = arr.a[0];
    transform.m[1][1] = arr.a[1];
  } else {
    transform.m[0][0] = arr.a[0];
    transform.m[1][1] = arr.a[0];
  }

  return transform;
}

static Transform rotate(Array arr) {
  Transform transform;
  double angle = arr.a[0] * PI / 180;
  transform.m[0][0] = std::cos(angle); 
  transform.m[0][1] = -std::sin(angle);
  transform.m[1][0] = std::sin(angle);
  transform.m[1][1] = std::cos(angle);
  transform.d[0] = 0;
  transform.d[1] = 0;
 
  if (arr.n > 1) {
    Array translate_arr;
    translate_arr.a[0] = arr.a[1];
    translate_arr.a[1] = arr.a[2];
    translate_arr.n = arr.n - 1;
    transform = translate(translate_arr) * transform;

    translate_arr.a[0] = -arr.a[1];
    translate_arr.a[1] = -arr.a[2];
    transform = transform * translate(translate_arr);
  }

  return transform;
}

static Transform skewX(Array arr) {
  Transform transform = Transform::identity();
  transform.m[0][1] = std::tan(arr.a[0] * PI / 180);

  return transform;  
}

static Transform skewY(Array arr) {
  Transform transform = Transform::identity();
  transform.m[1][0] = std::tan(arr.a[0] * PI / 180);

  return transform;
}

Transform (*transform_fns[TRANSFORM_COUNT])(Array arr) = {
  matrix,
  translate,
  scale,
  rotate,
  skewX,
  skewY,
};

static Transform solve_transform(std::string_view inf) {
  int end = inf.find('(');
  std::string_view str_type = inf.substr(0, end);

  inf = inf.substr(end + 1);

  int type = inv_transform[str_type];
  if (type != -1) {
    return transform_fns[type](split_number(inf));
  }

  return Transform::identity();
}

static Transform convert_transform(std::string_view value) {
  Transform transform = Transform::identity();
  
  while (value.size() > 0) {
    value = trim_start(value);
    int end = value.find(')');
    std::string_view type = value.substr(0, end);
    transform = transform * solve_transform(type);

    value = value.substr(end + 1);
  }
  return transform;
}

static std::string_view remove_spaces_end(std::string_view data) {
  while (isspace(data[0])) data = data.substr(1);
  while (isspace(data[data.size() - 1])) data = data.substr(0, data.size() - 1);

  return data;
}

static ArrayList<Attribute> process_style(std::string_view value) {
  ArrayList<Attribute> arr;
  while (value.size() > 0) {
    value = trim_start(value);
    size_t end = value.find(';');
    if (end > value.size()) end = value.size(); 
    std::string_view str = value.substr(0, end);
    
    size_t pos = str.find(':');
    Attribute attr;
    attr.key = str.substr(0, pos);
    attr.key = remove_spaces_end(attr.key);

    attr.value = str.substr(pos + 1);
    attr.value = remove_spaces_end(attr.value);
    arr.push(attr);

    if (end != value.size()) value = value.substr(end + 1);
    else value = value.substr(end);
  }

  return arr;
}


static void apply_style(BaseShape *shape, BaseShape *parent, Attribute *attrs, int attrs_count) {
  for (int i = 0; i < attrs_count; i++) {
    std::string_view key = attrs[i].key;
    std::string_view value = attrs[i].value;

    switch ((StyleType) inv_style[key]) {
      case STYLE_VISIBLE: {
        if (value != "visible") shape->visible = false;
      } break;

      case STYLE_FILL: {
        shape->fill = read_paint(value);
      } break;

      case STYLE_STROKE: {
        shape->stroke = read_paint(value);
      } break;

      case STYLE_FONT_SIZE: {
        shape->font_size = strtod(value.data(), nullptr);
      } break;

      case STYLE_OPACITY: {
        shape->opacity = convert_opacity(value);
      } break;

      case STYLE_FILL_OPACITY: {
        shape->fill_opacity = convert_opacity(value);
      } break;

      case STYLE_STROKE_OPACITY: {
        shape->stroke_opacity = convert_opacity(value);
      } break;

      case STYLE_STROKE_WIDTH: {
        shape->stroke_width = strtod(value.data(), nullptr);
      } break;

      case STYLE_STROKE_DASH_OFFSET: {
        shape->stroke_dash_offset = strtod(value.data(), nullptr);
      } break;

      case STYLE_STROKE_DASH_ARRAY: {
        convert_array(value, shape->stroke_dash_array, &shape->stroke_dash_count);
      } break;

      case STYLE_STROKE_LINE_JOIN: {
        int type = inv_linejoin[value];
        if (type != -1) shape->stroke_line_join = (StrokeLineJoin) type;
      } break;

      case STYLE_STROKE_LINE_CAP: {
        int type = inv_linecap[value];
        if (type != -1) shape->stroke_line_cap = (StrokeLineCap)type;
      } break;

      case STYLE_MITER_LIMIT: {
        shape->miter_limit = strtod(value.data(), nullptr);
      } break;

      case STYLE_FILL_RULE: {
        int type = inv_fillrule[value];
        if (type != -1) shape->fill_rule = (FillRule)type;
      } break;

      case STYLE_FONT_STYLE: {
        shape->font_style = (FontStyle)inv_fontstyle[value];
      } break;

      case STYLE_FONT_WEIGHT: {
        int type = inv_fontweight[value];
        if (type == -1) {
          shape->font_weight = strtod(value.data(), nullptr);
        }  else {
          switch ((FontWeight)type) {
            case FONTWEIGHT_NORMAL: {
              shape->font_weight = 400;
            } break;
            case FONTWEIGHT_BOLD: {
              shape->font_weight = 700;
            } break;
            case FONTWEIGHT_BOLDER: {
              if (parent == nullptr) {
                shape->font_weight = 700;
              } else {
                if (parent->font_weight < 100) {
                  shape->font_weight = 100;
                } else if (parent->font_weight < 400) {
                  shape->font_weight = 400;
                } else if (parent->font_weight < 700) {
                  shape->font_weight = 700;
                } else if (parent->font_weight < 900){
                  shape->font_weight = 900;
                } else {
                  shape->font_weight = parent->font_weight;
                }
              } 
            } break;
            case FONTWEIGHT_LIGHTER: {
              if (parent == nullptr) {
                shape->font_weight = 100;
              } else {
                if (parent->font_weight > 900) {
                  shape->font_weight = 900;
                } else if (parent->font_weight > 700) {
                  shape->font_weight = 700;
                } else if (parent->font_weight > 400) {
                  shape->font_weight = 400;
                } else if (parent->font_weight > 100){
                  shape->font_weight = 100;
                } else {
                  shape->font_weight = parent->font_weight;
                }
              } 
            } break;
            case FONTWEIGHT_COUNT: {
              __builtin_unreachable();
            }
          }
        } 
      } break;
      
      case STYLE_FONT_FAMILY: {
        shape->font_family = value;
      } break;

      case STYLE_COUNT: {
        __builtin_unreachable();
      }
    }
  }
}

BaseShape::BaseShape(Attribute *attrs, int attrs_count, BaseShape *parent) {
  if (parent == nullptr) {
    this->visible = true;
    this->fill = Paint::new_rgb(0, 0, 0);
    this->stroke = Paint::new_transparent();
    this->font_size = 16;
    this->opacity = 1.0;
    this->fill_opacity = 1.0;
    this->stroke_opacity = 1.0;
    this->stroke_width = 1.0;
    this->stroke_dash_offset = 0.0;
    this->stroke_dash_count = 0;
    this->stroke_line_join = StrokeLineJoin::LINE_JOIN_MITER;
    this->stroke_line_cap = StrokeLineCap::LINE_CAP_BUTT;
    this->miter_limit = 4;
    this->transform = Transform::identity();
    this->fill_rule = FillRule::FILL_RULE_NONZERO;
    this->font_style = FontStyle::FONTSTYLE_NORMAL;
    this->font_weight = 400;
    this->font_family = "serif";
  } else {
    this->visible = parent->visible;
    this->fill = parent->fill;
    this->stroke = parent->stroke;
    this->font_size = parent->font_size;
    this->opacity = parent->opacity;
    this->fill_opacity = parent->fill_opacity;
    this->stroke_opacity = parent->stroke_opacity;
    this->stroke_width = parent->stroke_width;
    this->stroke_dash_offset = parent->stroke_dash_offset;
    this->stroke_dash_count = parent->stroke_dash_count;
    this->stroke_line_join = parent->stroke_line_join;
    this->stroke_line_cap = parent->stroke_line_cap;
    this->miter_limit = parent->miter_limit;
    this->transform = parent->transform;
    this->fill_rule = parent->fill_rule;
    this->font_style = parent->font_style;
    this->font_weight = parent->font_weight;
    this->font_family = parent->font_family;
  }

  for (int i = 0; i < attrs_count; i++) {
    std::string_view key = attrs[i].key;
    std::string_view value = attrs[i].value;
  
    switch ((AttributeType)inv_attribute[key]) {
      case ATTRIBUTE_TRANSFORM: {
        this->transform = this->transform * convert_transform(value);
      } break;

      case ATTRIBUTE_STYLE: {
        ArrayList<Attribute> attrs_style = process_style(value);
        apply_style(this, parent, attrs_style.begin(), attrs_style.len());
      } break;

      case ATTRIBUTE_COUNT: {
        __builtin_unreachable();
      }
    }
  }

  apply_style(this, parent, attrs, attrs_count);
}

ArrayList<BezierCurve> BaseShape::get_beziers() const {
  return ArrayList<BezierCurve> {};
}
