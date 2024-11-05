#pragma once

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>

#include <cstdint>
#include <string>

class Grpcxx : public google::protobuf::compiler::CodeGenerator {
public:
	Grpcxx() {}
	virtual ~Grpcxx() {}

	virtual auto Generate(
		const google::protobuf::FileDescriptor *file, const std::string &parameter,
		google::protobuf::compiler::GeneratorContext *context, std::string *error) const
		-> bool override;

	constexpr auto GetSupportedFeatures() const -> uint64_t override {
		return FEATURE_PROTO3_OPTIONAL;
	}

private:
	auto GenerateService(
		const google::protobuf::FileDescriptor *file, const std::string &parameter,
		google::protobuf::compiler::GeneratorContext *context, std::string *error,
		std::string &output) const -> bool;

	auto GenerateClientStub(
		const google::protobuf::FileDescriptor *file, const std::string &parameter,
		google::protobuf::compiler::GeneratorContext *context, std::string *error,
		std::string &output) const -> bool;

	static auto MapToCppType(
		const google::protobuf::FileDescriptor *file, const google::protobuf::Descriptor *t)
		-> std::string;

	static auto ReplaceString(std::string input, std::string_view search, std::string_view replace)
		-> std::string;
};
