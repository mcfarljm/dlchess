#include <fstream>
#include <filesystem>
#include <typeinfo>

#include "experience.h"

namespace zero {

  namespace {

    void write_metadata(const std::vector<Tensor<float>>& tensors, const std::string& directory, const std::string& name) {

      // We assume that each entry in the collection is a tensor containing a
      // single item.  This allows us to determine the total length of the first
      // axis as the number of items in the vector.
      for (const auto& tensor : tensors)
        assert(tensor.shape[0] == 1);

      if (tensors.empty())
        return;

      auto json_path = std::filesystem::path(directory) / (name + ".json");
      auto dim = tensors[0].dim();

      std::ofstream fout(json_path, std::ios::out);
      fout << "{\n  \"data\": \"" << name << ".dat" << "\",\n";

      fout << R"(  "dtype": ")";
      if (typeid(tensors[0]) == typeid(Tensor<float>))
        fout << "float32";
      else if (typeid(tensors[0]) == typeid(Tensor<int16_t>))
        fout << "int16";
      else if (typeid(tensors[0]) == typeid(Tensor<int32_t>))
        fout << "int32";
      else
        throw std::runtime_error("unexpected tensor dtype");
      fout << "\",\n";

      fout << "  \"shape\": [";
      for (auto i=0; i<dim; ++i) {
        if (i == 0)
          fout << tensors.size();
        else
          fout << tensors[0].shape[i];
        if (i + 1 < dim)
          fout << ", ";
      }
      fout << "],\n";

      fout << "  \"strides\": [";
      for (auto i=0; i<dim; ++i) {
        fout << tensors[0].strides[i];
        if (i+1 < dim)
          fout << ", ";
      }
      // Note: no trailing comma for pure json compatibility
      fout << "]\n";
      fout << "}\n";

      fout.close();
    }

    // Overload for handling case where the data are stored in a simple vector
    // (i.e., the actual data are scalars, so the collection of records is just
    // one-dimensional).
    void write_metadata(const std::vector<float>& vec, const std::string& directory, const std::string& name) {

      if (vec.empty())
        return;

      auto json_path = std::filesystem::path(directory) / (name + ".json");

      std::ofstream fout(json_path, std::ios::out);
      fout << "{\n  \"data\": \"" << name << ".dat" << "\",\n";

      fout << R"(  "dtype": ")";
      if (typeid(vec) == typeid(std::vector<float>))
        fout << "float32";
      else if (typeid(vec) == typeid(std::vector<int16_t>))
        fout << "int16";
      else if (typeid(vec) == typeid(std::vector<int32_t>))
        fout << "int32";
      else
        throw std::runtime_error("unexpected tensor dtype");
      fout << "\",\n";

      fout << "  \"shape\": [";
      fout << vec.size();
      fout << "],\n";

      fout << "  \"strides\": [";
      fout << 1;
      // Note: no trailing comma for pure json compatibility
      fout << "]\n";
      fout << "}\n";

      fout.close();
    }

    void serialize_vector(std::vector<float>& vec, const std::string& directory, const std::string&name) {
      if (vec.empty())
        return;

      write_metadata(vec, directory, name);

      auto data_path = std::filesystem::path(directory) / (name + ".dat");
      std::ofstream fout(data_path, std::ios::out | std::ios::binary);
      fout.write(reinterpret_cast<char*>(vec.data()), sizeof(std::remove_reference_t<decltype(vec)>::value_type) * vec.size()); // NOLINT(bugprone-narrowing-conversions)
      fout.close();
    }

    // Serialize a vector of tensors so that they are concatenated along the
    // first axis.
    void serialize_tensors(std::vector<Tensor<float>>& tensors, const std::string& directory, const std::string& name) {
      if (tensors.empty())
        return;

      write_metadata(tensors, directory, name);

      auto data_path = std::filesystem::path(directory) / (name + ".dat");
      std::ofstream fout(data_path, std::ios::out | std::ios::binary);
      for (auto& tensor : tensors)
        fout.write(reinterpret_cast<char*>(tensor.data.data()), sizeof(decltype(tensor.data)::value_type) * tensor.data.size()); // NOLINT(bugprone-narrowing-conversions)
      fout.close();
    }
  }


  void ExperienceCollector::serialize_binary(const std::string& directory, const std::string& label) {
    if (! states.size())
      return;

    if (std::filesystem::exists(directory)) {
      if (! std::filesystem::is_directory(directory))
        throw std::runtime_error("path exists and is not a directory: " + directory);
    }
    else
      std::filesystem::create_directory(directory);
  
    serialize_tensors(states, directory, "states" + label);
    serialize_tensors(visit_counts, directory, "visit_counts" + label);
    serialize_vector(rewards, directory, "rewards" + label);
  }

};
