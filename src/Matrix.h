#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>
#include <type_traits>

template<typename T, size_t M, size_t N>
struct Matrix {
  static constexpr size_t NROWS = M;
  static constexpr size_t NCOLS = N;
  T data[NROWS * NCOLS];

  constexpr std::conditional_t<NCOLS != 1, const T *, T> operator[](size_t idx) const {
    if constexpr (N != 1) {
      return data + NCOLS * idx;
    } else {
      return data[idx];
    }
  }

  constexpr std::conditional_t<NCOLS != 1, T *, T&> operator[](size_t idx) {
    if constexpr (N != 1) {
      return data + NCOLS * idx;
    } else {
      return data[idx];
    }
  }

  template<size_t P>
  constexpr Matrix<T, M, P> operator*(Matrix<T, N, P> other) const {
    Matrix<T, M, P> result = {};
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < P; ++j) {
        for (size_t k = 0; k < N; ++k) result.data[P * i + j] += data[N * i + k] * other.data[P * k + j];
      }
    }
    return result;
  }

  constexpr Matrix operator-() const {
    Matrix result;
    for (size_t i = 0; i < M * N; ++i) {
      result.data[i] = -this->data[i];
    }
    return result;
  }

  constexpr Matrix operator+(Matrix other) const {
    Matrix result;
    for (size_t i = 0; i < M * N; ++i) {
      result.data[i] = this->data[i] + other.data[i];
    }
    return result;
  }

  constexpr Matrix operator-(Matrix other) const {
    Matrix result;
    for (size_t i = 0; i < M * N; ++i) {
      result.data[i] = this->data[i] - other.data[i];
    }
    return result;
  }

  constexpr Matrix operator*(T other) const {
    Matrix result;
    for (size_t i = 0; i < M * N; ++i) {
      result.data[i] = this->data[i] * other;
    }
    return result;
  }

  constexpr friend Matrix operator*(T a, Matrix b) {
    return b * a;
  }
  
  constexpr Matrix operator/(T other) const {
    Matrix result;
    for (size_t i = 0; i < M * N; ++i) {
      result.data[i] = this->data[i] / other;
    }
    return result;
  }

  constexpr static Matrix zeros() {
    Matrix result;
    for (size_t i = 0; i < NROWS * NCOLS; ++i) result.data[i] = 0;
    return result;
  }

  constexpr static Matrix identity() {
    static_assert(NROWS == NCOLS, "Non-square identity matrix not supported");
    Matrix result = Matrix::zeros();
    for (size_t i = 0, j = 0; i < NROWS; ++i, j += NCOLS + 1) {
      result.data[j] = 1;
    }
    return result;
  }
};

template<typename T, size_t N>
struct AffineMatrix {
  Matrix<T, N, N> m;
  Matrix<T, N, 1> d;

  constexpr static AffineMatrix identity() {
    return AffineMatrix {
      Matrix<T, N, N>::identity(),
      Matrix<T, N, 1>::zeros()
    };
  }

  constexpr static AffineMatrix from_translate(Matrix<T, N, 1> translate) {
    return AffineMatrix {
      Matrix<T, N, N>::identity(),
      translate
    };
  }

  constexpr static AffineMatrix from_data(const T (&data)[(N + 1) * N]) {
    AffineMatrix matrix;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        matrix.m.data[N * i + j] = data[(N + 1) * i + j];
      }
    }

    for (size_t i = 0; i < N; ++i) {
      matrix.d.data[i] = data[(N + 1) * i + N];
    }
    return matrix;
  }

  constexpr AffineMatrix operator+(AffineMatrix other) const {
    return AffineMatrix {
      this->m + other.m,
      this->d + other.d,
    };
  }

  constexpr AffineMatrix operator*(AffineMatrix other) const {
    return AffineMatrix {
      this->m * other.m,
      this->m * other.d + this->d, 
    };
  }
};

using Transform = AffineMatrix<double, 2>;
using Point = Matrix<double, 2, 1>;

#endif