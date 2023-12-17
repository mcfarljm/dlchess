#ifndef INFERENCE_H
#define INFERENCE_H

#include <optional>
#include <array>

#include <onnxruntime_cxx_api.h>

#include "tensor.h"


namespace zero {

  class InferenceModel {

    Ort::MemoryInfo memory_info{nullptr};

    Ort::Env env;
    Ort::Session session{nullptr};
    // Since there is only one input name, we can get the necessary pointer with
    // input_name.c_str() and pass that to session.Run.  Thus, we don't need to
    // store a "char" version separately.
    std::string input_name;
    std::array<const char*, 1> input_names_char;
    std::array<std::string, 2> output_names;
    std::array<const char*, 2> output_names_char;

  public:
    InferenceModel(const char* model_path, std::optional<int> num_threads) {

      memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
      Ort::SessionOptions sessionOptions;
      if (num_threads)
        sessionOptions.SetIntraOpNumThreads(num_threads.value());
      session = Ort::Session(env, model_path, sessionOptions);

      // Get the input and output names:
      assert(session.GetInputCount() == 1);
      assert(session.GetOutputCount() == 2);
      Ort::AllocatorWithDefaultOptions allocator;
      input_name = session.GetInputNameAllocated(0, allocator).get();
      input_names_char[0] = input_name.c_str();
      for (int i=0; i<session.GetOutputCount(); ++i) {
        output_names[i] = session.GetOutputNameAllocated(i, allocator).get();
      }
      std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
                     [&](const std::string& str) { return str.c_str(); });

    }

    std::array<Tensor<float>, 2> operator() (Tensor<float>& input_tensor) {
      auto ort_input_value = Ort::Value::CreateTensor<float>(memory_info,
                                                          input_tensor.data.data(), input_tensor.data.size(),
                                                          input_tensor.shape.data(), input_tensor.shape.size());

      std::vector<int64_t> policy_shape = {1, PRIOR_SHAPE[0], PRIOR_SHAPE[1], PRIOR_SHAPE[2]};
      std::vector<int64_t> value_shape = {1, 1};
      // Pre-allocate the memory for the results:
      std::array<Tensor<float>, 2> output_tensors = {
        Tensor<float>(policy_shape),
        Tensor<float>(value_shape),
      };

      Ort::Value ort_outputs[] = {
        Ort::Value::CreateTensor<float>(memory_info,
                                        output_tensors[0].data.data(), output_tensors[0].data.size(),
                                        output_tensors[0].shape.data(), output_tensors[0].shape.size()),
        Ort::Value::CreateTensor<float>(memory_info,
                                        output_tensors[1].data.data(), output_tensors[1].data.size(),
                                        output_tensors[1].shape.data(), output_tensors[1].shape.size()),
      };

      session.Run(Ort::RunOptions{nullptr}, input_names_char.data(), &ort_input_value, 1,
                  output_names_char.data(), ort_outputs, output_names.size());
      return output_tensors;


      // A simpler alternative was to return ORT allocated vector of outputs,
      // but then they are ORT Value tensors, so we don't get to use our wrapper
      // for accessing them.
      // return session.Run(Ort::RunOptions{nullptr}, input_names_char.data(), &ort_input_value, 1,
      //                    output_names_char.data(), output_names.size());
    }

  };

};

#endif // INFERENCE_H
