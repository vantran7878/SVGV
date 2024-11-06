#ifndef CIRCLE_H
#define CIRCLE_H

#include "BaseShape.h"

class Circle final : public BaseShape {
public:
  double cx;
  double cy;
  double r;

  Circle(Attribute *attrs, int attrs_count, BaseShape *parent);
  void draw() const override;
};

#endif
