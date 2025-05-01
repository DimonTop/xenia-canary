/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Xenia Canary. All rights reserved.                          *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_XAM_UNMARSHALLER_UNMARSHALLER_H_
#define XENIA_KERNEL_XAM_UNMARSHALLER_UNMARSHALLER_H_

#include "xenia/kernel/util/shim_utils.h"

namespace xe {
namespace kernel {
namespace xam {

class Unmarshaller {
 public:
  std::span<uint8_t> Advance(size_t count);

  template <typename T>
  std::span<uint8_t> AdvanceSizeOf() {
    return Advance(sizeof(T));
  };

  template <typename T>
  T Read() {
    std::span<uint8_t> data = AdvanceSizeOf<T>();

    if (data.empty()) {
      return {};
    }

    return *reinterpret_cast<T*>(data.data());
  };

  template <typename T>
  T ReadSwap() {
    return xe::byte_swap<T>(Read<T>());
  };

  std::u16string ReadSwapUTF16String(uint32_t length);

  std::string ReadString(uint32_t length);

  virtual X_HRESULT Deserialize();

  template <typename T>
  T* DeserializeReinterpret() {
    if (!xlive_async_task_ptr_ ||
        !xlive_async_task_ptr_->marshalled_request_ptr) {
      return nullptr;
    }

    assert_false(sizeof(T) != xlive_async_task_ptr_->marshalled_request_size);

    return kernel_state()->memory()->TranslateVirtual<T*>(
        xlive_async_task_ptr_->marshalled_request_ptr);
  };

  template <typename T>
  T* Results() const {
    if (!xlive_async_task_ptr_ || !xlive_async_task_ptr_->results_ptr) {
      return nullptr;
    }

    return kernel_state()->memory()->TranslateVirtual<T*>(
        xlive_async_task_ptr_->results_ptr);
  };

  bool ZeroResults() const {
    if (!xlive_async_task_ptr_ || !xlive_async_task_ptr_->results_ptr) {
      return false;
    }

    uint8_t* results_ptr = kernel_state()->memory()->TranslateVirtual<uint8_t*>(
        xlive_async_task_ptr_->results_ptr);

    std::fill_n(results_ptr, xlive_async_task_ptr_->results_size.get(), 0);

    return true;
  };

  XLIVEBASE_ASYNC_MESSAGE* GetXLiveBaseAsyncMessage();

  XLIVE_ASYNC_TASK* GetXLiveAsyncTask();

  SCHEMA_DATA* GetSchemaData();

  size_t GetPosition() const;

  ~Unmarshaller() {};

 protected:
  Unmarshaller(uint32_t marshaller_buffer);

  XLIVEBASE_ASYNC_MESSAGE* xlivebase_async_message_ptr_;
  XLIVE_ASYNC_TASK* xlive_async_task_ptr_;
  SCHEMA_DATA* schema_data_ptr_;
  ORDINAL_TO_INDEX* ordinal_to_index_ptr_;
  SCHEMA_TABLE_ENTRY* schema_table_entry_ptr_;

  std::span<uint8_t> data_ptr_;

 private:
  size_t position_;
};

}  // namespace xam
}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_XAM_UNMARSHALLER_UNMARSHALLER_H_