#include <iostream>
#include <cstddef>
#include <iterator>
#include <vector>
#include <fstream>
#include <sstream>

#include <errno.h>
#include <system_error>

#include <cstddef>  // std::byte
#include <vector>
#include <fstream>
#include <type_traits>


#include <filesystem>
#include <array>



template<class T>
using InputIteratorToChar = typename std::enable_if_t<
    std::is_same_v<typename std::iterator_traits<T>::value_type, char>
      and std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>,
    T>;

template<class Iter>
struct iterator_char_to_byte : public InputIteratorToChar<Iter> {
  using base_type = InputIteratorToChar<Iter>;

  using value_type = std::byte;
  using pointer    = const std::byte*;
  using reference  = const std::byte&;

  iterator_char_to_byte(const iterator_char_to_byte<Iter>&) noexcept = default;
  explicit iterator_char_to_byte(const Iter& iter) noexcept : base_type(iter) {}
  value_type operator*() const noexcept { return static_cast<std::byte>(base_type::operator*()); }
};


#include <iterator>

struct istreambuf_iterator_byte : public std::istreambuf_iterator<char> {
  using base_type = std::istreambuf_iterator<char>;

  static_assert(std::is_same_v<base_type::iterator_category, std::input_iterator_tag>, "Stronger iterator requires more methods to be reimplemented here.");

  using value_type = std::byte;
  using reference  = std::byte;

  std::byte operator*() const noexcept(noexcept(base_type::operator*())) {
    return static_cast<std::byte>(base_type::operator*());
  }
  istreambuf_iterator_byte& operator++() noexcept(noexcept(base_type::operator++())) {
    base_type::operator++();
    return *this;
  }
  istreambuf_iterator_byte operator++(int) noexcept(noexcept(base_type::operator++(int{}))) {
    return istreambuf_iterator_byte{base_type::operator++(int{})};
  }
};

#if defined __cpp_concepts
static_assert(std::input_iterator<istreambuf_iterator_byte>);
#endif

auto read_binary_file_iterators(const std::string& file_path) -> std::vector<std::byte> {
  std::ifstream input_file(file_path, std::ios::binary);
  return {istreambuf_iterator_byte{input_file}, istreambuf_iterator_byte{}};
}

auto read_binary_file_iterators_prealloc(const std::string& file_path) -> std::vector<std::byte> {
  std::ifstream input_file(file_path, std::ios::binary);
  auto result = std::vector<std::byte>{};
  input_file.seekg(0, std::ios::end);
  result.reserve(input_file.tellg());
  input_file.seekg(0, std::ios::beg);
  result.assign(istreambuf_iterator_byte{input_file}, istreambuf_iterator_byte{});
  return result;
}

auto read_binary_file_fast(const std::string& file_path) -> std::vector<std::byte> {
  char buffer_decoy;

  std::filebuf file_buf;
  file_buf.pubsetbuf(&buffer_decoy, 1);  // If `nullptr` provided then `filebuf` will allocate 1 byte in heap.
  file_buf.open(file_path, std::ios::binary | std::ios::in);

  constexpr auto block_size = size_t{BUFSIZ};
  auto result = std::vector<std::byte>{};
  {
    const auto seek_error = std::filebuf::pos_type(std::filebuf::off_type(-1));
    const auto file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
    if (file_size == seek_error) {
      throw std::runtime_error("cannot get size of file \"" + file_path + "\"");
    }
    result.reserve(block_size + file_size);
    file_buf.pubseekoff(0, std::ios::beg, std::ios::in);
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + block_size);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - block_size, block_size);
    if (read_bytes != block_size) {
      result.resize(result.size() + read_bytes - block_size);
      break;
    }
  }

  return result;
}

#if defined __has_include
  #if __has_include (<expected>)
    #include <expected>
  #endif
  #if __has_include (<system_error>)
    #define CPP_LIB_SYSTEM_ERROR 201402L
    #include <system_error>
  #endif
#endif

#if defined __cpp_lib_expected and defined CPP_LIB_SYSTEM_ERROR

auto read_binary_file_fastfast(const std::filesystem::path& file_path) -> std::expected<std::vector<std::byte>, std::system_error> {
  auto buffer_decoy = std::array<char, sizeof(size_t)>{};

  std::filebuf file_buf;
  file_buf.pubsetbuf(buffer_decoy.data(), buffer_decoy.size());  // In libstdc++ if `nullptr` provided then `filebuf` will allocate 1 byte in heap.
  file_buf.open(file_path, std::ios::binary | std::ios::in);

  if (not file_buf.is_open()) {
    const auto error = std::make_error_code(std::errc(errno));
    return std::unexpected(std::system_error(error, "cannot open file \"" + file_path.string() + "\""));
  }

  auto result = std::vector<std::byte>{};
  {
    const auto file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
    const auto seek_error = std::filebuf::pos_type(std::filebuf::off_type(-1));
    if (file_size == seek_error) {
      const auto error = std::make_error_code(std::errc(errno));
      return std::unexpected(std::system_error(error, "cannot seek file \"" + file_path.string() + "\""));
    }

    // `lseek` on unix OS returns INTPTR_MAX on directory file descriptor.
    // In 32bit system we easily can encounter 2GiB file, so we check this just for 64bit systems.
  #if INTPTR_MAX > INT32_MAX && defined __unix__
    const auto seek_directory = std::filebuf::pos_type(std::numeric_limits<intptr_t>::max());
    if (file_size == seek_directory) {
      const auto error = std::make_error_code(std::errc::is_a_directory);
      return std::unexpected(std::system_error(error, "cannot seek file \"" + file_path.string() + "\""));
    }
  #endif
    file_buf.pubseekoff(0, std::ios::beg, std::ios::in);
    result.resize(file_size);
  }

  try {
    const auto read_bytes = file_buf.sgetn(reinterpret_cast<char*>(&result[0]), result.size());
    result.resize(read_bytes);
  } catch (const std::system_error& error) {
    return std::unexpected(std::system_error(error.code(), "Error reading file \"" + file_path.string() + "\""));
  }
  return result;
}
#endif // if defined __cpp_lib_expected and defined CPP_LIB_SYSTEM_ERROR

// void* operator new[] (std::size_t size) {
//     printf("allocating[] %lu bytes\n", size);
//     auto ptr = malloc(size);
//     if (ptr == nullptr) {
//       throw std::bad_alloc();
//     }
//     return ptr;
// }

// void* operator new (std::size_t size) {
//     printf("allocating %lu bytes\n", size);
//     auto ptr = malloc(size);
//     if (ptr == nullptr) {
//       throw std::bad_alloc();
//     }
//     if (size == (1024*1024)) {
//       // throw std::runtime_error("test");
//     }
//     return ptr;
// }

#include <array>
#include <numeric>

auto read_binary_file_simple(const std::string& file_path) -> std::vector<std::byte> {
  std::ifstream input_file(file_path, std::ios::binary);

  input_file.seekg(0, std::ios::end);
  const auto file_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  std::vector<std::byte> result(file_size);
  input_file.read(reinterpret_cast<char*>(&result[0]), file_size);
  result.resize(input_file.tellg());
  return result;
}




#include <system_error>
#include <filesystem>


#if defined __linux__
    #include <unistd.h>  // maybe macro _POSIX_MAPPED_FILES
    #if defined _POSIX_MAPPED_FILES
        #include <fcntl.h>  // open
        #include <sys/stat.h>  // fstat
        #include <sys/mman.h>  // mmap
        #define MMAP_AVAILABLE 1
    #endif
#endif


#if defined MMAP_AVAILABLE and __cplusplus > 201703L

    #include <expected>

    auto read_file(const std::filesystem::path& path) -> std::expected<std::vector<std::byte>, std::system_error> {
        const auto file_descriptor = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (file_descriptor == -1) {
            const auto error_code = std::make_error_code(std::errc{errno});
            return std::unexpected(std::system_error(error_code, "cannot open file \"" + path.string() + "\""));
        }

        struct stat file_stat = {};
        void* data_ptr = nullptr;
        {
            struct scoped_file_descriptor {
                decltype(file_descriptor) fd = -1;
                ~scoped_file_descriptor() { if (fd != -1) { ::close(fd); } }
            };
            const scoped_file_descriptor scoped_fd(file_descriptor);

            if (fstat(file_descriptor, &file_stat) == -1) {
                const auto error_code = std::make_error_code(std::errc{errno});
                return std::unexpected(std::system_error(error_code, "cannot get size of file \"" + path.string() + "\""));
            }

            const auto offset = off_t{0};
            data_ptr = mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, offset);
            if (data_ptr == MAP_FAILED) {
                const auto error_code = std::make_error_code(std::errc{errno});
                return std::unexpected(std::system_error(error_code, "cannot memory map file \"" + path.string() + "\""));
            }
        }

        struct scoped_mmaping {
            void* mmaped_ptr = nullptr;
            size_t mmaped_len = 0;
            ~scoped_mmaping() noexcept(false) {
                if (mmaped_ptr == nullptr) {
                    return;
                }
                if (::munmap(mmaped_ptr, mmaped_len) != -1) {
                    return;
                }
                const auto error_code = std::make_error_code(std::errc{errno});
                throw std::system_error(error_code, "logic error, cannot memory unmap address \"" + std::to_string(reinterpret_cast<std::uintptr_t>(mmaped_ptr)) + "\" with length of \"" + std::to_string(mmaped_len) + "\"");
            }
        };
        const scoped_mmaping scoped_mmap(data_ptr, static_cast<size_t>(file_stat.st_size));


        auto result = std::vector<std::byte>{};
        result.assign(
            static_cast<const std::byte*>(data_ptr),
            static_cast<const std::byte*>(data_ptr) + file_stat.st_size);

        return result;
    }

    auto read_binary_file(const std::string& file_path) {
      auto expected_data = read_file(file_path);
      if (not expected_data.has_value()) {
        throw expected_data.error();
      }
      return std::move(expected_data).value();
    }

#else  // if defined MMAP_AVAILABLE and __cplusplus > 201703L else

  auto read_binary_file(const std::string& file_path) {
    auto file_content = std::string{};
    {
      std::stringbuf input_buffer;
      {
        std::ifstream input_file;

      #if __cplusplus > 201703L
        input_file.open(file_path, std::ios::binary | std::ios::ate);
        if (not input_file.is_open()) {
          const auto error_code = std::make_error_code(std::errc{errno});
          throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
        }
        input_file.exceptions(input_file.exceptions() | std::ios::badbit | std::ios::failbit);
        auto const file_size = input_file.tellg();
        input_file.seekg(0, std::ios::beg);
        try {
          std::string buffer;
          buffer.reserve(file_size);
          input_buffer.str(std::move(buffer));
        } catch (std::bad_alloc) {
          throw std::runtime_error("cannot allocate " + std::to_string(file_size) + " bytes of memory to read file \"" + file_path + "\"");
        }
      #else
        input_file.open(file_path, std::ios::binary);
        if (not input_file.is_open()) {
          const auto error_code = std::make_error_code(std::errc{errno});
          throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
        }
        input_file.exceptions(input_file.exceptions() | std::ios::badbit | std::ios::failbit);
      #endif

        input_file >> &input_buffer;
      }

      file_content = std::move(input_buffer).str();
    }

    auto result = std::vector<std::byte>{};
    result.assign(
        static_cast<const std::byte*>(static_cast<const void*>(&file_content[0])),
        static_cast<const std::byte*>(static_cast<const void*>(&file_content[file_content.size()])));
    return result;
  }


#endif  // if defined MMAP_AVAILABLE else
#undef MMAP_AVAILABLE

#include <cstdio>

auto get_file_size(std::FILE* const file_stream, const char* optional_file_path = nullptr) {
  const auto cur_offset = std::ftell(file_stream);
  if (-1L == cur_offset) {
    const auto error = std::make_error_code(std::errc{errno});
    auto file_path = (optional_file_path != nullptr ? " \"" + std::string{optional_file_path} + "\"" : std::string{});
    throw std::system_error(error, "Cannot get current file position of file" + std::move(file_path));
  }
  if (0 != fseek(file_stream, 0, SEEK_END)) {
    const auto error = std::make_error_code(std::errc{errno});
    auto file_path = (optional_file_path != nullptr ? " \"" + std::string{optional_file_path} + "\"" : std::string{});
    throw std::system_error(error, "Cannot seek to the end of file" + std::move(file_path));
  }
  const auto result = std::ftell(file_stream);
  if (-1L == result) {
    const auto error = std::make_error_code(std::errc{errno});
    auto file_path = (optional_file_path != nullptr ? " \"" + std::string{optional_file_path} + "\"" : std::string{});
    throw std::system_error(error, "Cannot get end file position of file" + std::move(file_path));
  }
  if (0 != fseek(file_stream, cur_offset, SEEK_SET)) {
    const auto error = std::make_error_code(std::errc{errno});
    auto file_path = (optional_file_path != nullptr ? " \"" + std::string{optional_file_path} + "\"" : std::string{});
    throw std::system_error(error, "Cannot seek to original file position of file" + std::move(file_path));
  }
  return result;
}

auto read_binary_file_c(const std::string& file_path) {
  std::FILE* const file_stream = std::fopen(file_path.c_str(), "rb");
  if (file_stream == nullptr) {
    const auto error = std::make_error_code(std::errc{errno});
    throw std::system_error(error, "Cannot open file \"" + file_path + "\"");
  }

  struct scoped_file_handler_t {
    std::FILE* const file_stream = nullptr;
    const std::string& file_path;
    ~scoped_file_handler_t() noexcept(false) {
      if (file_stream == nullptr) {
        return;
      }
      while (-1 == std::fclose(file_stream)) {
        const auto error = std::make_error_code(std::errc{errno});
        if (error != std::make_error_code(std::errc::interrupted)) {
          throw std::system_error(error, "Cannot close file \"" + file_path + "\"");
        }
      }
    }
  };
  const scoped_file_handler_t scoped_file_handler{file_stream, file_path};

  std::setbuf(file_stream, nullptr);

  const auto file_size = get_file_size(file_stream, file_path.c_str());

  constexpr auto resize_step = 8*1024;
  auto result = std::vector<std::byte>{};
  try {
    result.reserve(file_size + resize_step);
  } catch (std::bad_alloc) {
    throw std::runtime_error("cannot allocate " + std::to_string(file_size) + " bytes of memory to read file \"" + file_path + "\"");
  }

  while (true) {
    result.resize(result.size() + resize_step);
    std::byte* const put_ptr = &result[result.size() - resize_step];
    const auto read_bytes = std::fread(put_ptr, 1, resize_step, file_stream);
    if (read_bytes != resize_step) {
      result.resize(result.size() + read_bytes - resize_step);
    }
    if (std::ferror(file_stream)) {
      const auto error = std::make_error_code(std::errc{errno});
      throw std::system_error(error, "Error after reading " + std::to_string(result.size()) + " bytes of file \"" + file_path + "\"");
    } else if (std::feof(file_stream)) {
      break;
    }
  }

  return result;
}



auto read_binary_file22(const std::string& file_path) {
  std::ifstream input_file;
  input_file.open(file_path, std::ios::binary | std::ios::ate);
  if (not input_file.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  input_file.exceptions(input_file.exceptions() | std::ios::badbit | std::ios::failbit);
  auto const file_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);
  auto result = std::vector<std::byte>{};
  try {
    result.reserve(file_size + resize_step);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(file_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = input_file.readsome(to_char_ptr(result.end() - resize_step), resize_step);
    result.resize(result.size() + read_bytes - resize_step);
    if (read_bytes == 0) {
      break;
    }
  }

  return result;
}


auto read_binary_file23(const std::string& file_path) {
  std::ifstream input_file;
  input_file.open(file_path, std::ios::binary);
  if (not input_file.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  input_file.exceptions(input_file.exceptions() | std::ios::badbit | std::ios::failbit);
  auto const file_size = input_file.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size == -1) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  input_file.seekg(0, std::ios::beg);

  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);
  auto result = std::vector<std::byte>{};
  try {
    result.reserve(file_size + resize_step);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(file_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = input_file.rdbuf()->sgetn(to_char_ptr(result.end() - resize_step), resize_step);
    result.resize(result.size() + read_bytes - resize_step);
    if (read_bytes == 0) {
      break;
    }
  }

  return result;
}


auto read_binary_file24(const std::string& file_path) {
  std::filebuf file_buf;
  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);
  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size + resize_step);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - resize_step, resize_step);
    if (read_bytes != resize_step) {
      result.resize(result.size() + read_bytes - resize_step);
      break;
    }
  }

  return result;
}

#include <array>

auto read_binary_file25(const std::string& file_path) {
  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);

  std::filebuf file_buf;
  std::array<char, resize_step> raw_buffer;
  file_buf.pubsetbuf(raw_buffer.data(), raw_buffer.size());

  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size + resize_step);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - resize_step, resize_step);
    if (read_bytes != resize_step) {
      result.resize(result.size() + read_bytes - resize_step);
      break;
    }
  }

  return result;
}

auto read_binary_file251(const std::string& file_path) {
  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);

  std::filebuf file_buf;
  std::array<char, resize_step> raw_buffer;
  file_buf.pubsetbuf(raw_buffer.data(), raw_buffer.size());

  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size + resize_step);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  for (size_t i = 1; true; ++i) {
    result.resize(result.size() + i);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - i, i);
    if (read_bytes != i) {
      result.resize(result.size() + read_bytes - i);
      break;
    }
  }

  return result;
}

auto read_binary_file26(const std::string& file_path) {
  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);

  std::filebuf file_buf;
  file_buf.pubsetbuf(nullptr, 0);

  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size + resize_step);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - resize_step, resize_step);
    if (read_bytes != resize_step) {
      result.resize(result.size() + read_bytes - resize_step);
      break;
    }
  }

  return result;
}


auto read_binary_file261(const std::string& file_path) {

  std::filebuf file_buf;
  file_buf.pubsetbuf(nullptr, 0);

  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  for (size_t i = 2; true; ++i) {
    result.resize(result.size() + i);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - i, i);
    if (read_bytes != i) {
      result.resize(result.size() + read_bytes - i);
      break;
    }
  }

  return result;
}

auto read_binary_file262(const std::string& file_path) {
  char raw_buffer;

  std::filebuf file_buf;
  file_buf.pubsetbuf(&raw_buffer, 1);  // If `nullptr` provided then `filebuf` will allocate 1 byte in heap.

  file_buf.open(file_path, std::ios::binary | std::ios::in);
  if (not file_buf.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  auto const file_size = file_buf.pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size < 0) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  file_buf.pubseekoff(0, std::ios::beg, std::ios::in);

  constexpr auto resize_step = ptrdiff_t{BUFSIZ};
  static_assert(resize_step >= 1024 and resize_step <= 64*1024);

  auto result = std::vector<std::byte>{};
  const auto buffer_size = static_cast<size_t>(file_size + resize_step);
  try {
    result.reserve(buffer_size);
  } catch (const std::exception& ex) {
    throw std::runtime_error("cannot allocate " + std::to_string(buffer_size + resize_step) + " bytes of memory to read file \"" + file_path + "\": exception message \"" + ex.what() + "\"");
  }

  const auto to_char_ptr = [](auto iter) { return static_cast<char*>(static_cast<void*>(iter.base())); };

  while (true) {
    result.resize(result.size() + resize_step);
    const auto read_bytes = file_buf.sgetn(to_char_ptr(result.end()) - resize_step, resize_step);
    if (read_bytes != resize_step) {
      result.resize(result.size() + read_bytes - resize_step);
      break;
    }
  }

  return result;
}


auto read_binary_file3(const std::string& file_path) {

  struct rangeable_filebuf : public std::filebuf {
    using base_type = std::filebuf;
  public:
    using base_type::operator=;

  public:
    auto get_readable_range() -> std::pair<const std::byte*, const std::byte*> {
      auto result = std::pair<const std::byte*, const std::byte*>{};
      if (gptr() == egptr()) {
        fetch_next_range();
      }
      result = std::make_pair(
          static_cast<const std::byte*>(static_cast<const void*>(gptr())),
          static_cast<const std::byte*>(static_cast<const void*>(egptr()))
        );
      return result;
    }

    void fetch_next_range() {
      gbump(egptr() - gptr());
      underflow();
    }

  };

  std::ifstream input_file;
  input_file.open(file_path, std::ios::binary);
  if (not input_file.is_open()) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot open file \"" + file_path + "\"");
  }
  input_file.exceptions(input_file.exceptions() | std::ios::badbit | std::ios::failbit);
  auto const file_size = input_file.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
  if (file_size == -1) {
    const auto error_code = std::make_error_code(std::errc{errno});
    throw std::system_error(error_code, "cannot get size of the file \"" + file_path + "\"");
  }
  input_file.seekg(0, std::ios::beg);

  auto result = std::vector<std::byte>{};
  try {
    result.reserve(file_size);
  } catch (std::bad_alloc) {
    throw std::runtime_error("cannot allocate " + std::to_string(file_size) + " bytes of memory to read file \"" + file_path + "\"");
  }

  auto original_rdbuf_ptr = input_file.rdbuf();
  rangeable_filebuf filebuf;
  filebuf = std::move(*original_rdbuf_ptr);

  auto range = filebuf.get_readable_range();
  while (range.first != range.second) {
    std::cout << "readable_range size: " << (range.second - range.first) << std::endl;
    result.insert(result.end(), range.first, range.second);
    filebuf.fetch_next_range();
    range = filebuf.get_readable_range();
  }

  return result;
}


#include <array>
#include <algorithm>
#include <random>


#include <chrono>

int main(int ac, char*av[])
{

  if (false)
  {
    {
      const auto file_size = size_t{150*1024*1024+123456};
      std::minstd_rand rnd(1);
      std::ofstream out_file("/tmp/data150.bin", std::ios::binary);
      std::array<char, 8*1000> buff;
      size_t n = 0;
      for (size_t i = 0; i < file_size; i += buff.size()) {
        std::generate(buff.begin(), buff.end(), [&rnd]() mutable { return static_cast<char>(rnd()); } );
        out_file.write(buff.begin(), std::min<ssize_t>(buff.size(), file_size - i));
      }
    }
    {
      const auto file_size = size_t{15*1024*1024+12345};
      std::minstd_rand rnd(1);
      std::ofstream out_file("/tmp/data15.bin", std::ios::binary);
      std::array<char, 8*1000> buff;
      size_t n = 0;
      for (size_t i = 0; i < file_size; i += buff.size()) {
        std::generate(buff.begin(), buff.end(), [&rnd]() mutable { return static_cast<char>(rnd()); } );
        out_file.write(buff.begin(), std::min<ssize_t>(buff.size(), file_size - i));
      }
    }
    {
      const auto file_size = size_t{300*1024+456};
      std::minstd_rand rnd(1);
      std::ofstream out_file("/tmp/data03.bin", std::ios::binary);
      std::array<char, 8*1000> buff;
      size_t n = 0;
      for (size_t i = 0; i < file_size; i += buff.size()) {
        std::generate(buff.begin(), buff.end(), [&rnd]() mutable { return static_cast<char>(rnd()); } );
        out_file.write(buff.begin(), std::min<ssize_t>(buff.size(), file_size - i));
      }
    }
  }


  {
  if (ac <= 3) {
      std::cout << "algo, path or size not selected\n";
      return 1;
    }

    const auto algo = std::string(av[1]);
    const auto file_path = std::string(av[2]);
    const auto file_size = std::string(av[3]);
    auto data = std::vector<std::byte>{};

    const auto start = std::chrono::steady_clock::now();
    if (algo == "i") {
      std::cout << "read_binary_file_iterators," << file_path;
      data = read_binary_file_iterators(file_path);
    } else if (algo == "p") {
      std::cout << "read_binary_file_iterators_prealloc," << file_path;
      data = read_binary_file_iterators_prealloc(file_path);
    } else if (algo == "f") {
      std::cout << "read_binary_file_fast," << file_path;
      data = read_binary_file_fast(file_path);
    } else if (algo == "s") {
      std::cout << "read_binary_file_simple," << file_path;
      data = read_binary_file_simple(file_path);
    } else if (algo == "c") {
      std::cout << "read_binary_file_c," << file_path;
      data = read_binary_file_c(file_path);
  #if __cplusplus > 201703L
    } else if (algo == "m") {
      std::cout << "memory map," << file_path;
      data = read_binary_file(file_path);
  #endif
  #if defined __cpp_lib_expected
    } else if (algo == "ff") {
      std::cout << "read_binary_file_fastfast," << file_path;
      auto res = read_binary_file_fastfast(file_path);
      if (not res.has_value()) {
        std::cout << ",(error: " << res.error().what() << ")" << std::endl;
        return 1;
      }
      data = std::move(res).value();
  #endif
    } else {
      std::cout << "unkonwn command: " << av[1] << "\n";
      return 1;
    }
  const auto end = std::chrono::steady_clock::now();
  if (std::to_string(data.size()) != file_size) {
    std::cout << ",INVALID=" << data.size() << std::endl;
  } else {
    const auto micr = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "," << micr.count() << std::endl;
  }


    // std::minstd_rand rnd(1);
    // std::cout << "read " << data.size() << " bytes\n";
    // std::cout << "first, last: " << static_cast<unsigned>(data.front()) << ", " << static_cast<unsigned>(data.back()) << "\n";
    // for (size_t i = 0; i < file_size; ++i) {
    //   if (data[i] != static_cast<std::byte>(static_cast<char>(rnd()))) {
    //     std::cout << "bad data at position " << i << "\n";
    //     return 1;
    //   }
    // }
  }
  return 0;
}
