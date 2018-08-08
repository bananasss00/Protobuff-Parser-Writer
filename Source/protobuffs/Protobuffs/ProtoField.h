//https://developers.google.com/protocol-buffers/docs/encoding
#pragma once
#include <cstdint>
#include <string>

#define MAKE_TAG(FIELD_NUMBER, TYPE) static_cast<uint32_t>(((FIELD_NUMBER) << kTagTypeBits) | (TYPE))

struct Field;

static uint32_t readVarUint32(void* data, size_t& bytesRead);
static uint64_t readVarUint64(void* data, size_t& bytesRead);
static Field ReadField(void* data, size_t& bytesRead);

static constexpr int kTagTypeBits = 3;
static constexpr uint32_t kTagTypeMask = (1 << kTagTypeBits) - 1;
static constexpr int kMaxVarintBytes = 10;
static constexpr int kMaxVarint32Bytes = 5;

struct Tag
{
	unsigned field;
	unsigned type;
};

enum FieldType {
	TYPE_DOUBLE = 1,
	TYPE_FLOAT = 2,
	TYPE_INT64 = 3,
	TYPE_UINT64 = 4,
	TYPE_INT32 = 5,
	TYPE_FIXED64 = 6,
	TYPE_FIXED32 = 7,
	TYPE_BOOL = 8,
	TYPE_STRING = 9,
	TYPE_GROUP = 10,
	TYPE_MESSAGE = 11,
	TYPE_BYTES = 12,
	TYPE_UINT32 = 13,
	TYPE_ENUM = 14,
	TYPE_SFIXED32 = 15,
	TYPE_SFIXED64 = 16,
	TYPE_SINT32 = 17,
	TYPE_SINT64 = 18,
	MAX_FIELD_TYPE = 18,
};

enum WireType {
	WIRETYPE_VARINT = 0,
	WIRETYPE_FIXED64 = 1,
	WIRETYPE_LENGTH_DELIMITED = 2,
	WIRETYPE_START_GROUP = 3,
	WIRETYPE_END_GROUP = 4,
	WIRETYPE_FIXED32 = 5,
};

static const WireType kWireTypeForFieldType[MAX_FIELD_TYPE + 1] = {
	static_cast<WireType>(-1),  // invalid
	WIRETYPE_FIXED64,           // TYPE_DOUBLE
	WIRETYPE_FIXED32,           // TYPE_FLOAT
	WIRETYPE_VARINT,            // TYPE_INT64
	WIRETYPE_VARINT,            // TYPE_UINT64
	WIRETYPE_VARINT,            // TYPE_INT32
	WIRETYPE_FIXED64,           // TYPE_FIXED64
	WIRETYPE_FIXED32,           // TYPE_FIXED32
	WIRETYPE_VARINT,            // TYPE_BOOL
	WIRETYPE_LENGTH_DELIMITED,  // TYPE_STRING
	WIRETYPE_START_GROUP,       // TYPE_GROUP
	WIRETYPE_LENGTH_DELIMITED,  // TYPE_MESSAGE
	WIRETYPE_LENGTH_DELIMITED,  // TYPE_BYTES
	WIRETYPE_VARINT,            // TYPE_UINT32
	WIRETYPE_VARINT,            // TYPE_ENUM
	WIRETYPE_FIXED32,           // TYPE_SFIXED32
	WIRETYPE_FIXED64,           // TYPE_SFIXED64
	WIRETYPE_VARINT,            // TYPE_SINT32
	WIRETYPE_VARINT,            // TYPE_SINT64
};

struct Field
{
	friend class ProtoWriter;
private:
	Tag tag;
	std::string value;
	std::string full;

public:
	Field() : tag({ 0,0 }), value(""), full("") { }

	Field& operator=(const Field& f) {
		this->tag = f.tag;
		this->value = f.value;
		this->full = f.full;
		return *this;
	}

	Field(unsigned field, unsigned type, std::string value, std::string full) {
		this->tag = { field, type };
		this->value = value;
		this->full = full;
	}

	template<typename T>
	Field(Tag tag, T value) {
		auto wireType = kWireTypeForFieldType[tag.type];
		full = getBytesVarint32(MAKE_TAG(tag.field, wireType));

		switch (wireType) {
		case WIRETYPE_VARINT:
			full += getBytesVarint64((int64_t)value);
			break;
		case WIRETYPE_FIXED32:
			full += std::string{ reinterpret_cast<const char*>(&value), 4 };
			break;
		case WIRETYPE_FIXED64:
			full += std::string{ reinterpret_cast<const char*>(&value), 8 };
			break;
		}
	}

	template<typename T>
	Field(unsigned field, unsigned type, T value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = {field, (unsigned)wireType};
		full = getBytesVarint32(MAKE_TAG(field, wireType));

		switch (wireType) {
			case WIRETYPE_VARINT:
				full += getBytesVarint64( static_cast<uint64_t>(value) );
				break;
			case WIRETYPE_FIXED32:
				full += std::string{ reinterpret_cast<const char*>(&value), 4 };
				break;
			case WIRETYPE_FIXED64:
				full += std::string{ reinterpret_cast<const char*>(&value), 8 };
				break;
		}
	}

	Field(unsigned field, unsigned type, std::string value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = {field, (unsigned)wireType};
		full = getBytesVarint32(MAKE_TAG(field, wireType));
		full += getBytesVarint32(value.size());
		full += value;
	}
public:
	float Float() {
		return *reinterpret_cast<float*>((void*)value.data());
	}
	double Double() {
		return *reinterpret_cast<double*>((void*)value.data());
	}
	int32_t Int32() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	int64_t Int64() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	uint32_t UInt32() {
		size_t bytesRead;
		return readVarUint32((void*)value.data(), bytesRead);
	}
	uint64_t UInt64() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	uint32_t Fixed32() {
		return *reinterpret_cast<uint32_t*>((void*)value.data());
	}
	uint64_t Fixed64() {
		return *reinterpret_cast<uint64_t*>((void*)value.data());
	}
	int32_t SFixed32() {
		return *reinterpret_cast<int32_t*>((void*)value.data());
	}
	int64_t SFixed64() {
		return *reinterpret_cast<int64_t*>((void*)value.data());
	}
	bool Bool() {
		size_t bytesRead;
		return !!readVarUint32((void*)value.data(), bytesRead);
	}
	std::string String()
	{
		size_t bytesRead;
		void* data = (void*)value.data();
		auto length = readVarUint32((void*)value.data(), bytesRead);
		auto value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length };
		return value;
	}
private:
	static std::string getBytesVarint32(uint32_t value) {
		uint8_t bytes[kMaxVarint32Bytes];
		int size = 0;
		while (value > 0x7F) {
			bytes[size++] = (static_cast<uint8_t>(value) & 0x7F) | 0x80;
			value >>= 7;
		}
		bytes[size++] = static_cast<uint8_t>(value) & 0x7F;
		return std::string{ reinterpret_cast<const char*>(&bytes[0]), (size_t)size };
	}
	static std::string getBytesVarint64(uint64_t value) {
		uint8_t bytes[kMaxVarintBytes];
		int size = 0;
		while (value > 0x7F) {
			bytes[size++] = (static_cast<uint8_t>(value) & 0x7F) | 0x80;
			value >>= 7;
		}
		bytes[size++] = static_cast<uint8_t>(value) & 0x7F;
		return std::string{ reinterpret_cast<const char*>(&bytes[0]), (size_t)size };
	}
};

static uint32_t readVarUint32(void* data, size_t& bytesRead)
{
	auto ptr = reinterpret_cast<const uint8_t*>(data);
	auto value = 0u;
	auto bytes = 0u;

	do {
		value |= static_cast<uint32_t>(*ptr & 0x7f) << (7 * bytes);
		bytes++;
	} while (*(ptr++) & 0x80 && bytes <= 5);

	bytesRead = bytes;
	return value;
}

static uint64_t readVarUint64(void* data, size_t& bytesRead)
{
	auto ptr = reinterpret_cast<const uint8_t*>(data);
	auto value = 0ull;
	auto bytes = 0u;

	do
	{
		value |= static_cast<uint64_t>(*ptr & 0x7f) << (7 * bytes);
		bytes++;
	} while (*(ptr++) & 0x80 && bytes <= 10);

	bytesRead = bytes;

	return value;
}

static Field ReadField(void* data, size_t& bytesRead)
{
	unsigned field = *reinterpret_cast<uint16_t*>(data);
	unsigned type = field & kTagTypeMask;

	if (field == 0xffff) {
		bytesRead = 0;
		return Field();
	}

	if (field & 0x80) {
		field = ((field & 0x7f) | ((field & 0xff00) >> 1)) >> kTagTypeBits;
		bytesRead = 2;
	}
	else {
		field = (field & 0xff) >> kTagTypeBits;
		bytesRead = 1;
	}

	size_t length, sizeDelimited;
	std::string value, full;
	switch (type)
	{
	case WIRETYPE_VARINT:
		readVarUint64((void*)((ptrdiff_t)data + bytesRead), length);
		value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length };
		full = std::string{ reinterpret_cast<const char*>(data), bytesRead + length };
		bytesRead += length;
		break;
	case WIRETYPE_FIXED64:
		value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), 8 };
		full = std::string{ reinterpret_cast<const char*>(data), bytesRead + 8 };
		bytesRead += 8;
		break;
	case WIRETYPE_LENGTH_DELIMITED:
		sizeDelimited = readVarUint32((void*)((ptrdiff_t)data + bytesRead), length);
		value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length + sizeDelimited };
		full = std::string{ reinterpret_cast<const char*>(data), bytesRead + length + sizeDelimited };
		bytesRead += length + sizeDelimited;
		break;
	case WIRETYPE_START_GROUP:
		throw("WIRETYPE_START_GROUP unrealised");
		break;
	case WIRETYPE_END_GROUP:
		throw("WIRETYPE_END_GROUP unrealised");
		break;
	case WIRETYPE_FIXED32:
		value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), 4 };
		full = std::string{ reinterpret_cast<const char*>(data), bytesRead + 4 };
		bytesRead += 4;
		break;
	default:
		throw("Unknown type %i", type);
		break;
	}


	return Field(field, type, value, full);
}