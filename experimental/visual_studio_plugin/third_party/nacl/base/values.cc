// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/values.h"

#include "base/logging.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"

namespace {

// Make a deep copy of |node|, but don't include empty lists or dictionaries
// in the copy. It's possible for this function to return NULL and it
// expects |node| to always be non-NULL.
Value* CopyWithoutEmptyChildren(Value* node) {
  DCHECK(node);
  switch (node->GetType()) {
    case Value::TYPE_LIST: {
      ListValue* list = static_cast<ListValue*>(node);
      ListValue* copy = new ListValue;
      for (ListValue::const_iterator it = list->begin(); it != list->end();
           ++it) {
        Value* child_copy = CopyWithoutEmptyChildren(*it);
        if (child_copy)
          copy->Append(child_copy);
      }
      if (!copy->empty())
        return copy;

      delete copy;
      return NULL;
    }

    case Value::TYPE_DICTIONARY: {
      DictionaryValue* dict = static_cast<DictionaryValue*>(node);
      DictionaryValue* copy = new DictionaryValue;
      for (DictionaryValue::key_iterator it = dict->begin_keys();
           it != dict->end_keys(); ++it) {
        Value* child = NULL;
        bool rv = dict->GetWithoutPathExpansion(*it, &child);
        DCHECK(rv);
        Value* child_copy = CopyWithoutEmptyChildren(child);
        if (child_copy)
          copy->SetWithoutPathExpansion(*it, child_copy);
      }
      if (!copy->empty())
        return copy;

      delete copy;
      return NULL;
    }

    default:
      // For everything else, just make a copy.
      return node->DeepCopy();
  }
}

}  // namespace

///////////////////// Value ////////////////////

Value::~Value() {
}

// static
Value* Value::CreateNullValue() {
  return new Value(TYPE_NULL);
}

// static
Value* Value::CreateBooleanValue(bool in_value) {
  return new FundamentalValue(in_value);
}

// static
Value* Value::CreateIntegerValue(int in_value) {
  return new FundamentalValue(in_value);
}

// static
Value* Value::CreateRealValue(double in_value) {
  return new FundamentalValue(in_value);
}

// static
Value* Value::CreateStringValue(const std::string& in_value) {
  return new StringValue(in_value);
}

// static
Value* Value::CreateStringValue(const std::wstring& in_value) {
  return new StringValue(in_value);
}

// static
Value* Value::CreateStringValueFromUTF16(const string16& in_value) {
  return new StringValue(in_value);
}

// static
BinaryValue* Value::CreateBinaryValue(char* buffer, size_t size) {
  return BinaryValue::Create(buffer, size);
}

bool Value::GetAsBoolean(bool* in_value) const {
  return false;
}

bool Value::GetAsInteger(int* in_value) const {
  return false;
}

bool Value::GetAsReal(double* in_value) const {
  return false;
}

bool Value::GetAsString(std::string* in_value) const {
  return false;
}

bool Value::GetAsString(std::wstring* in_value) const {
  return false;
}

bool Value::GetAsUTF16(string16* out_value) const {
  return false;
}

Value* Value::DeepCopy() const {
  // This method should only be getting called for null Values--all subclasses
  // need to provide their own implementation;.
  DCHECK(IsType(TYPE_NULL));
  return CreateNullValue();
}

bool Value::Equals(const Value* other) const {
  // This method should only be getting called for null Values--all subclasses
  // need to provide their own implementation;.
  DCHECK(IsType(TYPE_NULL));
  return other->IsType(TYPE_NULL);
}

///////////////////// FundamentalValue ////////////////////

FundamentalValue::~FundamentalValue() {
}

bool FundamentalValue::GetAsBoolean(bool* out_value) const {
  if (out_value && IsType(TYPE_BOOLEAN))
    *out_value = boolean_value_;
  return (IsType(TYPE_BOOLEAN));
}

bool FundamentalValue::GetAsInteger(int* out_value) const {
  if (out_value && IsType(TYPE_INTEGER))
    *out_value = integer_value_;
  return (IsType(TYPE_INTEGER));
}

bool FundamentalValue::GetAsReal(double* out_value) const {
  if (out_value && IsType(TYPE_REAL))
    *out_value = real_value_;
  return (IsType(TYPE_REAL));
}

Value* FundamentalValue::DeepCopy() const {
  switch (GetType()) {
    case TYPE_BOOLEAN:
      return CreateBooleanValue(boolean_value_);

    case TYPE_INTEGER:
      return CreateIntegerValue(integer_value_);

    case TYPE_REAL:
      return CreateRealValue(real_value_);

    default:
      NOTREACHED();
      return NULL;
  }
}

bool FundamentalValue::Equals(const Value* other) const {
  if (other->GetType() != GetType())
    return false;

  switch (GetType()) {
    case TYPE_BOOLEAN: {
      bool lhs, rhs;
      return GetAsBoolean(&lhs) && other->GetAsBoolean(&rhs) && lhs == rhs;
    }
    case TYPE_INTEGER: {
      int lhs, rhs;
      return GetAsInteger(&lhs) && other->GetAsInteger(&rhs) && lhs == rhs;
    }
    case TYPE_REAL: {
      double lhs, rhs;
      return GetAsReal(&lhs) && other->GetAsReal(&rhs) && lhs == rhs;
    }
    default:
      NOTREACHED();
      return false;
  }
}

///////////////////// StringValue ////////////////////

StringValue::StringValue(const std::string& in_value)
    : Value(TYPE_STRING),
      value_(in_value) {
  DCHECK(IsStringUTF8(in_value));
}

StringValue::StringValue(const std::wstring& in_value)
    : Value(TYPE_STRING),
      value_(WideToUTF8(in_value)) {
}

#if !defined(WCHAR_T_IS_UTF16)
StringValue::StringValue(const string16& in_value)
    : Value(TYPE_STRING),
      value_(UTF16ToUTF8(in_value)) {
}
#endif

StringValue::~StringValue() {
}

bool StringValue::GetAsString(std::string* out_value) const {
  if (out_value)
    *out_value = value_;
  return true;
}

bool StringValue::GetAsString(std::wstring* out_value) const {
  if (out_value)
    *out_value = UTF8ToWide(value_);
  return true;
}

bool StringValue::GetAsUTF16(string16* out_value) const {
  if (out_value)
    *out_value = UTF8ToUTF16(value_);
  return true;
}

Value* StringValue::DeepCopy() const {
  return CreateStringValue(value_);
}

bool StringValue::Equals(const Value* other) const {
  if (other->GetType() != GetType())
    return false;
  std::string lhs, rhs;
  return GetAsString(&lhs) && other->GetAsString(&rhs) && lhs == rhs;
}

///////////////////// BinaryValue ////////////////////

// static
BinaryValue* BinaryValue::Create(char* buffer, size_t size) {
  if (!buffer)
    return NULL;

  return new BinaryValue(buffer, size);
}

// static
BinaryValue* BinaryValue::CreateWithCopiedBuffer(const char* buffer,
                                                 size_t size) {
  if (!buffer)
    return NULL;

  char* buffer_copy = new char[size];
  memcpy(buffer_copy, buffer, size);
  return new BinaryValue(buffer_copy, size);
}


BinaryValue::BinaryValue(char* buffer, size_t size)
  : Value(TYPE_BINARY),
    buffer_(buffer),
    size_(size) {
  DCHECK(buffer_);
}

BinaryValue::~BinaryValue() {
  DCHECK(buffer_);
  if (buffer_)
    delete[] buffer_;
}

Value* BinaryValue::DeepCopy() const {
  return CreateWithCopiedBuffer(buffer_, size_);
}

bool BinaryValue::Equals(const Value* other) const {
  if (other->GetType() != GetType())
    return false;
  const BinaryValue* other_binary = static_cast<const BinaryValue*>(other);
  if (other_binary->size_ != size_)
    return false;
  return !memcmp(buffer_, other_binary->buffer_, size_);
}

///////////////////// DictionaryValue ////////////////////

DictionaryValue::~DictionaryValue() {
  Clear();
}

Value* DictionaryValue::DeepCopy() const {
  DictionaryValue* result = new DictionaryValue;

  for (ValueMap::const_iterator current_entry(dictionary_.begin());
       current_entry != dictionary_.end(); ++current_entry) {
    result->SetWithoutPathExpansion(current_entry->first,
                                    current_entry->second->DeepCopy());
  }

  return result;
}

bool DictionaryValue::Equals(const Value* other) const {
  if (other->GetType() != GetType())
    return false;

  const DictionaryValue* other_dict =
      static_cast<const DictionaryValue*>(other);
  key_iterator lhs_it(begin_keys());
  key_iterator rhs_it(other_dict->begin_keys());
  while (lhs_it != end_keys() && rhs_it != other_dict->end_keys()) {
    Value* lhs;
    Value* rhs;
    if (!GetWithoutPathExpansion(*lhs_it, &lhs) ||
        !other_dict->GetWithoutPathExpansion(*rhs_it, &rhs) ||
        !lhs->Equals(rhs)) {
      return false;
    }
    ++lhs_it;
    ++rhs_it;
  }
  if (lhs_it != end_keys() || rhs_it != other_dict->end_keys())
    return false;

  return true;
}

bool DictionaryValue::HasKey(const std::wstring& key) const {
  ValueMap::const_iterator current_entry = dictionary_.find(key);
  DCHECK((current_entry == dictionary_.end()) || current_entry->second);
  return current_entry != dictionary_.end();
}

bool DictionaryValue::HasKeyASCII(const std::string& key) const {
  return HasKey(ASCIIToWide(key));
}

void DictionaryValue::Clear() {
  ValueMap::iterator dict_iterator = dictionary_.begin();
  while (dict_iterator != dictionary_.end()) {
    delete dict_iterator->second;
    ++dict_iterator;
  }

  dictionary_.clear();
}

void DictionaryValue::Set(const std::wstring& path, Value* in_value) {
  DCHECK(in_value);

  std::wstring current_path(path);
  DictionaryValue* current_dictionary = this;
  for (size_t delimiter_position = current_path.find('.');
       delimiter_position != std::wstring::npos;
       delimiter_position = current_path.find('.')) {
    // Assume that we're indexing into a dictionary.
    std::wstring key(current_path, 0, delimiter_position);
    DictionaryValue* child_dictionary = NULL;
    if (!current_dictionary->GetDictionary(key, &child_dictionary)) {
      child_dictionary = new DictionaryValue;
      current_dictionary->SetWithoutPathExpansion(key, child_dictionary);
    }

    current_dictionary = child_dictionary;
    current_path.erase(0, delimiter_position + 1);
  }

  current_dictionary->SetWithoutPathExpansion(current_path, in_value);
}

void DictionaryValue::SetBoolean(const std::wstring& path, bool in_value) {
  Set(path, CreateBooleanValue(in_value));
}

void DictionaryValue::SetInteger(const std::wstring& path, int in_value) {
  Set(path, CreateIntegerValue(in_value));
}

void DictionaryValue::SetReal(const std::wstring& path, double in_value) {
  Set(path, CreateRealValue(in_value));
}

void DictionaryValue::SetString(const std::wstring& path,
                                const std::string& in_value) {
  Set(path, CreateStringValue(in_value));
}

void DictionaryValue::SetString(const std::wstring& path,
                                const std::wstring& in_value) {
  Set(path, CreateStringValue(in_value));
}

void DictionaryValue::SetStringFromUTF16(const std::wstring& path,
                                         const string16& in_value) {
  Set(path, CreateStringValueFromUTF16(in_value));
}

void DictionaryValue::SetWithoutPathExpansion(const std::wstring& key,
                                              Value* in_value) {
  // If there's an existing value here, we need to delete it, because
  // we own all our children.
  if (HasKey(key)) {
    DCHECK(dictionary_[key] != in_value);  // This would be bogus
    delete dictionary_[key];
  }

  dictionary_[key] = in_value;
}

bool DictionaryValue::Get(const std::wstring& path, Value** out_value) const {
  std::wstring current_path(path);
  const DictionaryValue* current_dictionary = this;
  for (size_t delimiter_position = current_path.find('.');
       delimiter_position != std::wstring::npos;
       delimiter_position = current_path.find('.')) {
    DictionaryValue* child_dictionary = NULL;
    if (!current_dictionary->GetDictionary(
            current_path.substr(0, delimiter_position), &child_dictionary))
      return false;

    current_dictionary = child_dictionary;
    current_path.erase(0, delimiter_position + 1);
  }

  return current_dictionary->GetWithoutPathExpansion(current_path, out_value);
}

bool DictionaryValue::GetBoolean(const std::wstring& path,
                                 bool* bool_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsBoolean(bool_value);
}

bool DictionaryValue::GetInteger(const std::wstring& path,
                                 int* out_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsInteger(out_value);
}

bool DictionaryValue::GetReal(const std::wstring& path,
                              double* out_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsReal(out_value);
}

bool DictionaryValue::GetString(const std::string& path,
                                string16* out_value) const {
  return GetStringAsUTF16(ASCIIToWide(path), out_value);
}

bool DictionaryValue::GetStringASCII(const std::string& path,
                                     std::string* out_value) const {
  std::string out;
  if (!GetString(ASCIIToWide(path), &out))
    return false;

  if (!IsStringASCII(out)) {
    NOTREACHED();
    return false;
  }

  out_value->assign(out);
  return true;
}

bool DictionaryValue::GetString(const std::wstring& path,
                                std::string* out_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsString(out_value);
}

bool DictionaryValue::GetString(const std::wstring& path,
                                std::wstring* out_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsString(out_value);
}

bool DictionaryValue::GetStringAsUTF16(const std::wstring& path,
                                       string16* out_value) const {
  Value* value;
  if (!Get(path, &value))
    return false;

  return value->GetAsUTF16(out_value);
}

bool DictionaryValue::GetBinary(const std::wstring& path,
                                BinaryValue** out_value) const {
  Value* value;
  bool result = Get(path, &value);
  if (!result || !value->IsType(TYPE_BINARY))
    return false;

  if (out_value)
    *out_value = static_cast<BinaryValue*>(value);

  return true;
}

bool DictionaryValue::GetDictionary(const std::wstring& path,
                                    DictionaryValue** out_value) const {
  Value* value;
  bool result = Get(path, &value);
  if (!result || !value->IsType(TYPE_DICTIONARY))
    return false;

  if (out_value)
    *out_value = static_cast<DictionaryValue*>(value);

  return true;
}

bool DictionaryValue::GetList(const std::wstring& path,
                              ListValue** out_value) const {
  Value* value;
  bool result = Get(path, &value);
  if (!result || !value->IsType(TYPE_LIST))
    return false;

  if (out_value)
    *out_value = static_cast<ListValue*>(value);

  return true;
}

bool DictionaryValue::GetWithoutPathExpansion(const std::wstring& key,
                                              Value** out_value) const {
  ValueMap::const_iterator entry_iterator = dictionary_.find(key);
  if (entry_iterator == dictionary_.end())
    return false;

  Value* entry = entry_iterator->second;
  if (out_value)
    *out_value = entry;
  return true;
}

bool DictionaryValue::GetIntegerWithoutPathExpansion(const std::wstring& path,
                                                     int* out_value) const {
  Value* value;
  if (!GetWithoutPathExpansion(path, &value))
    return false;

  return value->GetAsInteger(out_value);
}

bool DictionaryValue::GetStringWithoutPathExpansion(
    const std::wstring& path,
    std::string* out_value) const {
  Value* value;
  if (!GetWithoutPathExpansion(path, &value))
    return false;

  return value->GetAsString(out_value);
}

bool DictionaryValue::GetStringWithoutPathExpansion(
    const std::wstring& path,
    std::wstring* out_value) const {
  Value* value;
  if (!GetWithoutPathExpansion(path, &value))
    return false;

  return value->GetAsString(out_value);
}

bool DictionaryValue::GetStringAsUTF16WithoutPathExpansion(
    const std::wstring& path,
    string16* out_value) const {
  Value* value;
  if (!GetWithoutPathExpansion(path, &value))
    return false;

  return value->GetAsUTF16(out_value);
}

bool DictionaryValue::GetDictionaryWithoutPathExpansion(
    const std::wstring& path,
    DictionaryValue** out_value) const {
  Value* value;
  bool result = GetWithoutPathExpansion(path, &value);
  if (!result || !value->IsType(TYPE_DICTIONARY))
    return false;

  if (out_value)
    *out_value = static_cast<DictionaryValue*>(value);

  return true;
}

bool DictionaryValue::GetListWithoutPathExpansion(const std::wstring& path,
                                                  ListValue** out_value) const {
  Value* value;
  bool result = GetWithoutPathExpansion(path, &value);
  if (!result || !value->IsType(TYPE_LIST))
    return false;

  if (out_value)
    *out_value = static_cast<ListValue*>(value);

  return true;
}

bool DictionaryValue::Remove(const std::wstring& path, Value** out_value) {
  std::wstring current_path(path);
  DictionaryValue* current_dictionary = this;
  size_t delimiter_position = current_path.rfind('.');
  if (delimiter_position != std::wstring::npos) {
    if (!GetDictionary(current_path.substr(0, delimiter_position),
                       &current_dictionary))
      return false;
    current_path.erase(0, delimiter_position + 1);
  }

  return current_dictionary->RemoveWithoutPathExpansion(current_path,
                                                        out_value);
}

bool DictionaryValue::RemoveWithoutPathExpansion(const std::wstring& key,
                                                 Value** out_value) {
  ValueMap::iterator entry_iterator = dictionary_.find(key);
  if (entry_iterator == dictionary_.end())
    return false;

  Value* entry = entry_iterator->second;
  if (out_value)
    *out_value = entry;
  else
    delete entry;
  dictionary_.erase(entry_iterator);
  return true;
}

DictionaryValue* DictionaryValue::DeepCopyWithoutEmptyChildren() {
  Value* copy = CopyWithoutEmptyChildren(this);
  return copy ? static_cast<DictionaryValue*>(copy) : new DictionaryValue;
}

///////////////////// ListValue ////////////////////

ListValue::~ListValue() {
  Clear();
}

void ListValue::Clear() {
  for (ValueVector::iterator i(list_.begin()); i != list_.end(); ++i)
    delete *i;
  list_.clear();
}

bool ListValue::Set(size_t index, Value* in_value) {
  if (!in_value)
    return false;

  if (index >= list_.size()) {
    // Pad out any intermediate indexes with null settings
    while (index > list_.size())
      Append(CreateNullValue());
    Append(in_value);
  } else {
    DCHECK(list_[index] != in_value);
    delete list_[index];
    list_[index] = in_value;
  }
  return true;
}

bool ListValue::Get(size_t index, Value** out_value) const {
  if (index >= list_.size())
    return false;

  if (out_value)
    *out_value = list_[index];

  return true;
}

bool ListValue::GetBoolean(size_t index, bool* bool_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsBoolean(bool_value);
}

bool ListValue::GetInteger(size_t index, int* out_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsInteger(out_value);
}

bool ListValue::GetReal(size_t index, double* out_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsReal(out_value);
}

bool ListValue::GetString(size_t index, std::string* out_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsString(out_value);
}

bool ListValue::GetString(size_t index, std::wstring* out_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsString(out_value);
}

bool ListValue::GetStringAsUTF16(size_t index, string16* out_value) const {
  Value* value;
  if (!Get(index, &value))
    return false;

  return value->GetAsUTF16(out_value);
}

bool ListValue::GetBinary(size_t index, BinaryValue** out_value) const {
  Value* value;
  bool result = Get(index, &value);
  if (!result || !value->IsType(TYPE_BINARY))
    return false;

  if (out_value)
    *out_value = static_cast<BinaryValue*>(value);

  return true;
}

bool ListValue::GetDictionary(size_t index, DictionaryValue** out_value) const {
  Value* value;
  bool result = Get(index, &value);
  if (!result || !value->IsType(TYPE_DICTIONARY))
    return false;

  if (out_value)
    *out_value = static_cast<DictionaryValue*>(value);

  return true;
}

bool ListValue::GetList(size_t index, ListValue** out_value) const {
  Value* value;
  bool result = Get(index, &value);
  if (!result || !value->IsType(TYPE_LIST))
    return false;

  if (out_value)
    *out_value = static_cast<ListValue*>(value);

  return true;
}

bool ListValue::Remove(size_t index, Value** out_value) {
  if (index >= list_.size())
    return false;

  if (out_value)
    *out_value = list_[index];
  else
    delete list_[index];

  list_.erase(list_.begin() + index);
  return true;
}

int ListValue::Remove(const Value& value) {
  for (ValueVector::iterator i(list_.begin()); i != list_.end(); ++i) {
    if ((*i)->Equals(&value)) {
      size_t index = i - list_.begin();
      delete *i;
      list_.erase(i);

      // TODO(anyone): Returning a signed int type here is just wrong.
      // Change this interface to return a size_t.
      DCHECK(index <= INT_MAX);
      int return_index = static_cast<int>(index);
      return return_index;
    }
  }
  return -1;
}

void ListValue::Append(Value* in_value) {
  DCHECK(in_value);
  list_.push_back(in_value);
}

bool ListValue::AppendIfNotPresent(Value* in_value) {
  DCHECK(in_value);
  for (ValueVector::iterator i(list_.begin()); i != list_.end(); ++i) {
    if ((*i)->Equals(in_value))
      return false;
  }
  list_.push_back(in_value);
  return true;
}

bool ListValue::Insert(size_t index, Value* in_value) {
  DCHECK(in_value);
  if (index > list_.size())
    return false;

  list_.insert(list_.begin() + index, in_value);
  return true;
}

Value* ListValue::DeepCopy() const {
  ListValue* result = new ListValue;

  for (ValueVector::const_iterator i(list_.begin()); i != list_.end(); ++i)
    result->Append((*i)->DeepCopy());

  return result;
}

bool ListValue::Equals(const Value* other) const {
  if (other->GetType() != GetType())
    return false;

  const ListValue* other_list =
      static_cast<const ListValue*>(other);
  const_iterator lhs_it, rhs_it;
  for (lhs_it = begin(), rhs_it = other_list->begin();
       lhs_it != end() && rhs_it != other_list->end();
       ++lhs_it, ++rhs_it) {
    if (!(*lhs_it)->Equals(*rhs_it))
      return false;
  }
  if (lhs_it != end() || rhs_it != other_list->end())
    return false;

  return true;
}
