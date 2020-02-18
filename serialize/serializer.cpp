#include "serializer.hpp"
#include <iostream>

Serialization::SerializerExceptions::InvalidType::InvalidType(const std::string& err)
    : std::runtime_error{err} {}

Serialization::SerializerExceptions::InvalidArgs::InvalidArgs(const std::string& err)
    : std::runtime_error{err} {}


void Serialization::MultipleSerializable::registerSerializer(SerializerId _id, const SerializerFunction& f)
{
    smap[_id] = f;
}

// throws if not have such Id
const Serialization::SerializerFunction& Serialization::MultipleSerializable::getCurrentSerializer()
{
    return smap.at(curId);
}


void Serialization::MultipleDeserializable::registerDeserializer(SerializerId _id, const DeserializerFunction& f)
{
    dsmap[_id] = f;
}

// throws if not have such Id
const Serialization::DeserializerFunction& Serialization::MultipleDeserializable::getCurrentDeserializer()
{
    return dsmap.at(curId);
}
