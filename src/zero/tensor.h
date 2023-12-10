#ifndef TENSOR_H_
#define TENSOR_H_

#include <vector>
#include <initializer_list>
#include <numeric>
#include <cassert>
#include <cstdint>


namespace zero {

  template <class T>
  struct Tensor {
    std::vector<T> data;
    std::vector<int64_t> shape;
    std::vector<int64_t> strides;

    Tensor(const std::vector<int64_t>& shape) :
      shape(shape) {

      // Initialize data.  Todo: there might be a way to do this in the
      // initializer list, using a constexpr function for product.
      int n = 1;
      for (auto& i : shape) n *= i;
      data.resize(n);

      strides.resize(shape.size(), 1);
      int val = 1;
      for (int i=strides.size()-2; i>=0; --i) {
        val *= shape[i+1];
        strides[i] = val;
      }
    }

    int dim() const {
      return shape.size();
    }

    // void clear() {
    //   data.clear();
    //   shape[0] = 0;
    // }

    T& at(std::initializer_list<int64_t> indices) {
      assert(indices.size() == shape.size());
      auto index = std::inner_product(indices.begin(), indices.end(), strides.begin(), 0);
      return data[index];
    }

    // Assuming shape NCWH, fill the data at [idx0, idx1, :, :]
    void fill_channel(int64_t idx0, int64_t idx1, T val) {
      assert(shape.size() == 4);
      for (int i=0; i<shape[2]; ++i) {
        for (int j=0; j<shape[3]; ++j) {
          at({idx0, idx1, i, j}) = val;
        }
      }
    }

    // void append(const Tensor<T>& other) {
    //   assert(shape.size() == other.shape.size());
    //   for (int i=1; i<shape.size(); ++i)
    //     assert(shape[i] == other.shape[i]);
    //   data.insert(data.end(), other.begin(), other.end());
    //   ++shape[0];
    // }
  };

};


#endif // TENSOR_H_
