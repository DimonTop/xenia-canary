/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Xenia Canary. All rights reserved.                          *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/xam/unmarshaller/unmarshaller.h"

namespace xe {
namespace kernel {
namespace xam {

Unmarshaller::Unmarshaller(uint32_t marshaller_address)
    : xlivebase_async_message_ptr_(nullptr),
      xlive_async_task_ptr_(nullptr),
      schema_data_ptr_(nullptr),
      ordinal_to_index_ptr_(nullptr),
      schema_table_entry_ptr_(nullptr),
      data_ptr_({}),
      position_(0) {
  if (!marshaller_address) {
    return;
  }

  xlivebase_async_message_ptr_ =
      kernel_state()->memory()->TranslateVirtual<XLIVEBASE_ASYNC_MESSAGE*>(
          marshaller_address);

  xlive_async_task_ptr_ =
      kernel_state()->memory()->TranslateVirtual<XLIVE_ASYNC_TASK*>(
          xlivebase_async_message_ptr_->xlive_async_task_ptr);

  schema_data_ptr_ = kernel_state()->memory()->TranslateVirtual<SCHEMA_DATA*>(
      xlive_async_task_ptr_->schema_data_ptr);

  ordinal_to_index_ptr_ =
      kernel_state()->memory()->TranslateVirtual<ORDINAL_TO_INDEX*>(
          schema_data_ptr_->OrdinalToIndexPtr);

  schema_table_entry_ptr_ =
      kernel_state()->memory()->TranslateVirtual<SCHEMA_TABLE_ENTRY*>(
          schema_data_ptr_->TableEntriesPtr);

  uint8_t* data_request_ptr =
      kernel_state()->memory()->TranslateVirtual<uint8_t*>(
          xlive_async_task_ptr_->marshalled_request_ptr);

  data_ptr_ = std::span<uint8_t>(
      data_request_ptr, xlive_async_task_ptr_->marshalled_request_size);

  XELOGD(
      "\n***************** Unmarshaller Info *****************\n"
      "SchemaVersionMajor: {}\n"
      "SchemaVersionMinor: {}\n"
      "ToolVersion: {:08X}\n"
      "SechmaIndex: {:08X}\n"
      "TaskFlags: {}\n"
      "MarshalledRequestPtr: {:08X}\n"
      "MarshalledRequestSize: {}\n"
      "ResultsPtr: {:08X}\n"
      "RequestsSize: {}\n",
      schema_data_ptr_->Header.SchemaVersionMajor.get(),
      schema_data_ptr_->Header.SchemaVersionMinor.get(),
      schema_data_ptr_->Header.ToolVersion.get(),
      xlive_async_task_ptr_->schema_index.get(),
      xlive_async_task_ptr_->task_flags.get(),
      xlive_async_task_ptr_->marshalled_request_ptr.get(),
      xlive_async_task_ptr_->marshalled_request_size.get(),
      xlive_async_task_ptr_->results_ptr.get(),
      xlive_async_task_ptr_->results_size.get());
}

std::span<uint8_t> Unmarshaller::Advance(size_t count) {
  const size_t offset = position_ + count;

  if (offset > data_ptr_.size()) {
    assert_always(std::format("{}: Out of Bounds Span!", __func__));

    return std::span<uint8_t>();
  }

  std::span<uint8_t> data = data_ptr_.subspan(position_, count);

  position_ = offset;

  return data;
}

std::u16string Unmarshaller::ReadSwapUTF16String(uint32_t length) {
  // Excludes null terminator
  std::u16string server_path = xe::load_and_swap<std::u16string>(
      reinterpret_cast<char16_t*>(data_ptr_.data() + position_));

  std::span<uint8_t> string_data =
      Advance(xe::string_util::size_in_bytes(server_path, true));

  assert_false(length != server_path.length() + 1);

  if (string_data.empty()) {
    return u"";
  }

  return server_path;
}

std::string Unmarshaller::ReadString(uint32_t length) {
  std::span<uint8_t> string_data = Advance(length);

  if (string_data.empty()) {
    return "";
  }

  return std::string(reinterpret_cast<char*>(string_data.data()), length);
}

X_HRESULT Unmarshaller::Deserialize() { return X_E_FAIL; }

XLIVEBASE_ASYNC_MESSAGE* Unmarshaller::GetXLiveBaseAsyncMessage() {
  return xlivebase_async_message_ptr_;
}

XLIVE_ASYNC_TASK* Unmarshaller::GetXLiveAsyncTask() {
  return xlive_async_task_ptr_;
}

SCHEMA_DATA* Unmarshaller::GetSchemaData() { return schema_data_ptr_; }

size_t Unmarshaller::GetPosition() const { return position_; }

}  // namespace xam
}  // namespace kernel
}  // namespace xe
