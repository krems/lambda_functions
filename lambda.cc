// Author: Krems

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>

template <class E>
class Expression {
 public:
  operator E& () {
    return static_cast<E&>(*this);
  }
  operator const E& () {
    return static_cast<const E&>(*this);
  }

  int operator () (int arg) const {
    return static_cast<const E*>(this)->operator() (arg);
  }
  int operator () (int arg1, int arg2) const {
    return static_cast<const E*>(this)->operator() (arg1, arg2);
  }
};

class Constant : public Expression<Constant> {
 public:
  Constant(int value): value_(value) {}

  int operator () (int) const {
    return value_;
  }
  int operator () (int, int) const {
    return value_;
  }

 private:
  int value_;
};

class Identity1 : public Expression<Identity1> {
 public:
  int operator () (int arg) const {
    return arg;
  }

  int operator () (int arg1, int arg2) const {
    return arg1;
  }
};

class Identity2 : public Expression<Identity2> {
 public:
  int operator () (int arg) const {
    return arg;
  }

  int operator () (int arg1, int arg2) const {
    return arg2;
  }
};
  
template <typename T>
class RefHolder : Expression<RefHolder<T> > {
 public:
  RefHolder(T& to_hold): to_hold_(to_hold) {}

  template<typename Other>
  T& operator () (Other arg) {
    return to_hold_;
  }
  
 private:
  T& to_hold_;
};

class IdentityWithEq : public Identity1 {
 public:
  int operator () (int& ptr) {
    return ptr = value_;
  }
  
  IdentityWithEq& operator = (int value) {
    value_ = value;
    return *this;
  }
  
 private:
  int value_;
};

// Arithmetics
template <typename L, typename R>
class PlusExpression : public Expression<PlusExpression<L, R> > {
 public:
  PlusExpression(const L& left, const R& right)
      : left_(left), right_(right) {}

  int operator () (int arg) const {
    return left_(arg) + right_(arg);
  }

 private:
  L left_;
  R right_;
};

template <typename E1, typename E2>
PlusExpression<Expression<E1>, Expression<E2> > operator + (
    const Expression<E1>& lhs,
    const Expression<E2>& rhs) {
  return PlusExpression<Expression<E1>, Expression<E2> >(lhs,
                                                         rhs);
}

template <typename E>
PlusExpression<Expression<E>, Constant> operator + (
    const Expression<E>& lhs,
    int rhs) {
  return PlusExpression<Expression<E>, Constant>(lhs,
                                                 Constant(rhs));
}

template <typename E>
PlusExpression<Constant, Expression<E> > operator + (
    int lhs,
    const Expression<E>& rhs) {
  return PlusExpression<Constant, Expression<E> >(Constant(lhs),
                                                  rhs);
}
// and so on for multiplication and division
template <typename L, typename R>
class MinusExpression
    : public Expression<MinusExpression<L, R> > {
 public:
  MinusExpression(const L& left, const R& right)
      : left_(left), right_(right) {}

  int operator () (int arg) const {
    return left_(arg) - right_(arg);
  }

 private:
  L left_;
  R right_;
};

template <typename E1, typename E2>
MinusExpression<Expression<E1>, Expression<E2> > operator - (
    const Expression<E1>& lhs,
    const Expression<E2>& rhs) {
  return MinusExpression<Expression<E1>, Expression<E2> >(lhs,
                                                         rhs);
}

template <typename E>
MinusExpression<Expression<E>, Constant> operator - (
    const Expression<E>& lhs,
    int rhs) {
  return MinusExpression<Expression<E>, Constant>(lhs,
                                                 Constant(rhs));
}

template <typename E>
MinusExpression<Constant, Expression<E> > operator - (
    int lhs,
    const Expression<E>& rhs) {
  return MinusExpression<Constant, Expression<E> >(Constant(lhs),
                                                   rhs);
}
// Compare
template <typename L, typename R>
class LessExpression : public Expression<LessExpression<L, R> > {
 public:
  LessExpression(const L& lhs, const R& rhs):
      lhs_(lhs), rhs_(rhs) {}

  // template <typename A1>
  // bool operator () (const A1& a1) const {
  //   return lhs_(a1) < rhs_(a1);
  // }

  template <typename A1, typename A2>
  bool operator () (const A1& a1, const A2& a2) const {
    return lhs_(a1, a2) < rhs_(a1, a2);
  }

 private:
  L lhs_;
  R rhs_;
};

template <typename L, typename R>
class GreaterExpression :
    public Expression<GreaterExpression<L, R> > {
 public:
  GreaterExpression(const L& lhs, const R& rhs):
      lhs_(lhs), rhs_(rhs) {
  }

  template <typename A1>
  bool operator () (const A1& a1) const {
    return lhs_(a1) > rhs_(a1);
  }

  template <typename A1, typename A2>
  bool operator () (const A1& a1, const A2& a2) const {
    return lhs_(a1, a2) > rhs_(a1, a2);
  }

 private:
  L lhs_;
  R rhs_;
};

template <typename E1, typename E2>
GreaterExpression<E1, E2> operator > (const Expression<E1>& lhs,
                                      const Expression<E2>& rhs) {
  return GreaterExpression<E1, E2>(lhs, rhs);
}

template <typename E1, typename E2>
LessExpression<E1, E2> operator < (const Expression<E1>& lhs,
                                   const Expression<E2>& rhs) {
  return LessExpression<E1, E2>(lhs, rhs);
}

// Output
template <typename L, typename R>
class OutputExpression:
    public Expression<OutputExpression<L,R> > {
 public:
  OutputExpression(const L& lhs, const R& rhs):
      left_(lhs), right_(rhs) {}

  template <typename T>
  void operator () (T &arg) {
    left_(arg) << right_(arg);
  }
 private:
  L left_;
  R right_;
};

template <typename T>
OutputExpression<RefHolder<T>, Identity1>
operator << (T& lhs, Identity1& rhs) {
  return OutputExpression<RefHolder<T>, Identity1>
      (RefHolder<T>(lhs), rhs);
}

IdentityWithEq _1;
Identity2 _2;

int main() {
  int a[] = { 7, 1, 2, 3, 4, 5 };
  std::sort(a, a + 6, _1 < _2);
  //  std::sort(a, a + 6, _2 < _1);
    std::for_each(a, a + 6, std::cout << _1 ); //<< " ");
    std::for_each(a, a + 5, _1 = 100);
   std::transform(a,
                  a + 6,
                  std::ostream_iterator<int>(std::cout, " "),
                  3 + _1 - 1);
  std::cout << std::endl;
  return 0;
}


